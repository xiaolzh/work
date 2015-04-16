
#ifndef WF_LOG_H
#define WF_LOG_H

#include "WFLock.h"
#include "WFPub.h"

#include <stdarg.h>
#include <assert.h>

namespace nsWFLog
{

//每天更换文件的日志
class CDailyLog
{
public:
    CDailyLog()
    {
        memset(m_szLogDir, 0, sizeof(m_szLogDir));
        memset(m_szPrefix, 0, sizeof(m_szPrefix));
        m_bNeedTimeStamp = true;
        m_FlushFreq = 1;
        m_SavePeriod = 30;
        m_fpA = NULL;
        m_SaveCount = 0;
        m_tLastOpenDay = 0;
        m_tLastOpenTime = 0;
    }
    //FUNC  初始化
    //PARAM pLogDir：日志目录
    //      pPrefix：日志前缀
    //      bNeedTimeStamp：是否需要时间戳
    //      FlushFreq：刷新频率，若为3，表示每写三次就强制刷新文件
    //      SavePeriod：保存周期，若为7，表示只保存最近7日内的日志文件
    //RET   0：初始化成功
    //      其他：初始化失败，返回错误码
    int Init(const char *pLogDir, const char *pPrefix, bool bNeedTimeStamp = true, size_t FlushFreq = 1, size_t SavePeriod = 30)
    {
        if (!nsWFPub::FileIsExisting(pLogDir))
        {//日志目录不存在
            return -10;
        }

        strcpy(m_szLogDir, pLogDir);
        strcpy(m_szPrefix, pPrefix);
        m_bNeedTimeStamp = bNeedTimeStamp;
        m_FlushFreq = FlushFreq;
        m_SavePeriod = SavePeriod;

        return 0;
    }
    //FUNC  记录日志的printf版本
    //PARAM bConsoleShow：是否显示在控制台上
    void LPrintf(bool bConsoleShow, const char *pFormat, ...)
    {
        char szFilename[FILENAME_MAX] = "";
        char szTimeStamp[64] = "";
        if (m_bNeedTimeStamp)
        {
            nsWFPub::GetDateTimeStr(szTimeStamp);
        }
        
        va_list ap;        
     
        nsWFLock::CAutolock autolock(&m_lock);

        time_t tNow = time(NULL);
        struct tm *pNow = localtime(&tNow);
        if (NULL == m_fpA)
        {
            sprintf(szFilename, "%s/%s_%04d%02d%02d.txt", 
                m_szLogDir, m_szPrefix,
                pNow->tm_year+1900, pNow->tm_mon+1, pNow->tm_mday);
            do
            {
                m_fpA = fopen(szFilename, "a");
            }
            while (NULL == m_fpA);
            m_tLastOpenTime = time(NULL); 
            m_tLastOpenDay = pNow->tm_mday;
        }
        assert(m_fpA != NULL);

        
        if (m_bNeedTimeStamp)
        {
            fprintf(m_fpA, "[%s]", szTimeStamp);
        }
        
        va_start(ap, pFormat);
        vfprintf(m_fpA, pFormat, ap);
        va_end(ap);

        ++m_SaveCount;
        TryFlushingData();
        
        if (bConsoleShow == true)
        {
            if (m_bNeedTimeStamp)
            {
                printf("[%s]", szTimeStamp);
            }
            va_start(ap, pFormat);
            vprintf(pFormat, ap);
            va_end(ap);
        }        		
    }
    //FUNC  强制刷新日志文件
    void FlushData()
    {
        if (m_fpA != NULL)
        {            
            fflush(m_fpA);
        }
    }
protected:
    //FUNC  尝试刷新数据
    void TryFlushingData()
    {
        time_t tNow = time(NULL);	
        if (m_tLastOpenTime+10*60 < tNow || m_SaveCount >= m_FlushFreq)
        {//打开文件超过十分钟 || 保存次数已经超过刷新周期
            
            fflush(m_fpA);      //刷新
            m_SaveCount = 0;    //保存次数清零
            
            //判断是否需要重新打开一个文件
            struct tm *pNow = localtime(&tNow);
            if (m_tLastOpenTime+10*60 < tNow || pNow->tm_mday != m_tLastOpenDay)
            {//打开文件超过十分钟 || 已经跨天
                
                fclose(m_fpA);
                m_fpA = NULL;
                                
                char szFilename[FILENAME_MAX] = "";
                sprintf(szFilename, "%s/%s_%04d%02d%02d.txt", 
                    m_szLogDir, m_szPrefix,
                    pNow->tm_year+1900, pNow->tm_mon+1, pNow->tm_mday);				
                do 
                {
                    m_fpA = fopen(szFilename, "a");
                }
                while (m_fpA == NULL);
                m_tLastOpenTime = tNow;
                m_tLastOpenDay = pNow->tm_mday;

                CleanOldLog();  //每打开一个新文件，就清理一次旧日志
            }
            assert(m_fpA != NULL);
        }
    }
    //FUNC  清理旧的日志文件
    void CleanOldLog()
    {
        char szFilename[FILENAME_MAX] = "";
        time_t tNow = time(NULL);
        tNow -= m_SavePeriod*86400;
        for (size_t i = 0; i < m_SavePeriod; ++i, tNow -= 86400)
        {
            struct tm *ptm = localtime(&tNow);
            sprintf(szFilename, "%s/%s_%04d%02d%02d.txt",
                m_szLogDir, m_szPrefix,
                ptm->tm_year+1900, ptm->tm_mon+1, ptm->tm_mday);
            remove(szFilename);
        }
    }
private:
    char m_szLogDir[FILENAME_MAX];  //日志目录
    char m_szPrefix[FILENAME_MAX];  //日志前缀
    bool m_bNeedTimeStamp;          //是否需要时间戳
    size_t m_FlushFreq;             //刷新频率
    size_t m_SavePeriod;            //保存周期
    nsWFLock::CLock m_lock;         //写文件的锁
    FILE *m_fpA;                    //文件追加指针
    size_t m_SaveCount;             //保存次数
    time_t m_tLastOpenDay;          //上次打开文件的时间，以天为单位
    time_t m_tLastOpenTime;         //上次打开文件的时间，以秒为单位
};

//尝试记录日志
class CTrySaveLog
{
public:
    CTrySaveLog()
    {
        memset(m_szLogDir, 0, sizeof(m_szLogDir));
        memset(m_szPrefix, 0, sizeof(m_szPrefix));
        m_bNeedTimeStamp = true;
        m_MaxLogSize = 100<<20;
    }
    //FUNC  初始化
    //PARAM pPrefix：日志前缀
    //      bNeedTimeStamp：是否需要时间戳
    //      MaxLogSize：日志文件的最大大小，若日志文件超过此大小，则会重命名当前文件，并使用一个新文件
    //RET   0：初始化成功
    //      其他：初始化失败
    int Init(const char *pPrefix, bool bNeedTimeStamp = true, size_t MaxLogSize = 100<<20)
    {
        if (pPrefix == NULL)
        {
            return -1;
        }

        strcpy(m_szPrefix, pPrefix);
        m_bNeedTimeStamp = bNeedTimeStamp;
        m_MaxLogSize = m_MaxLogSize;

        return 0;
    }
    int Init(const char *pLogDir, const char *pPrefix, bool bNeedTimeStamp = true, size_t MaxLogSize = 100<<20)
    {
        if (pLogDir == NULL || pPrefix == NULL)
        {
            return -1;
        }
        
        strcpy(m_szLogDir, pLogDir);
        return Init(pPrefix, bNeedTimeStamp, MaxLogSize);
    }
    void TryLPrintfKernel(const char *pDir, bool bConsoleShow, const char *pFormat, va_list ap)
    {
        if (pDir == NULL || !nsWFPub::FileIsExisting(pDir))
        {//目录不存在
            return;
        }
        
        char szTimeStamp[64] = "";
        if (m_bNeedTimeStamp)
        {
            nsWFPub::GetDateTimeStr(szTimeStamp);
        }
        
        char szFilename[FILENAME_MAX] = "";
        sprintf(szFilename, "%s/%s.txt", pDir, m_szPrefix);

        nsWFLock::CAutolock autolock(&m_lock);

        //记日志
        FILE *fp = fopen(szFilename, "a");
        if (NULL != fp)
        {
            if (m_bNeedTimeStamp)
            {
                fprintf(fp, "[%s]", szTimeStamp);
            }
            vfprintf(fp, pFormat, ap);

            fclose(fp);
        }
        
        //屏幕输出
        if (bConsoleShow == true)
        {
            if (m_bNeedTimeStamp)
            {
                printf("[%s]", szTimeStamp);
            }
            vprintf(pFormat, ap);
        }		
                
        //日志文件足够大，就重命名
        uint64_t u64FileSize = 0;
        int iRet = nsWFPub::GetFileSize(szFilename, u64FileSize);
        if (iRet == 0 && u64FileSize >= m_MaxLogSize)
        {
            char szDestFilename[FILENAME_MAX] = "";
            sprintf(szDestFilename, "%s.%d", szFilename, time(NULL));
            nsWFPub::TryRenameFile(szFilename, szDestFilename);
        }
    }
    void TryLPrintf(const char *pDir, bool bConsoleShow, const char *pFormat, ...)
    {
        va_list ap;
        va_start(ap, pFormat);
        TryLPrintfKernel(pDir, bConsoleShow, pFormat, ap);
        va_end(ap);
    }  
    void TryShowLPrintf(const char *pFormat, ...)
    {
        va_list ap;
        va_start(ap, pFormat);
        TryLPrintfKernel(m_szLogDir, true, pFormat, ap);
        va_end(ap);
    }  
    void TrySaveLPrintf(const char *pFormat, ...)
    {
        va_list ap;
        va_start(ap, pFormat);
        TryLPrintfKernel(m_szLogDir, false, pFormat, ap);
        va_end(ap);
    }    
protected:
private:
    char m_szLogDir[FILENAME_MAX];  //日志目录
    char m_szPrefix[FILENAME_MAX];  //日志前缀
    bool m_bNeedTimeStamp;          //是否需要时间戳
    size_t m_MaxLogSize;            //日志文件的最大长度
    nsWFLock::CLock m_lock;         //日志锁
};

}   // end of namespace

#endif  // end of #ifndef

