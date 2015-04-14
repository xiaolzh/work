//////////////////////////////////////////////////////////////////////
//文件名称: PulicInt.h:
//摘要:     公共数据类型
//当前版本: 1.0
//作者:     qinfei
//完成日期: 20090530 
///////////////////////////////////////////////////////
#ifndef _PUBLICINT_H_
#define _PUBLICINT_H_

#ifndef WIN32
	#ifndef _LINUX_ENV_
		#define _LINUX_ENV_
	#endif
#endif

#ifdef _LINUX_ENV_
	#include <ctype.h>
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/param.h>  
	#include <sys/ioctl.h>  
	#include <net/if.h>  
	#include <net/if_arp.h>

	#include <pthread.h>

	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
    #include <errno.h>

    #include<sys/file.h>
	#include <sys/select.h>
	#include <sys/time.h>
	#include <unistd.h>

	#include <sys/types.h>
	#include <sys/stat.h>
	#include <sys/epoll.h>
	#include <sys/socket.h>
    #include<dirent.h>

	#include <fcntl.h>
	#include <pthread.h>
	#include <signal.h>

#else
#pragma warning(disable:4996)

	#include <io.h>
	#include <sys/locking.h>
	#include <sys/types.h>
    #include <sys/stat.h>
	#include <sys/locking.h>
	#include <share.h>
	#include <fcntl.h>
	#include <direct.h>
	#include <errno.h>

	#include <Winsock2.h>
	#include <Windows.h>

	#pragma comment(lib,"Ws2_32.lib")

	#define _LINUX_64_SYSTEM
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>


typedef char INT8_T;
typedef short INT16_T;
typedef int INT32_T;
typedef float FLOAT32_T;
typedef long LINT_T;

typedef unsigned char UINT8_T;
typedef unsigned short UINT16_T;
typedef unsigned int UINT32_T;
typedef unsigned long ULINT_T;

#ifdef _LINUX_ENV_

#ifdef _LINUX_32_SYSTEM
typedef unsigned long long UINT64_T;
typedef long long INT64_T;
#else
typedef unsigned long UINT64_T;
typedef long INT64_T;
#endif

typedef long LONG;
#else
typedef  __int64  INT64_T;
typedef unsigned __int64 UINT64_T;

#endif

#ifdef _LINUX_ENV_
#define PTF_U64 "%lu"
#define PTF_64 "%ld"
#else
#define PTF_U64 "%I64u"
#define PTF_64 "%I64"
#endif

#endif


