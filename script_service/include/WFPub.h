
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
#pragma message("��ǰ��Windowsϵͳ:x32")
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#pragma message("��ǰ��Windowsϵͳ:x64")
#endif

#ifdef WINDOWS_PLATFORM //��WINDOWSƽ̨

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

#else   //�Ƿ�WINDOWSƽ̨

    #pragma message("��ǰ����Unixϵͳ")
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

//FUNC  �õ�������
static int LastError()
{
#ifdef WINDOWS_PLATFORM
    return GetLastError();
#else
    return errno;
#endif
}

//FUNC  Sleep���뼶
//PARAM second��Sleep������
static void SleepSecond(size_t second)
{
#ifdef WINDOWS_PLATFORM
    Sleep(second*1000);
#else
    sleep(second);
#endif
}

//FUNC  Sleep�����뼶
//PARAM millisecond��Sleep�ĺ�����
static void SleepMilliSecond(size_t millisecond)
{
#ifdef WINDOWS_PLATFORM
    Sleep(millisecond);
#else
    usleep(millisecond*1000);
#endif
}

//FUNC  Sleep��΢�뼶
//PARAM microsecond��Sleep��΢����
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

//FUNC  �����ַ�����ͷ���Ŀհף�Tab�����У��س�
//IN    pStr: ������Ĵ�
//RET   �����Ĵ�����
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

//FUNC  �����ַ�����β���Ŀհף�Tab�����У��س�
//IN    pStr: ������Ĵ�
//RET   �����Ĵ�����
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

//FUNC  �����ַ�����ͷ����β���Ŀհף�Tab�����У��س�
//IN    pStr: ������Ĵ�
//RET   �����Ĵ�����
static int ClearStr(char *pStr)
{
    ClearStrHead(pStr);
    return ClearStrTail(pStr);
}

//FUNC  �����ַ�����ͷ����β���Ŀհף�Tab�����У��س�
//IN    pStr: ������Ĵ�
//RET   �����Ĵ�����
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

//FUNC  �ַ����Ƚϣ������ִ�Сд
static int StriCmp(const char *pStr1, const char *pStr2)
{
#ifdef WINDOWS_PLATFORM
    return _stricmp(pStr1, pStr2);
#else
    return strcasecmp(pStr1, pStr2);
#endif
}

//FUNC  ָ�����ȵ��ַ����Ƚϣ������ִ�Сд
static int StrniCmp(const char *pStr1, const char *pStr2, size_t StrLen)
{
#ifdef WINDOWS_PLATFORM
    return _strnicmp(pStr1, pStr2, StrLen);
#else
    return strncasecmp(pStr1, pStr2, StrLen);
#endif
}

//FUNC  ��Դ�ַ����в���ģʽ����������Ӣ��
//PARAM	pSrc��Դ��
//      pPtn��ģʽ��
//RET	���ҵ����򷵻�ģʽ����һ�γ��ֵ�λ�ã����򷵻�NULL
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

//FUNC  ��Դ�ַ����в���ģʽ���������ִ�Сд
//PARAM	pSrc��Դ��
//      pPtn��ģʽ��
//RET	���ҵ����򷵻�ģʽ����һ�γ��ֵ�λ�ã����򷵻�NULL
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

//FUNC  ��Դ�ַ����в���ģʽ���������ִ�Сд��������Ӣ��
//PARAM	pSrc��Դ��
//      pPtn��ģʽ��
//RET	���ҵ����򷵻�ģʽ����һ�γ��ֵ�λ�ã����򷵻�NULL
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

//FUNC  ��Դ�ַ������������洢��Ŀ��Buf��
//PARAM pSrc��Դ�ַ���
//      SrcLen��Դ�ַ�������
//      pDest��Ŀ��Buf
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

//FUNC  ��Դ�ַ�������ԭ������
//PARAM pSrc��Դ�ַ���
//      SrcLen��Դ�ַ�������
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

//FUNC  ����Ŀ¼
//RET	�����ɹ�����0
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

    //����ϣ��������Ŀ¼
    char szFilename[FILENAME_MAX+4] = "";
    memcpy(szFilename, pDir, DirLen);
    szFilename[DirLen] = '\0';
    szFilename[DirLen+1] = '\0';
	
    //�����е�'\'תΪ'/'
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

    //�𼶴���Ŀ¼
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

