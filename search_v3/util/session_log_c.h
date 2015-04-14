#ifndef SESSION_LOG_H
#define SESSION_LOG_H

/*
*by liugang 2012.4.5
*log system for common log and session log
*mutithread environment logs in one session can be organized together
*mutithread write log lockfree
*/



#ifndef _WIN32
#include <dirent.h>
#include <pthread.h>
#include "circle_queue.h"
#endif

enum {
	L_DEBUG,
	L_INFO,
	L_NOTICE,
	L_WARN,
	L_ERROR,
	L_CRIT,
	L_FATAL,

	TOTAL_LEVEL
};


class session_log_c
{
public:
	session_log_c(void);
	~session_log_c(void);

	//call first in main thread , set log dir;
	bool init(const char* dir);
	void close();
	void write_log(int level, const char* file, int line, const char *fmt, ...);
	void session_mark();

	int get_level(){return m_level;}
	void set_level(int l){m_level = l;}

	static void dump_left(const char* dir);

public:
	int  m_level;
	char* get_thread_buf();
private:
	char m_dir[256];
	char m_buf_dir[256];
	
#ifndef _WIN32
	 pthread_spinlock_t  m_slock;
	 pthread_key_t  m_pkey;
	 pthread_t      m_collect_thread;
#endif

};


//this version for linux dll
void write_log_open_version(void* logger, bool single, int level, const char* file, int line, const char *fmt, ...);

#endif
