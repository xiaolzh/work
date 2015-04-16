
#ifndef WF_PUB_H
#define WF_PUB_H

#include "WFInt.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>

const int INVALID_ID = 0;
#define USE_MFS 0    //rename will check twice on MFS

#ifdef WIN32
#define WINDOWS_PLATFORM
#pragma message("当前是Windows系统:x32")
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#pragma message("当前是Windows系统:x64")
#endif

#ifdef WINDOWS_PLATFORM //是WINDOWS平台

    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <direct.h>
    #include <io.h>
    #define snprintf _snprintf
    #define access _access
    #define I64d "%I64d"
    #define I64u "%I64u"
    #define I64x "%I64x"

#ifdef _USE_32BIT_TIME_T
    #define PTTF "%d"   //PTF = print time_t format
#else
    #define PTTF "%I64d"
#endif

#else   //是非WINDOWS平台

    #pragma message("当前是类Unix系统")
    #include <unistd.h>
    #include <dirent.h>
    #include <errno.h>
    #include <pthread.h>
    #include <sys/time.h>
    #define I64d "%lld"
    #define I64u "%llu"
    #define I64x "%llx"

    #define PTTF "%lld"

#endif


namespace nsWFPub
{

//FUNC  得到错误码
static int LastError()
{
#ifdef WINDOWS_PLATFORM
    return GetLastError();
#else
    return errno;
#endif
}

//FUNC  Sleep，秒级
//PARAM second：Sleep的秒数
static void SleepSecond(size_t second)
{
#ifdef WINDOWS_PLATFORM
    Sleep(second*1000);
#else
    sleep(second);
#endif
}

//FUNC  Sleep，毫秒级
//PARAM millisecond：Sleep的毫秒数
static void SleepMilliSecond(size_t millisecond)
{
#ifdef WINDOWS_PLATFORM
    Sleep(millisecond);
#else
    usleep(millisecond*1000);
#endif
}

//FUNC  Sleep，微秒级
//PARAM microsecond：Sleep的微秒数
static void SleepMicroSecond(size_t microsecond)
{
#ifdef WINDOWS_PLATFORM
    LARGE_INTEGER m_liPerfFreq = {0};
    if (!QueryPerformanceFrequency(&m_liPerfFreq))
    {
        return;
    }
    LARGE_INTEGER m_liPerfStart = {0};
    QueryPerformanceCounter(&m_liPerfStart);
    LARGE_INTEGER liPerfNow={0};
    while (true)
    {
        QueryPerformanceCounter(&liPerfNow);
        double time = ((liPerfNow.QuadPart-m_liPerfStart.QuadPart)*1000000)/(double)m_liPerfFreq.QuadPart;
        if (time >= microsecond)
        {
            break;
        }
    }
#else
    usleep(microsecond);
#endif
}

//FUNC  清理字符串串头部的空白，Tab，换行，回车
//IN    pStr: 待处理的串
//RET   整理后的串长度
static int ClearStrHead(char *pStr)
{
    if (pStr == NULL || *pStr == '\0')
    {
        return -1;
    }

    char *pSrc = pStr;
    while (*pSrc != '\0')
    {
        if (*pSrc == ' ' || *pSrc == '\r' || *pSrc == '\n' || *pSrc == '\t')
        {
            ++pSrc;
        }
        else
        {
            break;
        }
    }

    char *pDest = pStr;
    while (*pSrc != '\0')
    {
        if (pDest != pSrc)
        {
            *pDest++ = *pSrc++; 
        }
        else
        {
            pSrc++;
            pDest++;
        }        
    }
    *pDest = '\0';
    
    return pDest-pStr;
}

//FUNC  清理字符串串尾部的空白，Tab，换行，回车
//IN    pStr: 待处理的串
//RET   整理后的串长度
static int ClearStrTail(char *pStr)
{
    if (pStr == NULL || *pStr == '\0')
    {
        return -1;
    }

    size_t Len = strlen(pStr);
    while (Len > 0)
    {
        --Len;
        if (pStr[Len] == ' ' || pStr[Len] == '\r' || pStr[Len] == '\n' || pStr[Len] == '\t')
        {
            pStr[Len] = '\0';
        }
        else
        {
            ++Len;        	
            break;
        }
    }
    return Len;
}

//FUNC  清理字符串串头部和尾部的空白，Tab，换行，回车
//IN    pStr: 待处理的串
//RET   整理后的串长度
static int ClearStr(char *pStr)
{
    ClearStrHead(pStr);
    return ClearStrTail(pStr);
}

//FUNC  清理字符串串头部和尾部的空白，Tab，换行，回车
//IN    pStr: 待处理的串
//RET   整理后的串长度
static int ClearStr(char *pStr, int iStrLen)
{
    if (pStr == NULL)
    {
        return -1;
    }
    char chSave = pStr[iStrLen];
    pStr[iStrLen] = '\0';
    ClearStrHead(pStr);
    int iLen = ClearStrTail(pStr);
    pStr[iStrLen] = chSave;
    return iLen;
}

//FUNC  字符串比较，不区分大小写
static int StriCmp(const char *pStr1, const char *pStr2)
{
#ifdef WINDOWS_PLATFORM
    return _stricmp(pStr1, pStr2);
#else
    return strcasecmp(pStr1, pStr2);
#endif
}

//FUNC  指定长度的字符串比较，不区分大小写
static int StrniCmp(const char *pStr1, const char *pStr2, size_t StrLen)
{
#ifdef WINDOWS_PLATFORM
    return _strnicmp(pStr1, pStr2, StrLen);
#else
    return strncasecmp(pStr1, pStr2, StrLen);
#endif
}

//FUNC  在源字符串中查找模式串，区分中英文
//PARAM	pSrc：源串
//      pPtn，模式串
//RET	若找到，则返回模式串第一次出现的位置，否则返回NULL
static char *GBStrStr(const char *pSrc, const char *pPtn)
{
    if (pSrc == NULL || pPtn == NULL)
    {
        return NULL;
    }
    
    size_t Len = strlen(pPtn);
    if (Len < 1)
    {
        return NULL;
    }
   
    const char *p = pSrc;
    while (*p != '\0')
    {
        if (memcmp(p, pPtn, Len) == 0)
        {
            return (char *)p;
        }
        
        if (*p < 0)
        {
            p++;
            if (*p != '\0')
            {
                p++;
            }
        }
        else
        {
            p++;
        }        
    }
    
    return NULL;
}

//FUNC  在源字符串中查找模式串，不区分大小写
//PARAM	pSrc：源串
//      pPtn，模式串
//RET	若找到，则返回模式串第一次出现的位置，否则返回NULL
static char *StriStr(const char *pSrc, const char *pPtn)
{
    if (pSrc == NULL || pPtn == NULL)
    {
        return NULL;
    }
    
    size_t Len = strlen(pPtn);
    if (Len < 1)
    {
        return NULL;
    }
    
    char ch = *pPtn;
    char chOtherCase = ch;
    if (ch >= 'a' && ch <= 'z')
    {
        chOtherCase = ch - 'a' + 'A';
    }
    else if (ch > 'A' && ch <= 'Z')
    {
        chOtherCase = ch - 'A' + 'a';
    }
    
    const char *p = pSrc;
    while (*p != '\0')
    {
        if (*p == ch || *p == chOtherCase)
        {
            if (nsWFPub::StrniCmp(p, pPtn, Len) == 0)
            {
                return (char *)p;
            }
        }
        
        p++;
    }
    
    return NULL;
}

//FUNC  在源字符串中查找模式串，不区分大小写，区分中英文
//PARAM	pSrc：源串
//      pPtn，模式串
//RET	若找到，则返回模式串第一次出现的位置，否则返回NULL
static char *GBStriStr(const char *pSrc, const char *pPtn)
{
    if (pSrc == NULL || pPtn == NULL)
    {
        return NULL;
    }
    
    size_t Len = strlen(pPtn);
    if (Len < 1)
    {
        return NULL;
    }
    
    char ch = *pPtn;
    char chOtherCase = ch;
    if (ch >= 'a' && ch <= 'z')
    {
        chOtherCase = ch - 'a' + 'A';
    }
    else if (ch > 'A' && ch <= 'Z')
    {
        chOtherCase = ch - 'A' + 'a';
    }
    
    const char *p = pSrc;
    while (*p != '\0')
    {
        if (*p == ch || *p == chOtherCase)
        {
            if (nsWFPub::StrniCmp(p, pPtn, Len) == 0)
            {
                return (char *)p;
            }
        }
        
        if (*p < 0)
        {
            p++;
            if (*p != '\0')
            {
                p++;
            }
        }
        else
        {
            p++;
        }        
    }
    
    return NULL;
}

//FUNC  将源字符串的逆序结果存储到目标Buf中
//PARAM pSrc：源字符串
//      SrcLen：源字符串长度
//      pDest：目标Buf
static void ReverseStr(const void *pSrc, size_t SrcLen, void *pDest)
{
    if (pSrc == NULL || pDest == NULL)
    {
      return;
    }
    char *p = (char *)pSrc;
    char *q = (char *)pDest;
    for (size_t i = 0, j = SrcLen-1; i < SrcLen; ++i, --j)
    {
        q[j] = p[i];
    }
}

//FUNC  将源字符串逆序，原地逆序
//PARAM pSrc：源字符串
//      SrcLen：源字符串长度
static void ReverseStr(void *pSrc, size_t SrcLen)
{
    if (pSrc == NULL)
    {
      return;
    }
    char *p = (char *)pSrc;
    for (size_t i = 0, j = SrcLen-1; i < j; ++i, --j)
    {
        char chTemp = p[i];
        p[i] = p[j];
        p[j] = chTemp;
    }
}

//FUNC  创建目录
//RET	创建成功返回0
static int MakeDir(const char *pDir)
{
    if (pDir == NULL)
    {
        return -10;
    }
    size_t DirLen = strlen(pDir);
    if (DirLen <= 0 || DirLen >= FILENAME_MAX)
    {
        return -20;
    }    

    //复制希望创建的目录
    char szFilename[FILENAME_MAX+4] = "";
    memcpy(szFilename, pDir, DirLen);
    szFilename[DirLen] = '\0';
    szFilename[DirLen+1] = '\0';
	
    //将所有的'\'转为'/'
    char *pCurrDir = szFilename;
    while (*pCurrDir != '\0')
    {
        if (*pCurrDir < 0)
        {
            pCurrDir += 2;
        }
        else
        {
            if (*pCurrDir == '\\')
            {
                *pCurrDir = '/';
            }
            ++pCurrDir;
        }
    }

    //逐级创建目录
    pCurrDir = szFilename;
    while (true)
    {
        pCurrDir = strchr(pCurrDir+1, '/');
        if (pCurrDir == NULL)
        {
#ifdef WINDOWS_PLATFORM
            _mkdir(szFilename);
#else
            mkdir(szFilename, S_IRWXO|S_IRWXU);
#endif
            return 0;
        }
        *pCurrDir = '\0';
#ifdef WINDOWS_PLATFORM
        _mkdir(szFilename);
#else
        mkdir(szFilename, S_IRWXO|S_IRWXU);
#endif
        *pCurrDir = '/';        
    }

    return 0;
}

//FUNC  得到文件大小
//PARAM pFilename：文件名
//      u64FileSize：文件大小
//RET   0：文件大小获取成功，其值存储在u64FileSize中
//      其他：文件大小获取失败
static int GetFileSize(const char *pFilename, uint64_t &u64FileSize)
{
    if (pFilename == NULL)
    {
        return -1;
    } 
    struct stat sFileInfo;
    int iRet = stat(pFilename, &sFileInfo);
    if (iRet != -1)
    {
        u64FileSize = sFileInfo.st_size;
        return 0;
    }
    else
    {
        return -10;
    }
}

//FUNC  得到文件大小
//PARAM pFilename：文件名
//      iFileSize：文件大小
//RET   0：文件大小获取成功，其值存储在iFileSize中
//      其他：文件大小获取失败
static int GetFileSize(const char *pFilename, int &iFileSize)
{
    uint64_t u64FileSize = 0;
    int iRet = GetFileSize(pFilename, u64FileSize);
    if (iRet != 0)
    {
        return iRet;
    }
    else
    {
        iFileSize = (int)u64FileSize;
        return 0;
    }
}

//FUNC  得到文件最后修改时间
//PARAM pFilename：文件名
//      tLastModTime：文件最后修改时间
//RET   0：最后修改时间获取成功，其值存储在tLastModTime中
//      其他：最后修改时间获取失败
static int GetFileLastModTime(const char *pFilename, time_t &tLastModTime)
{
    if (pFilename == NULL)
    {
        return -1;
    }
    struct stat sFileInfo;
    int iRet = stat(pFilename, &sFileInfo);
    if (iRet != -1)
    {
        tLastModTime = sFileInfo.st_mtime;
        return 0;
    }
    else
    {
        return -10;
    }
}

//FUNC  得到日期串
//PARAM pStr：存储日期串的Buf
//      tTime：时间值，将根据此值计算日期。若为0，则表示根据当前时间计算日期。
//NOTE  格式：yyyy/mm/dd
static void GetDateStr(char *pStr, time_t tTime = 0)
{
    if (pStr == NULL)
    {
        return;
    }
    if (tTime == 0)
    {
        tTime = time(NULL);
    }

    struct tm *pTime = localtime(&tTime);
    sprintf(pStr, "%04d/%02d/%02d",
        pTime->tm_year+1900, pTime->tm_mon+1, pTime->tm_mday);
}

//FUNC  得到日期串做文件名
//PARAM pStr：存储日期串的Buf
//      tTime：时间值，将根据此值计算日期。若为0，则表示根据当前时间计算日期。
//NOTE  格式：yyyymmdd
static void GetDateStrForFilename(char *pStr, time_t tTime = 0)
{
    if (pStr == NULL)
    {
        return;
    }
    if (tTime == 0)
    {
        tTime = time(NULL);
    }

    struct tm *pTime = localtime(&tTime);
    sprintf(pStr, "%04d%02d%02d",
        pTime->tm_year+1900, pTime->tm_mon+1, pTime->tm_mday);
}

//FUNC  得到时间串
//PARAM pStr：存储时间串的Buf
//      tTime：时间值，将根据此值计算时间。若为0，则表示根据当前时间计算时间。
//NOTE  格式：hh:mm:ss
static void GetTimeStr(char *pStr, time_t tTime = 0)
{
    if (pStr == NULL)
    {
        return;
    }	
    if (tTime == 0)
    {
        tTime = time(NULL);
    }

    struct tm *pTime = localtime(&tTime);
    sprintf(pStr, "%02d:%02d:%02d",
        pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
}

//FUNC  得到时间串做文件名
//PARAM pStr：存储时间串的Buf
//      tTime：时间值，将根据此值计算时间。若为0，则表示根据当前时间计算时间。
//NOTE  格式：hh:mm:ss
static void GetTimeStrForFilename(char *pStr, time_t tTime = 0)
{
    if (pStr == NULL)
    {
        return;
    }	
    if (tTime == 0)
    {
        tTime = time(NULL);
    }

    struct tm *pTime = localtime(&tTime);
    sprintf(pStr, "%02d%02d%02d",
        pTime->tm_hour, pTime->tm_min, pTime->tm_sec);
}

//FUNC  得到日期时间串
//PARAM pStr：存储日期时间串的Buf
//      tTime：时间值，将根据此值计算日期时间。若为0，则表示根据当前时间计算日期时间。
//NOTE  格式：yyyy/mm/dd_hh:mm:ss
static void GetDateTimeStr(char *pStr, time_t tTime = 0)
{
    if (pStr == NULL)
    {
        return;
    }	
    if (tTime == 0)
    {
        tTime = time(NULL);
    }

    GetDateStr(pStr, tTime);
    pStr[10] = '_';
    GetTimeStr(pStr+11, tTime);
}

//FUNC  得到日期时间串做文件名
//PARAM pStr：存储日期时间串的Buf
//      tTime：时间值，将根据此值计算日期时间。若为0，则表示根据当前时间计算日期时间。
//NOTE  格式：yyyymmdd_hhmmss
static void GetDateTimeStrForFilename(char *pStr, time_t tTime = 0)
{
    if (pStr == NULL)
    {
        return;
    }	
    if (tTime == 0)
    {
        tTime = time(NULL);
    }

    GetDateStr(pStr, tTime);
    pStr[8] = '_';
    GetTimeStr(pStr+9, tTime);
}


//FUNC  GetTickCount for Linux
static uint32_t GetTickCnt()
{
#ifdef WINDOWS_PLATFORM
    return GetTickCount();
#else
    struct timeval sTimeVal;
    if(gettimeofday(&sTimeVal, NULL) != 0)
    {
        return 0;
    }
    return sTimeVal.tv_sec*1000+sTimeVal.tv_usec/1000;
#endif
}


//FUNC  判断文件或目录是否存在
//PARAM pFile：文件或目录名
//RET   true：文件或目录存在
//      false：文件或目录不存在
static bool FileIsExisting(const char *pFile)
{
    if (pFile == NULL)
    {
        return false;
    }

    if (access(pFile, 0) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

//FUNC  尝试打开文件
//PARAM fp：文件指针
//      pFile：文件名
//      pMode：打开文件的模式
//      iMaxTryCount：最大尝试次数。仅当值为正的时候才有效。
//RET   0：打开成功，此时fp为相应的文件指针
//      其他：打开失败
static int TryOpenFile(FILE *&fp, const char *pFile, const char *pMode, int iMaxTryCount = -1)
{
    if (pFile == NULL || pMode == NULL)
    {
        return -1;
    }
    
    if (strchr(pMode, 'r') != NULL
        && !FileIsExisting(pFile))
    {//读方式打开一个不存在的文件
        return -10;
    }

    int iAlreadyTry = 0;
    while (true)
    {
        fp = fopen(pFile, pMode);
        if (fp != NULL)
        {
            return 0;
        }
        SleepMilliSecond(1);
        if (iMaxTryCount > 0 && ++iAlreadyTry >= iMaxTryCount)
        {//已到最大尝试次数
            return -100;
        }
    }
}

//FUNC  打开文件
//PARAM fp：文件指针
//      pFile：文件名
//      pMode：打开文件的模式
//RET   0：打开成功，此时fp为相应的文件指针
static int MustOpenFile(FILE *&fp, const char *pFile, const char *pMode)
{
    while (true)
    {
        int iRet = TryOpenFile(fp, pFile, pMode, 10000);
        if (iRet == 0)
        {
            return 0;
        }
        else
        {
            printf("MustOpenFile Failed: %s, %s, code=%d, LastError=%d\n",
                pFile, pMode, iRet, LastError());
        }
    }
}

//FUNC  尝试删除文件
//PARAM pFile：源文件名
//      iMaxTryCount：最大尝试次数。仅当值为正的时候才有效。
//RET   0：删除成功
//      其他：删除失败
static int TryRemoveFile(const char *pFile, int iMaxTryCount = -1)
{
    if (pFile == NULL)
    {
        return -1;
    }

    int iAlreadyTry = 0;
    while (true)
    {
        if (!FileIsExisting(pFile))
        {
            return 0;
        }

        if (remove(pFile) == 0)
        {
            return 0;
        }

        SleepMilliSecond(1);
        if (iMaxTryCount > 0 && ++iAlreadyTry >= iMaxTryCount)
        {
            return -100;
        }
    }
}

//FUNC  删除文件
//PARAM pFile：源文件名
//RET   0：删除成功
//      其他：删除失败
static int MustRemoveFile(const char *pFile)
{
    while (true)
    {
        int iRet = TryRemoveFile(pFile, 10000);
        if (iRet == 0)
        {
            return 0;
        }
        else
        {
            printf("MustRemoveFile Failed: %s, code=%d, LastError=%d\n",
                pFile, iRet, LastError());
        }
    }
}

//FUNC  尝试重命名文件
//PARAM pSrcFile：源文件名
//      pDestFile：目标文件名
//      iMaxTryCount：最大尝试次数。仅当值为正的时候才有效。
//RET   0：重命名成功
//      其他：重命名失败
static int TryRenameFile(const char *pSrcFile, const char *pDestFile, int iMaxTryCount = -1)
{
    if (pSrcFile == NULL || pDestFile == NULL)
    {
        return -1;
    }

    if (!FileIsExisting(pSrcFile))
    {//源不存在
        return -10;
    }

    if (FileIsExisting(pDestFile))
    {//目标已存在
        return -20;
    }

    int iAlreadyTry = 0;
    while (true)
    {
       	int iErr = 0;
        if ( (iErr = rename(pSrcFile, pDestFile)) == 0)
        {
            return 0;
        }
        SleepMilliSecond(1);
#if USE_MFS
        if (!FileIsExisting(pSrcFile) && FileIsExisting(pDestFile))
        {//MFS中，rename可能返回失败，但已经成功复制
            printf("rename() OK while it returns fault. err_rename:%d \n", iErr);
            return 0;
        }
#endif
        if (iMaxTryCount > 0 && ++iAlreadyTry >= iMaxTryCount)
        {
            return -100;
        }
    }
}

//FUNC  重命名文件
//PARAM pSrcFile：源文件名
//      pDestFile：目标文件名
//RET   0：重命名成功
//      其他：重命名失败
static int MustRenameFile(const char *pSrcFile, const char *pDestFile)
{
    while (true)
    {
        int iRet = TryRenameFile(pSrcFile, pDestFile, 10000);
        if (iRet == 0)
        {
            return 0;
        }
        else if (iRet == -20)
        {//目标已存在
            MustRemoveFile(pDestFile);
        }
        else
        {
            printf("MustRenameFile Failed: %s to %s, code=%d, LastError=%d\n",
                pSrcFile, pDestFile, iRet, LastError());
        }
    }
}

//FUNC  复制文件
//PARAM pSrcFile：源文件名
//      pDestFile：目标文件名
//RET   0：复制成功
//      其他：复制失败
static int CopyFile(const char *pSrcFile, const char *pDestFile)
{
    if (pSrcFile == NULL || pDestFile == NULL)
    {
        return -1;
    }

    if (!FileIsExisting(pSrcFile))
    {//源不存在
        return -10;
    }

    uint64_t u64FileSize = 0;
    int iRet = GetFileSize(pSrcFile, u64FileSize);
    if (iRet != 0)
    {//源大小获取失败
        return -20;
    }

    FILE *fpRB = fopen(pSrcFile, "rb");
    if (fpRB == NULL)
    {//源打不开
        return -30;
    }

    FILE *fpWB = fopen(pDestFile, "wb");
    if (fpWB == NULL)
    {//目标打不开
        fclose(fpRB);
        return -40;
    }

    iRet = 0;
    uint64_t u64CopySize = 0;
    char szTempBuf[1<<10] = "";
    while (u64CopySize < u64FileSize)
    {
        int iCopyLen = fread(szTempBuf, 1, 1<<10, fpRB);
        if (iCopyLen <= 0)
        {//read fail
            iRet = -100;
            break;
        }

        if (fwrite(szTempBuf, iCopyLen, 1, fpWB) != 1)
        {
            iRet = -200;
            break;
        }

        u64CopySize += iCopyLen;
    }

    fclose(fpRB);
    fclose(fpWB);

    return iRet;
}

//FUNC  按回车退出
static int QuitWithEnter()
{
    printf("请按回车退出");
    return getchar();
}

//FUNC  判断当前机器是否为小端表示
//RET   true：小端
//      false：大端
static bool IsLittleEndian()
{
    int iInt = 1;
    char cChar = 1;
    return *((char*)(&iInt)) == cChar;
}

//FUNC	遍历目录的类
class CTraversalDir
{
public:
    CTraversalDir()
    {
        memset(m_szDir, 0, sizeof(m_szDir));
        memset(m_szPattern, 0, sizeof(m_szPattern));
#ifdef WINDOWS_PLATFORM
        m_hFile = -1;
#else
        m_dp = NULL;
        m_entry = NULL;
#endif
    }
    ~CTraversalDir()
    {
#ifdef WINDOWS_PLATFORM
        if (m_hFile != -1)
        {
            _findclose(m_hFile);
            m_hFile = -1;
        }
#else
        if (m_dp != NULL)
        {
            closedir(m_dp);
            m_dp = NULL;
        }
#endif
    }
    //FUNC  初始化
    //PARAM pDir：将要遍历的目录
    int Init(const char *pDir)
    {
        if (pDir == NULL)
        {
            return -1;
        }
        strcpy(m_szDir, pDir);
        sprintf(m_szPattern, "%s/*.*", m_szDir);
        return 0;
    }
    //FUNC  是否有第一个名称
    bool HasFirst()
    {
#ifdef WINDOWS_PLATFORM
        m_hFile = _findfirst(m_szPattern, &m_sFileinfo);
        return (m_hFile != -1);
#else
        m_dp = opendir(m_szDir);
        if (m_dp == NULL)
        {
            return false;
        }
        m_entry = readdir(m_dp);
        return (m_entry != NULL);
#endif
    }
    //FUNC  是否有下一个名称
    bool HasNext()
    {
#ifdef WINDOWS_PLATFORM
        return _findnext(m_hFile, &m_sFileinfo) == 0;
#else
        m_entry = readdir(m_dp);
        return (m_entry != NULL); 
#endif
    }
    //FUNC  当前名称是否为目录
    bool IsDir()
    {
#ifdef WINDOWS_PLATFORM
        return (m_sFileinfo.attrib & _A_SUBDIR) != 0;
#else
        char szFullname[FILENAME_MAX] = "";
        sprintf(szFullname, "%s/%s", m_szDir, m_entry->d_name);
        stat(szFullname, &m_filestat);
        return S_ISDIR(m_filestat.st_mode);
#endif
    }
    //FUNC  得到当前的名称
    const char *GetName()
    {
#ifdef WINDOWS_PLATFORM
        return m_sFileinfo.name;
#else
        return m_entry->d_name;
#endif
    }
    //FUNC  是否有第一个文件
    bool HasFirstFile()
    {
        if (!HasFirst())
        {
            return false;
        }

        if (!IsDir())
        {
            return true;
        }

        return HasNextFile();
    }
    //FUNC  是否有下一个文件
    bool HasNextFile()
    {
        while (HasNext())
        {
            if (!IsDir())
            {
                return true;
            }
        }
        return false;
    }
    //FUNC  得到完整的名称
    int GetFullName(char *pFullName)
    {
        if (pFullName == NULL)
        {
            return -1;
        }
        return sprintf(pFullName, "%s/%s", m_szDir, GetName());
    }
protected:
private:
    char m_szDir[FILENAME_MAX];
    char m_szPattern[FILENAME_MAX];
#ifdef WINDOWS_PLATFORM
    long m_hFile;
    struct _finddata_t m_sFileinfo;
#else
    DIR *m_dp;
    struct dirent *m_entry;
    struct stat m_filestat;
#endif
};

//FUNC  从串得到无符号64位数
static uint64_t AtoUI64(const char *pStr)
{
    if (pStr == NULL)
    {
        return 0;
    }
    uint64_t u64 = 0;
    while ('0' <= *pStr && *pStr <= '9')
    {
        u64 = u64 * 10 + *pStr - '0';
        pStr++;
    }
    return u64;
}

//分析XML的类
class CXmlParser
{
public:
    //FUNC  得到Tag pair的前后位置
    int GetTagPairPos(const char *pParseBeg, const char *pParseEnd, const char *pTagName, char *&pTagBeg, char *&pTagEnd)
    {
        if (pParseBeg == NULL || pParseEnd == NULL || pTagName == NULL)
        {
            return -10;
        }
        pTagBeg = NULL;
        pTagEnd = NULL;
        
        int iTagLen = strlen(pTagName);
        char *p = (char *)pParseBeg;
        while (p < pParseEnd)
        {
            if (*p < 0)
            {
                p += 2;
                if (p > pParseEnd)
                {
                    return -100;
                }
            }
            else
            {
                if (*p == '<')
                {
                    if (memcmp(p+1, pTagName, iTagLen) == 0
                        && *(p+1+iTagLen) == '>')
                    {//是起始标记
                        p += 1+iTagLen+1;
                        
                        pTagBeg = p;
                    }
                    else if (pTagBeg != NULL
                        && *(p+1) == '/'
                        && memcmp(p+2, pTagName, iTagLen) == 0
                        && *(p+2+iTagLen) == '>')
                    {
                        pTagEnd = p;
                        
                        if (memcmp(pTagBeg, "<![CDATA[", 9) == 0
                            && memcmp(pTagEnd-3, "]]>", 3) == 0)
                        {
                            pTagBeg += 9;
                            pTagEnd -= 3;
                        }
                        return 0;
                    }
                    else
                    {
                        ++p;
                    }
                }
                else
                {
                    ++p;
                }
            }
        }
        
        return 0;
    }
protected:
private:
};


}   // end of namespace

#endif  // end of #ifndef