//FUNC  �õ��ļ���С
//PARAM pFilename���ļ���
//      u64FileSize���ļ���С
//RET   0���ļ���С��ȡ�ɹ�����ֵ�洢��u64FileSize��
//      �������ļ���С��ȡʧ��
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

//FUNC  �õ��ļ���С
//PARAM pFilename���ļ���
//      iFileSize���ļ���С
//RET   0���ļ���С��ȡ�ɹ�����ֵ�洢��iFileSize��
//      �������ļ���С��ȡʧ��
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

//FUNC  �õ��ļ�����޸�ʱ��
//PARAM pFilename���ļ���
//      tLastModTime���ļ�����޸�ʱ��
//RET   0������޸�ʱ���ȡ�ɹ�����ֵ�洢��tLastModTime��
//      ����������޸�ʱ���ȡʧ��
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

//FUNC  �õ����ڴ�
//PARAM pStr���洢���ڴ���Buf
//      tTime��ʱ��ֵ�������ݴ�ֵ�������ڡ���Ϊ0�����ʾ���ݵ�ǰʱ��������ڡ�
//NOTE  ��ʽ��yyyy/mm/dd
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

//FUNC  �õ����ڴ����ļ���
//PARAM pStr���洢���ڴ���Buf
//      tTime��ʱ��ֵ�������ݴ�ֵ�������ڡ���Ϊ0�����ʾ���ݵ�ǰʱ��������ڡ�
//NOTE  ��ʽ��yyyymmdd
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

//FUNC  �õ�ʱ�䴮
//PARAM pStr���洢ʱ�䴮��Buf
//      tTime��ʱ��ֵ�������ݴ�ֵ����ʱ�䡣��Ϊ0�����ʾ���ݵ�ǰʱ�����ʱ�䡣
//NOTE  ��ʽ��hh:mm:ss
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

//FUNC  �õ�ʱ�䴮���ļ���
//PARAM pStr���洢ʱ�䴮��Buf
//      tTime��ʱ��ֵ�������ݴ�ֵ����ʱ�䡣��Ϊ0�����ʾ���ݵ�ǰʱ�����ʱ�䡣
//NOTE  ��ʽ��hh:mm:ss
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

//FUNC  �õ�����ʱ�䴮
//PARAM pStr���洢����ʱ�䴮��Buf
//      tTime��ʱ��ֵ�������ݴ�ֵ��������ʱ�䡣��Ϊ0�����ʾ���ݵ�ǰʱ���������ʱ�䡣
//NOTE  ��ʽ��yyyy/mm/dd_hh:mm:ss
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

//FUNC  �õ�����ʱ�䴮���ļ���
//PARAM pStr���洢����ʱ�䴮��Buf
//      tTime��ʱ��ֵ�������ݴ�ֵ��������ʱ�䡣��Ϊ0�����ʾ���ݵ�ǰʱ���������ʱ�䡣
//NOTE  ��ʽ��yyyymmdd_hhmmss
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


//FUNC  �ж��ļ���Ŀ¼�Ƿ����
//PARAM pFile���ļ���Ŀ¼��
//RET   true���ļ���Ŀ¼����
//      false���ļ���Ŀ¼������
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

//FUNC  ���Դ��ļ�
//PARAM fp���ļ�ָ��
//      pFile���ļ���
//      pMode�����ļ���ģʽ
//      iMaxTryCount������Դ���������ֵΪ����ʱ�����Ч��
//RET   0���򿪳ɹ�����ʱfpΪ��Ӧ���ļ�ָ��
//      ��������ʧ��
static int TryOpenFile(FILE *&fp, const char *pFile, const char *pMode, int iMaxTryCount = -1)
{
    if (pFile == NULL || pMode == NULL)
    {
        return -1;
    }
    
    if (strchr(pMode, 'r') != NULL
        && !FileIsExisting(pFile))
    {//����ʽ��һ�������ڵ��ļ�
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
        {//�ѵ�����Դ���
            return -100;
        }
    }
}

//FUNC  ���ļ�
//PARAM fp���ļ�ָ��
//      pFile���ļ���
//      pMode�����ļ���ģʽ
//RET   0���򿪳ɹ�����ʱfpΪ��Ӧ���ļ�ָ��
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

