#include <time.h>
#include <stdio.h>
#include <string>
#include <stdarg.h>
#include <sys/stat.h>
#include "session_log_c.h"

using std::string;

#ifndef _WIN32
#include <sys/syscall.h>
#endif

const unsigned int     LOG_MAX_SZ = 1024*1024*1024;
const unsigned int     CIRCLE_BUF_SZ = 8*1024*1024;
const unsigned int     ONE_LOG_MAX_SZ = 80*1024;
const unsigned int     MAX_THREAD_CNT = 256;
const unsigned int     MAX_LOG_PERDAY = 10;               //每天最大日志个数
const char* TIME_FORMAT = "%Y-%m-%d %H:%M:%S";
const char* TIME_FORMAT_FNAME = "%Y%m%d";

static const char *g_str_level[TOTAL_LEVEL] = {
	"[DEBUG]",
	"[INFO]",
	"[NOTICE]",
	"[WARN]",
	"[ERROR]",
	"[CRIT]",
	"[FATAL]"
};

struct log_buf_s 
{
	char* p_cqueue;
	int   tid;
};





static  int  g_run = 1;
static  int  g_thread_cnt = 0;
static  log_buf_s  g_log_bufs[MAX_THREAD_CNT];
static  int g_cnt_1thread[MAX_THREAD_CNT];


static void thread_free (void* p)
{
	free(p);
}

bool write_to_file(string& str, const char* log_dir)
{
#ifndef _WIN32
	char new_file[256];
	char log_time[80];
	FILE *fp;

	time_t t;
	time(&t);
	struct tm now;
	localtime_r(&t, &now);
	strftime(log_time, 80, TIME_FORMAT_FNAME, &now);

	int fileNo = 0;
	while (true)//找到当天的一个未写满的日志, 此处用循环是考虑到多进程操作日志
	{				
		if (fileNo > MAX_LOG_PERDAY)//
		{
			fprintf(stderr,"file:%s , line: %d, error info: log file more than %d\n",__FILE__,__LINE__, MAX_LOG_PERDAY);
			return false;		
		}

		sprintf(new_file, "%s/log-%s_%d", log_dir, log_time, fileNo); // 只取日志中的日期部分
		if (access(new_file, F_OK) == -1)//不存在该文件
			break;
		else 
		{
			struct stat buf;
			if (0 == stat(new_file, &buf) && buf.st_size < LOG_MAX_SZ)
				break;
		}
		++fileNo;
	}	

	fp = fopen(new_file, "a");
	if (!fp)	
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return false;		
	}
	fwrite(str.c_str(), str.length(), 1, fp);
	fclose(fp);		
#endif
	return true;
}


char* session_log_c::get_thread_buf()
{
#ifndef _WIN32
	void *ptr;
	if ((ptr = pthread_getspecific(m_pkey)) == NULL)
	{
		ptr = malloc(sizeof(int));
		pthread_spin_lock(&m_slock);

		if (g_thread_cnt >= MAX_THREAD_CNT)
		{
			fprintf(stderr,"file:%s , line: %d, error info: thread count exceed the max %d-->%d \n",
				__FILE__,__LINE__, g_thread_cnt, MAX_THREAD_CNT);
			pthread_spin_unlock(&m_slock);
			return NULL;
		}
		*(int*)ptr= g_thread_cnt++;

		pthread_spin_unlock(&m_slock);
		pthread_setspecific(m_pkey, ptr);
	}

	log_buf_s* lbs = &(g_log_bufs[*(int*)ptr]);
	if (lbs->p_cqueue == NULL)
	{
		lbs->tid = (int) syscall(__NR_gettid);
		char buf[256];
		sprintf(buf, "%s/%d",m_buf_dir, lbs->tid);
		lbs->p_cqueue = InitializeCircleQueue(buf, CIRCLE_BUF_SZ);
	}

	if (lbs->p_cqueue == NULL)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return NULL;
	}
	return lbs->p_cqueue;
#endif
	return NULL;
}



