//////////////////////////////////////////////////////////////////////
//文件名称: PublicFunction.h:
//摘要:     公共函数头文件
//当前版本: 1.0
//作者:     qinfei
//完成日期: 20090530
///////////////////////////////////////////////////////

#ifndef _PUBLICFUNCTION_H_
#define _PUBLICFUNCTION_H_

#include "publicint.h"
using namespace std;

const UINT16_T I_PATH_NAME_SIZE=256; //目录名最大长度
const UINT16_T FILENAMEMAX=256; //文件名最大长度
const UINT16_T L_IPSIZE=16;

#ifdef _LINUX_ENV_

    #define GetLastError()         errno

	#define THREADFUNTYPE  void*

	typedef pthread_mutex_t CRITICAL_SECTION;

//  #define _snprintf   snprintf
	#define _vsnprintf  vsnprintf

	#define _atoi64(val)     strtoll(val, NULL, 10)
	#define _fseeki64         fseek

	#define Sleep(x) usleep(x*1000)

#else
	#define  strncasecmp		        _memicmp

	#define usleep Sleep
	#define THREADFUNTYPE  DWORD WINAPI
#endif


/////////////////////////////////////////////////////////////////////////////
//线程暂停指定秒
#ifdef _LINUX_ENV_
__inline const int GetTickCount()
{
	struct timeval tmVal;
	gettimeofday(&tmVal, 0);
	return tmVal.tv_sec*1000000+tmVal.tv_usec;
}

__inline const UINT64_T InterlockedIncrement(UINT64_T *lDest)
{
	UINT64_T ulRet=1;
#ifdef _LINUX_32_SYSTEM
	//asm("movl %0,%%eax" :"=m"(lDest) :"m"(lDest) :"%eax");
	//asm("lock; incl (%eax)");
	asm("lock ; xadd %0,(%2)"
		:"+r"(ulRet), "=m"(lDest)
		:"r"(lDest), "m"(lDest)
		: "memory");
#else
	//asm("movq %0,%%rax" :"=m"(lDest) :"m"(lDest) :"%rax");
	//asm("lock; incq (%rax)");
	asm("lock ; xadd %0,(%2)"
		:"+r"(ulRet), "=m"(lDest)
		:"r"(lDest), "m"(lDest)
		: "memory");
#endif
	return ulRet+1;
}

__inline const UINT64_T InterlockedDecrement(UINT64_T *lDest)
{
	UINT64_T ulRet=-1;
#ifdef _LINUX_32_SYSTEM
	//asm("movl %0,%%eax" :"=m"(lDest) :"m"(lDest) :"%eax");
	//asm("lock; decl (%eax)");
	asm("lock ; xadd %0,(%2)"
		:"+r"(ulRet), "=m"(lDest)
		:"r"(lDest), "m"(lDest)
		: "memory");
#else
	//asm("movq %0,%%rax" :"=m"(lDest):"m"(lDest) :"%rax");
	//asm("lock; decq (%rax)");

	asm("lock ; xadd %0,(%2)"
		:"+r"(ulRet), "=m"(lDest)
		:"r"(lDest), "m"(lDest)
		: "memory");
#endif

	return ulRet-1;
}

#define _setmaxstdio(iMax) (iMax)

#endif

//字符串转64位十进制数据函数
inline UINT64_T  iStrToI64(char *lpszSource)
{
	UINT64_T ulVal = 0;

	char *lpszStr=lpszSource, *lpszEnd=lpszStr+23;
	while(*lpszStr && lpszStr<lpszEnd)
	{
		if(*lpszStr<'0' || *lpszStr>'9')
			break;

		ulVal*=10;
		ulVal+=((*lpszStr)-'0');
		lpszStr++;
	}

	return ulVal;
}

//字符串转64位十进制数据函数
inline UINT64_T  iStrToI64(char *lpszSrc,const INT32_T iSrcLen)
{
	UINT64_T ulVal = 0;
	INT32_T iMax=iSrcLen>22?22:iSrcLen;
	for(INT32_T i=0;i<iMax;i++)
	{
		if(lpszSrc[i]<'0' || lpszSrc[i]>'9')
			break;

		ulVal*=10;
		ulVal+=(lpszSrc[i]-'0');
	}

	return ulVal;
}

////////////////////
//确定文件访问操作
//iMode :  00 Existence only 02  Write permission 04 Read permission 06 Read and write permission
inline const INT32_T  AccessFile( const char * const filename,INT32_T icode)
{
#ifdef _LINUX_ENV_
	return access(filename,icode);
#else
	return _access(filename,icode);
#endif
}

