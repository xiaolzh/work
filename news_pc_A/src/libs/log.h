#ifndef _LOG_H_
#define _LOG_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#ifdef WIN32
#include <direct.h>
#include <io.h>
#include <Windows.h>
#else
#include <linux/types.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#endif

#ifdef WIN32
#define SPRINT _snprintf
#else
#define SPRINT snprintf
#endif

#define VSPRINT vsnprintf
#define SLASH 			"/"
#define MAX_FILE_PATH 	256
#define FL				__FILE__
#define LN				__LINE__


class Log_file
{
private:
    char filenm[MAX_FILE_PATH];
    char ident[32];
    int curid;
    int level;
    int loop;
    int maxsize;
    int maxtime;
    int timestamp;
    int byteswrite;
#ifdef WIN32
    HANDLE h_fl;
    CRITICAL_SECTION l_CSLog;
#else
    int h_fl;
    pthread_mutex_t l_CSLog;
#endif
	int rotatelog()
	{
		int j=0;
		int oldid=0;
		int tmpid=0;
		char oldfile[MAX_FILE_PATH]={0};
	#ifdef WIN32
		LPVOID lpMsgBuf;
		DWORD dw=0;
	#endif
			
		//get the oldest file
		oldid = (curid + 1) % loop;	
		j=SPRINT(oldfile,MAX_FILE_PATH,"%s/%s.%02d",filenm,ident,oldid);
		if(j<=0)
		{
			return 0;
		}
	#ifdef WIN32
		if(DeleteFile(oldfile)==0)
		{
			LPVOID lpMsgBuf;
			DWORD dw = GetLastError(); 
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );
			LocalFree(lpMsgBuf);		
		}
		//close current file
		if(h_fl!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(h_fl);
			h_fl=INVALID_HANDLE_VALUE;
		}
		//set new file id
		tmpid = curid;
		curid = oldid;	
		//open new file
		h_fl=CreateFile(oldfile,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,0);
		if(h_fl==INVALID_HANDLE_VALUE)
		{
			dw = GetLastError(); 
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );			
			//printf("open file %s failed:%d,%s (init)!\n",oldfile,dw,lpMsgBuf);
			LocalFree(lpMsgBuf);
			curid = tmpid;
			return 0;
		}	
	#else
		//remove it
		if(remove(oldfile)!=0)
		{
			//printf("delete oldfile %s failed with error %d:%s!\n",oldfile,errno,strerror(errno));		
		}
		//close current file
		if(h_fl!=-1)
		{
			close(h_fl);
			h_fl=-1;
		}
		//set new file id
		tmpid = curid;
		curid = oldid;	
		//open new file
		h_fl=open(oldfile,O_WRONLY | O_TRUNC | O_APPEND | O_CREAT ,S_IRUSR | S_IWUSR);
		if(h_fl==-1)
		{
			//printf("open file %s failed:%d,%s (init)!\n",oldfile,errno,strerror(errno));
			curid = tmpid;
			return 0;
		}	
	#endif
		//set byteswrite
		byteswrite = 0;
		timestamp = (int)time(NULL);
		return 1;
	}

	int getlast(char *flast,int &f_size)
	{
		int find=0;
	#ifdef WIN32
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		FILETIME lstmodi;

		f_size=0;
		hFind = FindFirstFile(tofind, &FindFileData);
		if(hFind == INVALID_HANDLE_VALUE) 
		{
			printf("目录下没有找到%s文件,error:%d\n",tofind,GetLastError());	
			find=0;
		}
		else
		{	
			lstmodi=FindFileData.ftLastWriteTime;
			SPRINT(flast,254,"%s\\%s",filenm,FindFileData.cFileName);
			while(FindNextFile(hFind,&FindFileData))
			{
				if(CompareFileTime(&(FindFileData.ftLastWriteTime),&lstmodi)==1)					
				{
					lstmodi=FindFileData.ftLastWriteTime;
					SPRINT(flast,254,"%s\\%s",filenm,FindFileData.cFileName);
					f_size=(int)FindFileData.nFileSizeLow;
				}
			}
			FindClose(hFind);
			find=1;
		}
	#else
		DIR   *dirp;  
		struct   dirent   *direntp;  
		time_t lastmodi=0;           
		f_size=0;  
		//opendir
		if((dirp=opendir(filenm))==NULL)  
		{
			printf("Open   Directory   Error:\n");  
			return   0;  
		}         
		while((direntp=readdir(dirp))!=NULL)  
		{  
			if(strncmp(direntp->d_name,ident,strlen(ident)))  
						continue;  
			struct   stat   statbuf;  
			char   filename[256];  
			memset(filename,'\0',256);  
			strcpy(filename,filenm);  
			strcat(filename,"/");  
			strcat(filename,direntp->d_name);  

			if(stat(filename,&statbuf)==-1)  
			{ 
				//printf("Get   stat   on   %s   Error:%s\n",   direntp->d_name,strerror(errno));    
			closedir(dirp);
				return   0;  
			}  
			if(!S_ISDIR(statbuf.st_mode))  
			{  
				if(statbuf.st_mtime > lastmodi)
				{
					find=1;
					lastmodi = statbuf.st_mtime;
					f_size = statbuf.st_size;
					memcpy(flast,filename,strlen(filename));	
				}
			}
		}
		closedir(dirp);
	#endif
		return find;
	}