void session_log_c::dump_left(const char* dir)
{
#ifndef _WIN32
	

	fprintf(stderr, "dump_left enter\n");
	char buf[256];
	sprintf(buf, "%s/log_buf", dir);
	DIR *dp;
	struct dirent *entry;
	if((dp = opendir(buf)) == NULL)
	{
		fprintf(stderr, "cannot open directory: %s\n", buf);
		return ;
	}

	int tid;
	string str;
	time_t t;
	char* pq;
	char* retEle;
	char buf_tid[32];

	str.reserve(1024*1024);
	while((entry = readdir(dp)) != NULL)
	{
		tid = atoi(entry->d_name);
		if (tid != 0)
		{
			sprintf(buf_tid, "%d", tid);
			sprintf(buf, "%s/log_buf/%d",dir, tid);
			pq = InitializeCircleQueue(buf, CIRCLE_BUF_SZ);
			if (pq != NULL)
			{
				while(!IsCircleQueueEmpty(pq))
				{
					if(ReadFromCircleQueue(pq, &retEle, 0, &t) > 0)
						(((str +="tid = ") += buf_tid) += " ") +=retEle;
				}
				ReleaseCircleQuery(pq, CIRCLE_BUF_SZ);
			}
		}
	}
	closedir(dp);
	if (!str.empty())
	{
		write_to_file(str, dir);
	}
#endif

}

void dump_by_session(const char* dir)
{
#ifndef _WIN32
	int off, rdOff;
	char* retEle;
	char* pq;
	time_t t;
	string str;
	char buf_tid[32];

	str.reserve(1024*1024);
	int arr[MAX_THREAD_CNT]={0};
	for (int i = 0; i < g_thread_cnt; ++i)
	{
		pq = g_log_bufs[i].p_cqueue;
		if (pq)
		{
			if (GetBufferLeftRate(pq) < 90)//累计过多
			{
				off = PeekAllFromQueue(pq, str, g_log_bufs[i].tid, g_cnt_1thread[i]);

			}
			else
			{
				off = PeekSessionsFromQueue(pq, str, g_log_bufs[i].tid, g_cnt_1thread[i]);
			}
			arr[i]=off;
		}
	}

	if (!str.empty())
	{
		write_to_file(str, dir);
	}

	for (int i = 0; i < g_thread_cnt ; ++i)
	{
		if (arr[i] > 0)
		{
			SetReadOff(g_log_bufs[i].p_cqueue, arr[i]);
		}
	}
#endif

}

void* collect_thread_start(void* para)
{
	char* p_log_dir = (char*)para;
#ifndef _WIN32
	while(g_run)
	{
		usleep(1000);
		dump_by_session(p_log_dir);
	}
#endif
	return NULL;
}



session_log_c::session_log_c(void)
{

}

session_log_c::~session_log_c(void)
{

}


void session_log_c::close()
{
#ifndef _WIN32
	g_run = 0;
	pthread_join(m_collect_thread, NULL);
	pthread_spin_destroy(&m_slock);
	pthread_key_delete(m_pkey);
	for (int i = 0; i < g_thread_cnt; ++i)
	{
		if (g_log_bufs[i].p_cqueue)
			ReleaseCircleQuery(g_log_bufs[i].p_cqueue, CIRCLE_BUF_SZ);
	}
#endif
	fprintf(stderr, "logger exit");
}