/////////////////////////////////////////////////////////////////////////////
//文件是否存在
inline const bool FileExists( const char * const lpszFileName)
{
	if(lpszFileName)
		return (AccessFile(lpszFileName,0)==0);
	return false;
}

////////////////////////////////////////////////////////////////////////////
//时间差
inline const void TimeDiff(struct timeval begin, struct timeval end, struct timeval diff)
{
	if (end.tv_usec >= begin.tv_usec)
	{
		diff.tv_sec = end.tv_sec - begin.tv_sec;
		diff.tv_usec = end.tv_usec - begin.tv_usec;
	}
	else
	{
		diff.tv_sec = end.tv_sec - begin.tv_sec - 1;
		diff.tv_usec = 1000000 + end.tv_usec - begin.tv_usec;
	}
	return;
}

/////////////////////////////////////////////////////////////////////////////
//取得文件SIZE
inline const time_t GetFileMTime(const char * const lpszFileName)
{
	if(lpszFileName==NULL)
		return 0;
	if(!FileExists(lpszFileName))
		return 0;

#ifdef _LINUX_ENV_
	struct stat sFileInfo;

	if(stat(lpszFileName,&sFileInfo)!=0)
		return 0;
#else
	struct _stat sFileInfo;
	if(_stat(lpszFileName,&sFileInfo)!=0)
		return 0;
#endif

	return sFileInfo.st_mtime;
}

/////////////////////////////////////////////////////////////////////////////
//取得文件SIZE
inline const INT64_T GetFileSize( const char * const lpszFileName)
{
	if(lpszFileName==NULL)
		return -1;
	if(!FileExists(lpszFileName))
		return -2;

#ifdef _LINUX_ENV_
	struct stat sStatus;

	if(stat(lpszFileName,&sStatus)!=0)
		return 0;
	return sStatus.st_size;
#else
	struct _stat sFileInfo;
	if(_stat(lpszFileName,&sFileInfo)!=0)
		return 0;

	return sFileInfo.st_size ;
#endif
}

/////////////////////////////////////////////////////////////////////////////
//取得文件SIZE
inline const INT64_T GetFileSize(FILE * const fpR)
{
	if(fpR==NULL)
		return -1;

	INT64_T iCurPos=ftell(fpR);
	fseek(fpR,0,SEEK_END);
	INT64_T iSize=ftell(fpR);
	fseek(fpR,iCurPos,SEEK_SET);

	return  iSize;
}