//FUNC  ����ɾ���ļ�
//PARAM pFile��Դ�ļ���
//      iMaxTryCount������Դ���������ֵΪ����ʱ�����Ч��
//RET   0��ɾ���ɹ�
//      ������ɾ��ʧ��
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

//FUNC  ɾ���ļ�
//PARAM pFile��Դ�ļ���
//RET   0��ɾ���ɹ�
//      ������ɾ��ʧ��
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

//FUNC  �����������ļ�
//PARAM pSrcFile��Դ�ļ���
//      pDestFile��Ŀ���ļ���
//      iMaxTryCount������Դ���������ֵΪ����ʱ�����Ч��
//RET   0���������ɹ�
//      ������������ʧ��
static int TryRenameFile(const char *pSrcFile, const char *pDestFile, int iMaxTryCount = -1)
{
    if (pSrcFile == NULL || pDestFile == NULL)
    {
        return -1;
    }

    if (!FileIsExisting(pSrcFile))
    {//Դ������
        return -10;
    }

    if (FileIsExisting(pDestFile))
    {//Ŀ���Ѵ���
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
        {//MFS�У�rename���ܷ���ʧ�ܣ����Ѿ��ɹ�����
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

//FUNC  �������ļ�
//PARAM pSrcFile��Դ�ļ���
//      pDestFile��Ŀ���ļ���
//RET   0���������ɹ�
//      ������������ʧ��
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
        {//Ŀ���Ѵ���
            MustRemoveFile(pDestFile);
        }
        else
        {
            printf("MustRenameFile Failed: %s to %s, code=%d, LastError=%d\n",
                pSrcFile, pDestFile, iRet, LastError());
        }
    }
}

//FUNC  �����ļ�
//PARAM pSrcFile��Դ�ļ���
//      pDestFile��Ŀ���ļ���
//RET   0�����Ƴɹ�
//      ����������ʧ��
static int CopyFile(const char *pSrcFile, const char *pDestFile)
{
    if (pSrcFile == NULL || pDestFile == NULL)
    {
        return -1;
    }

    if (!FileIsExisting(pSrcFile))
    {//Դ������
        return -10;
    }

    uint64_t u64FileSize = 0;
    int iRet = GetFileSize(pSrcFile, u64FileSize);
    if (iRet != 0)
    {//Դ��С��ȡʧ��
        return -20;
    }

    FILE *fpRB = fopen(pSrcFile, "rb");
    if (fpRB == NULL)
    {//Դ�򲻿�
        return -30;
    }

    FILE *fpWB = fopen(pDestFile, "wb");
    if (fpWB == NULL)
    {//Ŀ��򲻿�
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

//FUNC  ���س��˳�
static int QuitWithEnter()
{
    printf("�밴�س��˳�");
    return getchar();
}

//FUNC  �жϵ�ǰ�����Ƿ�ΪС�˱�ʾ
//RET   true��С��
//      false�����
static bool IsLittleEndian()
{
    int iInt = 1;
    char cChar = 1;
    return *((char*)(&iInt)) == cChar;
}

//FUNC	����Ŀ¼����
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
    //FUNC  ��ʼ��
    //PARAM pDir����Ҫ������Ŀ¼
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
    //FUNC  �Ƿ��е�һ������
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
    //FUNC  �Ƿ�����һ������
    bool HasNext()
    {
#ifdef WINDOWS_PLATFORM
        return _findnext(m_hFile, &m_sFileinfo) == 0;
#else
        m_entry = readdir(m_dp);
        return (m_entry != NULL); 
#endif
    }
    //FUNC  ��ǰ�����Ƿ�ΪĿ¼
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
    //FUNC  �õ���ǰ������
    const char *GetName()
    {
#ifdef WINDOWS_PLATFORM
        return m_sFileinfo.name;
#else
        return m_entry->d_name;
#endif
    }
    //FUNC  �Ƿ��е�һ���ļ�
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
    //FUNC  �Ƿ�����һ���ļ�
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
    //FUNC  �õ�����������
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

//FUNC  �Ӵ��õ��޷���64λ��
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

//����XML����
class CXmlParser
{
public:
    //FUNC  �õ�Tag pair��ǰ��λ��
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
                    {//����ʼ���
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