bool session_log_c::init(const char* dir)
{
#ifndef _WIN32
	memset(g_log_bufs, 0, sizeof(g_log_bufs));
	memset(g_cnt_1thread, 0, sizeof(g_cnt_1thread));
	strncpy(m_dir, dir, sizeof(m_dir)-1);
	m_dir[sizeof(m_dir)-1] = '\0';
	
	char buf[512];
	sprintf(buf, "mkdir -p %s", m_dir);
	system(buf);

	memcpy(m_buf_dir, m_dir, sizeof(m_dir));
	strcat(m_buf_dir, "/log_buf");
	sprintf(buf, "mkdir -p %s", m_buf_dir);
	system(buf);

	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	if((dp = opendir(m_buf_dir)) == NULL)
	{
		fprintf(stderr, "cannot open directory: %s\n", m_dir);
		return false;
	}

	int tid;
	while((entry = readdir(dp)) != NULL)
	{
		tid = atoi(entry->d_name);
		if (tid != 0)
		{
			sprintf(buf, "%s/%d",m_buf_dir, tid);
			remove(buf);
		}
	}

	closedir(dp);
	m_level = L_NOTICE;
	if (pthread_spin_init(&m_slock, PTHREAD_PROCESS_PRIVATE) != 0)
		return false;
	if (pthread_key_create(&m_pkey, thread_free) != 0 )
		return false;
	g_thread_cnt = 0;

	pthread_create(&m_collect_thread, NULL, collect_thread_start, m_dir);

#endif
	return true;

}

void session_log_c::session_mark()
{
#ifndef _WIN32
	char* p_circle_buf = get_thread_buf();
	if (p_circle_buf == NULL)
		return;

	char buf[1]="";
	StoreToCircleQueue(p_circle_buf, buf, 0);
#endif
}


void session_log_c::write_log(int level, const char* file, int line, const char *fmt, ...)
{
#ifndef _WIN32

	char* p_circle_buf = get_thread_buf();
	if (p_circle_buf == NULL)
		return;

	if (level < 0 || level >= TOTAL_LEVEL || level < m_level)
		return ;

	if (fmt == NULL) 
		return ;
	
	int ret;
	struct tm now;
	char *log;
	char log_time[80];
	va_list ap;

	time_t t;
	time(&t);
	localtime_r(&t, &now);
	strftime(log_time, 80, TIME_FORMAT, &now);

	char buf[512];
	sprintf(buf, "%s %s [%s:%d] ", log_time, g_str_level[level], file, line);
	string str = buf;

	va_start(ap, fmt);
	ret = vasprintf(&log, fmt, ap);
	va_end(ap);
	if (ret != -1)
	{
		str+=log;
		str+="\n";
		free(log);
		if (GetBufferLeftRate(p_circle_buf) > 10 && str.length() < ONE_LOG_MAX_SZ)
		{
			StoreToCircleQueue(p_circle_buf, str.c_str(), str.length()+1);
		}
	}
	else
	{
		fprintf(stderr, "FILE:%s, LINE:%d, failed to format output\n", __FILE__, __LINE__);
		//notice
	}

#endif

}


void write_log_open_version(void* logger, bool single, int level, const char* file, int line, const char *fmt, ...)
{
#ifndef _WIN32

	session_log_c* plog = (session_log_c*)logger;
	char* p_circle_buf = plog->get_thread_buf();
	if (p_circle_buf == NULL)
		return;

	if (level < 0 || level >= TOTAL_LEVEL || level < plog->m_level)
		return ;

	if (fmt == NULL) 
		return ;

	int ret;
	struct tm now;
	char *log;
	char log_time[80];
	va_list ap;

	time_t t;
	time(&t);
	localtime_r(&t, &now);
	strftime(log_time, 80, TIME_FORMAT, &now);

	char buf[512];
	sprintf(buf, "%s %s [%s:%d] ", log_time, g_str_level[level], file, line);
	string str = buf;

	va_start(ap, fmt);
	ret = vasprintf(&log, fmt, ap);
	va_end(ap);
	if (ret != -1)
	{
		str+=log;
		str+="\n";
		free(log);
		if (GetBufferLeftRate(p_circle_buf) > 10 && str.length() < ONE_LOG_MAX_SZ)
		{
			StoreToCircleQueue(p_circle_buf, str.c_str(), str.length()+1);
		}
	}
	else
	{
		fprintf(stderr, "FILE:%s, LINE:%d, failed to format output\n", __FILE__, __LINE__);
		//notice
	}
	if (single)
	{
		plog->session_mark();
	}

#endif

}
