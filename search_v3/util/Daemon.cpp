#include "Daemon.h"
#include "session_log_c.h"
#include <sys/wait.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
CWorker *CDaemon::m_pWorker=NULL;
int CDaemon::m_nChildPid=0;
CDaemon::CDaemon(void)
{
}

CDaemon::~CDaemon(void)
{
}



//���ļ���һ�У�buf �����뻺������maxCount����ֽ�������mode �ļ��򿪷�ʽ��ʧ�ܷ��� 0��
int CDaemon::Read1LineFromFile(const char* fileName, char* buf, int maxCount, const char* mode)
{
	FILE* fp = fopen(fileName, mode);
	if (!fp)
		return 0;
	int ret;
	fgets(buf, maxCount, fp) ? ret = 1 : ret = 0;
	fclose(fp);
	return ret;
}

//��������д���ļ���buf ������ ��mode �ļ��򿪷�ʽ��ʧ�ܷ��� 0��
int CDaemon::WriteBuff2File(const char* fileName, const char* buf, const char* mode)
{
	FILE* fp = fopen(fileName, mode);
	if (!fp)
		return 0;
	int n = fprintf(fp, "%s", buf);
	fclose(fp);
	return n;
}

//************************************
// Method:    ParseCmdLine ���������в�������ʽΪ -x xxx .... ��ʽ
// FullName:  CDaemon::ParseCmdLine
// Access:    public 
// Returns:   int	
// Qualifier:
// Parameter: int argc
// Parameter: char * * argv
//************************************
int CDaemon::ParseCmdLine(int argc,char** argv)
{
	char* m_pName=argv[0];
	
	if(argc<2)return 0;
	for (int i=1;i<argc;++i)
	{
		if (argv[i][0]!='-')
			continue;

		if (i==argc-1)//���滹��
			break;

		if (argv[i+1][0]=='-')
		{
			m_hisOptVal[argv[i][1]]="";
			continue;
		}

		m_hisOptVal[argv[i][1]]=argv[i+1];
		++i;
	}	

	const char* pchDir;
	HISI it=m_hisOptVal.find('d');
	if(it!=m_hisOptVal.end())
		pchDir=it->second.c_str();
	else
		pchDir=".";
	
	struct stat st;	
	if(stat(pchDir,&st)<0)
	{
		fprintf(stderr,"stat dir %s fail,file %s,line %d -- info:%s\n", pchDir,__FILE__,__LINE__,strerror(errno));
		exit(0);
	}

	if(0>chdir(pchDir))
	{

		fprintf(stderr, "work dir %s does not exist\n");
		exit(0);
	}

	char buf[128];
	getcwd(buf,128);	
	m_runPath = buf;
	m_runPath+='/';

	return m_hisOptVal.size();
}

/* modify from book apue
* Ϊ��ֹ�ӽ�����������, �����̻����������ӽ���.
* ����:1.�ӽ�����EXIT��ʽ�˳�. 2. kill -9 ɱ���ý���
*/
bool isAbnormalExit(int pid, int status)
{
	bool bRestart = true;
	if (WIFEXITED(status)) //exit()or return ��ʽ�˳�
	{
		fprintf(stdout, "child normal termination, exit pid = %d, status = %d", pid, WEXITSTATUS(status));
		bRestart = false;
	}
	else if (WIFSIGNALED(status)) //signal��ʽ�˳�
	{
		fprintf(stderr, "abnormal termination, pid = %d, signal number = %d%s\n", pid, WTERMSIG(status),
#ifdef	WCOREDUMP
			WCOREDUMP(status) ? " (core file generated)" : "");
#else
			"");
#endif

		if (WTERMSIG(status) == SIGKILL)
		{
			bRestart = false;
			 fprintf(stderr, "has been killed by user ??, exit pid = %d, status = %d", pid, WEXITSTATUS(status));
		}
	}
	else if (WIFSTOPPED(status)) //��ͣ���ӽ����˳�
		 fprintf(stderr, "child stopped, pid = %d, signal number = %d\n", pid, WSTOPSIG(status));
	else
		 fprintf(stderr, "child other reason quit, pid = %d, signal number = %d\n", pid, WSTOPSIG(status));

	return bRestart;
}