public:
	Log_file()
	{
		memset(filenm,0,MAX_FILE_PATH);
		memset(ident,0,32);
		curid=0;
		level=1;
		loop=3;
		maxsize=200 << 20;
		maxtime=86400;
		timestamp=(int)time(NULL);
		byteswrite=0;

	#ifdef WIN32
		h_fl=INVALID_HANDLE_VALUE;
	#else
		h_fl=-1;
	#endif
	}

	~Log_file()
	{
	#ifdef WIN32
		if(h_fl!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(h_fl);
			h_fl=INVALID_HANDLE_VALUE;
		}
		DeleteCriticalSection(&l_CSLog);
	#else
		if(h_fl!=-1)
		{
			close(h_fl);
			h_fl=-1;
		}
		pthread_mutex_destroy(&l_CSLog);
	#endif
	}

	int Init(char *file,char *idt,int numb,int size,int time)
	{
		int j = 0;
		int find = 0;	
		char fnm[MAX_FILE_PATH + 1] = {0};
		char src_file[MAX_FILE_PATH + 1] = {0};
		char *p = NULL;
	#ifdef WIN32
		LPVOID lpMsgBuf;
		DWORD dw = 0, dwPtr = 0;
	#endif

		if(file==NULL || idt == NULL )
		{
			printf("Invalid parameter file for Log_file!\n");
			return 1;
		}
		j = SPRINT(filenm, MAX_FILE_PATH, "%s", file);
		if (j <= 0)
		{
			printf("set filenm for Log_file failed!\n");
			return 1;
		}
	#ifdef WIN32
		if(_access(filenm,0)==-1)
		{
			_mkdir(filenm);
		}
	#else
		if(access(filenm,F_OK)==-1)
		{
			if(mkdir(filenm,S_IRWXU)==-1)
			{
				printf("create dir %s failed!\n",filenm);
				return 1;
			}
		}
	#endif	
		j=SPRINT(ident,32,"%s",idt);
		if (j <= 0)
		{
			printf("set ident for Log_file failed!\n");
			return 1;
		}		
		curid = 0;
		level = 1;
		if (numb <= 0 || numb > 100)
		{
			loop = 4;
		}
		else
		{
			loop = numb;
		}
		if (size <= 0 || size > (1<<30))
		{
			maxsize = 200 << 20;
		}
		else
		{
			maxsize = size;
		}
		if(time <=0 || time > 86400 * 30)
		{
			maxtime = 86400;
		}
		else
		{
			maxtime = time;
		}
	#ifdef WIN32
		try{
			InitializeCriticalSection(&l_CSLog);
		}
		catch(...)
		{
			printf("Init CriticalSection failed for Log_file!\n");
			return 1;
		}
	#else
		if(pthread_mutex_init(&l_CSLog,NULL)!=0)
		{
			printf("Init mutex failed for Log_file!\n");
			return 1;
		}
	#endif
		//get last fileid,set current fileid
		find=getlast(src_file,byteswrite);
		//get last id
		if(find==1)
		{
			p = strrchr(src_file,'.');
			if (p != NULL)
			{
				curid=atol(p+1) % loop;
			}
			else
			{
				printf("Bad parameter %s\n",src_file);
				return 1;
			}
		}
		else
		{
			curid=0;
		}
		//get filenm
		j=SPRINT(fnm,MAX_FILE_PATH,"%s/%s.%02d",filenm,ident,curid);
		if(j>0)
		{
			if(find==1)
			{
				printf("file %s existing,appending ...\n",fnm);
	#ifdef WIN32
				h_fl=CreateFile(fnm,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,0);
				if(h_fl==INVALID_HANDLE_VALUE)
				{
					dw = GetLastError(); 
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );			
					printf("open file %s failed:%d,%s (init)!\n",fnm,dw,lpMsgBuf);
					LocalFree(lpMsgBuf);
					return 1;
				}
				//set byteswrite
				byteswrite = GetFileSize(h_fl,NULL);	
				if(byteswrite == INVALID_FILE_SIZE)
				{
					dw = GetLastError(); 
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );
					printf("Get file %s's size failed:%d,%s(init)!\n",fnm,dw,lpMsgBuf);
					return 1;
				}
				dwPtr = SetFilePointer(h_fl, byteswrite, NULL, FILE_BEGIN);  
				if (dwPtr == ((DWORD)-1)) //INVALID_SET_FILE_POINTER 
				{ 
					dw = GetLastError(); 
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );
					printf("Set file %s's ointer failed:%d,%s(init)!\n",fnm,dw,lpMsgBuf);
					return 1;
				}
	#else
				h_fl=open(fnm,O_WRONLY | O_APPEND);
				if(h_fl==-1)
				{
					printf("open file %s failed:%d,%s (init)!\n",fnm,errno,strerror(errno));
					return 1;
				}
	#endif
			}
			else
			{
	#ifdef WIN32
				h_fl=CreateFile(fnm,GENERIC_WRITE,FILE_SHARE_READ,NULL,OPEN_ALWAYS,0,0);
				if(h_fl==INVALID_HANDLE_VALUE)
				{
					dw = GetLastError(); 
					FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |FORMAT_MESSAGE_FROM_SYSTEM,NULL,dw,MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPTSTR) &lpMsgBuf,0, NULL );			
					printf("open file %s failed:%d,%s (init)!\n",fnm,dw,lpMsgBuf);
					LocalFree(lpMsgBuf);
					return 1;
				}
	#else
				h_fl=open(fnm,O_WRONLY | O_TRUNC | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
				if(h_fl==-1)
				{
					printf("open file %s failed:%d,%s (init)!\n",fnm,errno,strerror(errno));
					return 1;
				}
	#endif
				//set byteswrite
				byteswrite = 0;
			}
		}
		else
		{
			printf("Bad parameter %s for log_file current filename\n",fnm);
			return 1;
		}
		return 0;
	}

	int setloglevel(int lv)
	{
		if(lv<=0)
		{
			level=0;
		}
		else
		{
			level=1;
		}
		return 1;
	}

	int getloglevel()
	{	
		return level;
	}

	int writelog(const char *msgfmt,...)
	{
		va_list ap;
		char *pos = NULL;
		char *p=NULL,message[2048];
		int sz=0,j=0;
		time_t t=0;
		int tm=0;
	#ifdef WIN32	
		DWORD dwBytesWritten=0;
	#else
		int dwBytesWritten=0;
	#endif

		if(level==0)
		{
			return 0;
		}
		t = (int)time(NULL);
		pos = ctime(&t);
		if(NULL==pos)
		{
			return 0;
		}
			sz = strlen(pos);
			/* chop off the \n */
			pos[sz-1]=' ';
		/* insert the header */
			j=SPRINT(message, 100, "%s ", pos);

		if(j>0)
		{
			message[j]=0;
			for(p=message;*p!='\0';p++);
			sz=p-message;
			va_start(ap,msgfmt);
			VSPRINT(p,2048 - sz,msgfmt,ap);
			va_end(ap);
	#ifdef WIN32
			EnterCriticalSection(&l_CSLog);		
			//check if need rotate		
			tm = (int)time(NULL) - timestamp;
			if(tm >= maxtime || byteswrite >= maxsize)
			{
				rotatelog();
			}
			//write file
			if(h_fl!=INVALID_HANDLE_VALUE)
			{
				if(WriteFile(h_fl,message,strlen(message),&dwBytesWritten,NULL)!=0)
				{
					byteswrite += dwBytesWritten;
				}
			}			
			LeaveCriticalSection(&l_CSLog);
	#else
			pthread_mutex_lock(&l_CSLog);		
			//check if need rotate		
			tm = (int)time(NULL) - timestamp;
			if(tm >= maxtime || byteswrite >= maxsize)
			{
				rotatelog();
			}
			//write file
			if(h_fl!=-1)
			{
				if((dwBytesWritten=write(h_fl,message,strlen(message))) >0 )
				{
					byteswrite += dwBytesWritten;
				}
				else
				{
					//printf("write failed:%d,%s\n",errno,strerror(errno));
				}
			}
			pthread_mutex_unlock(&l_CSLog);		
	#endif
		}
		return 1;
	}
};
#endif