/////////////////////////////////////////////////////////////////////////////
//删除文件
static void RemoveFile(const char * const lpszFileName)
{
	if(lpszFileName==NULL)
		return;

	UINT32_T uCount=0;
	while(FileExists(lpszFileName) && remove(lpszFileName)!=0)
	{
		Sleep(1);
		if(uCount++>=10000)
		{
			printf("remove failed:%d\n",errno);
			uCount=0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//重命名文件
static void RenameFile( const char * const lpszFileName, const char * const lpszRenameFileName)
{
	if(lpszFileName==NULL || lpszRenameFileName==NULL)
		return ;

	RemoveFile(lpszRenameFileName);

	UINT32_T uCount=0;
	while(FileExists(lpszFileName) && !FileExists(lpszRenameFileName) && rename(lpszFileName,lpszRenameFileName)!=0)
	{
		Sleep(1);

		if(uCount++>=10000)
		{
			printf("rename failed:%ld\n",errno);
			uCount=0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////
//复制文件
static const INT32_T CopyFile(char *lpszFileName,char *lpszNewFileName)
{
	if(lpszFileName==NULL || lpszNewFileName==NULL)
		return -1;

	FILE *fpR=fopen(lpszFileName,"rb");
	if(fpR==NULL)
		return -2;

	FILE *fpW=fopen(lpszNewFileName,"wb");
	if(fpW==NULL)
	{
		fclose(fpR);
		return -3;
	}

	INT32_T lRet=0;
	INT64_T iLen=0,iAllLen=0;
	INT64_T iFileSize=GetFileSize(fpR);
	char szTmpBuf[1024];
	while(iAllLen<iFileSize)
	{
		iLen=fread(szTmpBuf,1,1024,fpR);
		if(iLen<=0)
		{
			printf("fread :%ld\n",errno);
			lRet=-4;
			break;
		}

		if(fwrite(szTmpBuf,iLen,1,fpW)!=1)
		{
			printf("fwrite :%ld\n",errno);
			lRet=-5;
			break;
		}

		iAllLen+=iLen;
	}

	fclose(fpR);
	fclose(fpW);
	return lRet;
}

/////////////////////////////////////////////////////////////////////////////
//复制文件
static const INT32_T MoveDataFile( const char * const lpszFileName, const char * const lpszNewFileName)
{
#ifdef _LINUX_ENV_
#else
	if(MoveFile(lpszFileName,lpszNewFileName))
		return 0;
	printf("move file:%ld\n",errno);
#endif
	return -1;
}
//
///////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//目录是否存在
static const bool IsExistDir( const char * const lpszDir)
{
#ifdef _LINUX_ENV_
#else

	struct _finddata_t sFindData;
	char szDir[I_PATH_NAME_SIZE];
	if(_snprintf(szDir,I_PATH_NAME_SIZE-1,"%s\\*.*",lpszDir)<=0)
		return false;

	INT32_T iRet   =   _findfirst(szDir,   &sFindData);
	if(iRet  !=   -1)
	{
		_findclose(iRet);
		return true;
	}
#endif
	return false;
}
//
////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
//建立目录
static const INT16_T CreateDir(char * const lpszDir)
{
	if(lpszDir==NULL)
		return 1;

	INT32_T lRet = 0, i = 0;

	char szTempDir[I_PATH_NAME_SIZE];
	char *lpszStr = lpszDir;
	INT32_T lLen = strlen(lpszDir),lPos = 0;
	if(lLen <= 0 || lLen >= I_PATH_NAME_SIZE)
		return 1;

	for (i = 0; i < lLen; i++)
	{
		if (lpszStr[i] == '\\' || lpszStr[i] =='/')
		{
			memcpy(szTempDir, lpszStr, i);
			szTempDir[i] = '\0';

			if(strcmp(szTempDir,"."))
			{
#ifdef _LINUX_ENV_
				lRet = mkdir(szTempDir,S_IRWXO|S_IRWXU);
#else
				lRet = _mkdir(szTempDir);
#endif
			}
		}
	}

	if (lpszStr[lLen - 1] != '\\' && lpszStr[lLen - 1] != '/')
	{
#ifdef _LINUX_ENV_
				lRet = mkdir(lpszStr,S_IRWXG|S_IRWXU);
#else
				lRet = _mkdir(lpszStr);
#endif
	}

#ifndef _LINUX_ENV_
	if(lRet==-1 && errno==EEXIST)
		return 1;
#endif

	return 0;
}
inline const INT32_T CreateFile(char *filename)
{
	FILE *fp = NULL;
	while ((fp = fopen(filename, "w")) == NULL);
	fclose(fp);
	fp = NULL;
	return 0;
}

inline const INT32_T RmDirFile(char *lpszDir)
{
	if(lpszDir == NULL)
		return 1;

	INT32_T iLen = (INT32_T)strlen(lpszDir), iNum = 0;
	if(iLen <= 0)
		return 1;

	bool bDir = true;
	if(lpszDir[iLen - 1] != '\\' && lpszDir[iLen - 1] != '/')
	{
		bDir = false;
	}

	char szFile[I_PATH_NAME_SIZE];



//	if(szFile[iLen - 1] != '\\' && szFile[iLen - 1] != '/')
//	{
//		szFile[iLen - 1] = '/';
//		iLen ++;
//	}
//	szFile[iLen] = 0;

#ifndef _LINUX_ENV_
	if(bDir)
	{
		iLen = _snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s*",lpszDir);
	}
	else
	{
		iLen = _snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s/*",lpszDir);
	}

	if(iLen <= 0)
		return 1;

	struct _finddata_t sF;
	int hFile = _findfirst(szFile, &sF);
	if(hFile == -1)
		return 1;

	do
	{
		if (strcmp(".", sF.name)==0 || strcmp("..", sF.name)==0 || (sF.attrib & _A_SUBDIR) != 0)
		{
			continue;
		}

		if(bDir)
		{
		     iLen = _snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s%s",lpszDir, sF.name);
		}
		else
		{
			iLen = _snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s/%s",lpszDir, sF.name);
		}

		if(iLen <= 0)
		{
			_findclose(hFile);
			return 1;
		}

		//printf("%s\n",szFile);
		if(remove(szFile) != 0)
		{
			printf("remove failed:%d\n",errno);
		}
		iNum++;
	} while (_findnext(hFile, &sF) == 0);

	_findclose(hFile);

#else

	if(bDir)
	{
		iLen = snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s",lpszDir);
	}
	else
	{
		iLen = snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s/",lpszDir);
	}

	if(iLen <= 0)
		return 1;

	DIR * spDir = opendir(szFile);
	if(spDir == NULL)
		return 1;

	struct dirent * spDE = NULL;

	while((spDE = readdir(spDir)) != NULL )
	{
		if (strcmp(".", spDE->d_name)==0 || strcmp("..", spDE->d_name)==0 )
		{
			continue;
		}

		if(bDir)
		{
		    iLen = snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s%s",lpszDir, spDE->d_name);
		}
		else
		{
			iLen = snprintf(szFile, I_PATH_NAME_SIZE - 1, "%s/%s",lpszDir, spDE->d_name);
		}

		if(iLen <= 0)
		{
			closedir(spDir);
			return 1;
		}

		if(remove(szFile) != 0)
		{
			printf("remove failed:%d\n",errno);
		}
		//printf("%s\n",szFile);
		iNum++;
	}

	closedir(spDir);

#endif

	printf("delete dir: %s, file num: %d \n",lpszDir, iNum);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
//去掉换行符 空格符
inline const UINT32_T CancelEnter(char * const lpszStr,UINT32_T uStrLen)
{
	if(lpszStr==NULL || uStrLen<=0)
		return 0;

	while(uStrLen>0  &&  (lpszStr[uStrLen-1]==0x20 || lpszStr[uStrLen-1]==0x0A || lpszStr[uStrLen-1]==0x0D) )
	{
		uStrLen--;
	}
	lpszStr[uStrLen]=0;
	return uStrLen;
}

/////////////////////////////////////////////////////////////////////////////
//去掉换行符 空格符
inline const UINT32_T CancelEnter(char * const lpszStr)
{
	if(lpszStr==NULL)
		return 0;
	UINT32_T uStrLen = strlen(lpszStr);

	while(uStrLen>0  &&  (lpszStr[uStrLen-1]==0x20 || lpszStr[uStrLen-1]==0x0A || lpszStr[uStrLen-1]==0x0D) )
	{
		uStrLen--;
	}
	lpszStr[uStrLen]=0;
	return uStrLen;
}

/////////////////////////////////////////////////////////////////////////////
//从字符串尾删除到指定字符的所有数据,不删除指定的字符
inline const UINT32_T DeleteEndString(char * const lpszStr,UINT32_T uStrLen,char cCode)
{
	if(lpszStr==NULL || uStrLen<=0)
		return 0;

	while(uStrLen>0 && lpszStr[uStrLen-1]!=cCode)
	{
		uStrLen--;
	}

	lpszStr[uStrLen]=0;
	return uStrLen;
}


/////////////////////////////////////////////////////////////////////////////
//线程锁
class CThreadMutex
{
public:
	CThreadMutex()
	{
#ifdef _LINUX_ENV_
		pthread_mutex_init(&crMutex,NULL);
#else
		InitializeCriticalSection(&crMutex);
#endif
	}

	~CThreadMutex()
	{
#ifdef _LINUX_ENV_
		pthread_mutex_destroy(&crMutex);
#else
		DeleteCriticalSection(&crMutex);
#endif
	}

	inline void Lock()
	{
#ifdef _LINUX_ENV_
		pthread_mutex_lock(&crMutex);
#else
		EnterCriticalSection(&crMutex);
#endif
	}

	inline void UnLock()
	{
#ifdef _LINUX_ENV_
		pthread_mutex_unlock(&crMutex);
#else
		LeaveCriticalSection(&crMutex);
#endif
	}

private:
	CRITICAL_SECTION crMutex;
};

class CThreadManage
{
public:
	CThreadManage()
	{
	}

	~CThreadManage()
	{
	}

#ifdef _LINUX_ENV_
	static const INT32_T StartThread(void* (in_function)(void*), void* in_vpArg)
	{
		pthread_t pid;
		pthread_create(&pid, 0, in_function, in_vpArg);
		return 0;
	}


	// 启动线程_带返回结果
	static const INT32_T StartThread(const UINT32_T iMax,void* (in_function)(void*), void* in_vpArg)
	{
		pthread_t pid;

		for(UINT32_T i=0;i<iMax;i++)
		{
			pthread_create(&pid, 0, in_function, in_vpArg);
		}
		return 0;
	}

#else
	static const INT32_T StartThread(const UINT32_T iMax,DWORD (WINAPI in_function)(void* ), void* in_vpArg)
	{
		DWORD dwThreadID=0;

		for(UINT32_T i=0;i<iMax;i++)
		{
			HANDLE hThread= CreateThread(NULL, 0, in_function, in_vpArg, 0, &dwThreadID);
			if(hThread == NULL)
				return -1;
			CloseHandle(hThread);
		}

		return 0;
	}

	static const INT32_T StartThread(DWORD (WINAPI in_function)(void* ), void* in_vpArg)
	{
		DWORD dwThreadID=0;
		HANDLE hHandle = CreateThread(NULL, 0, in_function, in_vpArg, 0, &dwThreadID);
		if(hHandle == NULL)
			return -1;

		return 0;
	}
#endif
};


inline const INT32_T GetXmlItem(char *&lpszStr,char *&lpszEnd,char *lpszTag,char *&lpszOutText)
{
	const INT32_T L_TAGNAMESIZE=100;
	char szTagName[L_TAGNAMESIZE];
	INT32_T iTagNameLen=snprintf(szTagName, L_TAGNAMESIZE, "%s>", lpszTag), iTagNM = iTagNameLen + 1;
	if(iTagNameLen <= 0)
		return -2;

	INT32_T iLen = -3;
	bool bCData=false;

	while(lpszStr<lpszEnd)
	{
		//if(*lpszStr == '<' && lpszStr + iTagNM < lpszEnd && strncasecmp(lpszStr + 1, szTagName, iTagNameLen) == 0)

		if(*lpszStr == '<')
		{
			if(lpszStr + iTagNM < lpszEnd && strncasecmp(lpszStr + 1, szTagName, iTagNameLen) == 0)
			{
				lpszStr += iTagNM;

				bCData=false;
				if(strncasecmp(lpszStr, "<![CDATA[", 9)==0)
				{
					lpszStr+=9;
					bCData=true;
				}

				lpszOutText=lpszStr;

				iTagNameLen=snprintf(szTagName, L_TAGNAMESIZE, "/%s>", lpszTag);
				if(iTagNameLen<=0)
					return -3;

				iTagNM = iTagNameLen + 1;

				while(lpszStr<lpszEnd)
				{
					if(*lpszStr == '<' && lpszStr + iTagNM < lpszEnd  && strncasecmp(lpszStr + 1, szTagName, iTagNameLen)==0)
						break;

					lpszStr++;
				}

				if(lpszStr>=lpszEnd)
					return -4;

				iLen=lpszStr-lpszOutText;

				if(bCData)
					iLen -= 3;

				lpszStr += iTagNM;
				break;
			}
		}

		lpszStr++;
	}

	return iLen;
}

inline const INT32_T GetXmlItem(char *&lpszStr, char *&lpszEnd, char *lpszTag, char *lpszOutText, const INT32_T iOutTextSize, INT8_T iCase = 0)
{
	const INT32_T L_TAGNAMESIZE = 100;
	char szTagName[L_TAGNAMESIZE];
	INT32_T iTagNameLen = snprintf(szTagName, L_TAGNAMESIZE, "%s>", lpszTag), iTagNM = iTagNameLen + 1;
	if(iTagNameLen <= 0)
		return -2;

	INT32_T iLen = -1;
	bool bCData = false;

	while(lpszStr<lpszEnd)
	{
		if(*lpszStr == '<' && lpszStr + iTagNM < lpszEnd && strncasecmp(lpszStr + 1,szTagName,iTagNameLen)==0)
		{
			lpszStr += iTagNM;

			if(strncasecmp(lpszStr,"<![CDATA[",9)==0)
			{
				lpszStr+=9;
				bCData=true;
			}

			char *lpszOut = lpszOutText, *lpszOutEnd = lpszOutText + iOutTextSize, *lpszStart = lpszStr;

			iTagNameLen = snprintf(szTagName,L_TAGNAMESIZE, "/%s>", lpszTag);
			if(iTagNameLen <= 0)
				return -3;

			iTagNM = iTagNameLen + 1;

			while(lpszStr<lpszEnd)
			{
				if(*lpszStr == '<' && lpszStr + iTagNM < lpszEnd  && strncasecmp(lpszStr + 1, szTagName, iTagNameLen)==0)
					break;

				if(lpszOut >= lpszOutEnd)
					return -4;

				if (iCase == 1)
				{
					if(*lpszStr >= 'A' && *lpszStr <= 'Z')
					{
						*lpszOut = *lpszStr + 32;
					}
					else
					{
						*lpszOut = *lpszStr;
					}
				}
				else if (iCase == 2)
				{
					if(*lpszStr >= 'a' && *lpszStr <= 'z')
					{
						*lpszOut = *lpszStr - 32;
					}
					else
					{
						*lpszOut = *lpszStr;
					}
				}
				else if (iCase == 0)
				{
					*lpszOut = *lpszStr;
				}
				else
				{
					return -11;
				}

				lpszOut++;
				lpszStr++;
			}

			if(lpszStr >= lpszEnd)
				return -5;

			iLen = lpszStr - lpszStart;

			if(bCData)
				iLen -= 3;

			lpszStr += iTagNM;
			break;
		}
		lpszStr++;
	}

	return iLen;
}
#endif