bool CDaemon::Start()
{
	//���ļ���ȡ�������е�daemon��pid
	char buf[640];
	int masterPid;
	
	string strName = m_runPath + MASTER_PID_FILE ;
	strName+=m_pName;
	bool bStart=false;
	if ( 0<Read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0)
	{
		printf("readlast %d:masterPid\n",masterPid);
		if (kill(masterPid, 0) == 0)
		{
			printf("Another instance exist, ready to quit!\n");
			return true;
		}

	}

	initAsDaemon();

	sprintf(buf, "%d", getpid());
	if (!WriteBuff2File(strName.c_str(), buf, "w"))
	{
		//log_warn(g_logger, "Write master pid fail!");
		fprintf(stderr, "Write master pid fail!\n");
	}

	while(true)
	{
		pid_t pid = fork();
		if (pid == 0)
		{
			//�ӽ�����������

			signal(SIGUSR1, sigChildHandler);
			signal(SIGPIPE, SIG_IGN);
			signal(SIGTTOU, SIG_IGN);
			signal(SIGTTIN, SIG_IGN);
			signal(SIGTERM, SIG_IGN);
			signal(SIGINT,  SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			
			if (!m_pWorker->Init(m_hisOptVal))
			{	
				fprintf(stderr, "Worker init  fail!\n");
				return false;
			}
			fprintf(stdout, "Worker init  ok pid = %d\n",(int)getpid());

			if (!m_pWorker->Run())
			{
				fprintf(stderr, "run finish -fail!\n");
				return false;
			}

			fprintf(stdout, "run finish -ok!\n");

			if(!m_pWorker->Dispose())
			{
				fprintf(stderr, "Worker dispose -fail!\n");
				return false;
			}

			fprintf(stdout, "Worker dispose -ok!\n");
			exit(0);
		}
		m_nChildPid=pid;
		int status;
		pid = wait(&status);
		session_log_c::dump_left("log");
		if (!isAbnormalExit(pid, status))
		{
			fprintf(stdout, "child exit normally?? see detail in err.log \n");
			break;
		}
	}
	return true;
}


bool CDaemon::Stop()
{
	char buf[640];
	int masterPid;
	
	string strName = m_runPath + MASTER_PID_FILE ;
	strName+=m_pName;

	bool bStart=false;
	if ( 0<Read1LineFromFile(strName.c_str(), buf, 64, "r") &&(masterPid = atoi(buf)) != 0)
	{
		if (kill(masterPid, 0) == 0)
		{
			fprintf(stdout, "find previous daemon pid= %d, current pid= %d\n", masterPid, getpid());
			int tryTime = 200;		
			kill(masterPid, SIGTERM);
			while (kill(masterPid, 0) == 0 && --tryTime)
				sleep(1);			

			if (!tryTime && kill(masterPid, 0) == 0)
			{
				fprintf(stderr, "Time out shutdown fail!\n");		
				return false	;
			}

			return true;
		}

	}

	printf("Another instance doesn't exist, ready to quit!\n");
	return true;
}
//��ʼ��DAEMON
void CDaemon::initAsDaemon()
{
	if (fork() > 0)
		exit(0);
	setsid();

	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTERM, sigMasterHandler);
	signal(SIGINT,  sigMasterHandler);
	signal(SIGQUIT, sigMasterHandler);
	signal(SIGKILL, sigMasterHandler);
}
//�����̽���USR1�źŴ�����
void CDaemon::sigMasterHandler(int sig)
{		
	kill(m_nChildPid,SIGUSR1);
	fprintf(stdout, "master = %d sig child =%d!\n",getpid(),m_nChildPid);

}

void CDaemon::sigChildHandler(int sig)
{		
	if (sig == SIGUSR1)
	{
		m_pWorker->close();
		fprintf(stdout, "master = %d signal accept current pid =%d!\n",getppid(),getpid());
	}

}


bool CDaemon::Run(int argc,char** argv)
{
	ParseCmdLine(argc,argv);
	char*p=strrchr(argv[0],'/');
	p!=NULL?p=p+1:
		p=argv[0];
	m_pName=p;


	HISI i=m_hisOptVal.find('k');
	if (i!=m_hisOptVal.end())
	{
		if (i->second=="start")
		{
			return Start();
		}
		else
		{
			return Stop();
		}
	}
	return true;

}


