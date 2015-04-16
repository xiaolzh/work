
#ifndef WF_LOG_H
#define WF_LOG_H

#include "WFLock.h"
#include "WFPub.h"

#include <stdarg.h>
#include <assert.h>

namespace nsWFLog
{

//ÿ������ļ�����־
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
    //FUNC  ��ʼ��
    //PARAM pLogDir����־Ŀ¼
    //      pPrefix����־ǰ׺
    //      bNeedTimeStamp���Ƿ���Ҫʱ���
    //      FlushFreq��ˢ��Ƶ�ʣ���Ϊ3����ʾÿд���ξ�ǿ��ˢ���ļ�
    //      SavePeriod���������ڣ���Ϊ7����ʾֻ�������7���ڵ���־�ļ�
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ�ܣ����ش�����
    int Init(const char *pLogDir, const char *pPrefix, bool bNeedTimeStamp = true, size_t FlushFreq = 1, size_t SavePeriod = 30)
    {
        if (!nsWFPub::FileIsExisting(pLogDir))
        {//��־Ŀ¼������
            return -10;
        }

        strcpy(m_szLogDir, pLogDir);
        strcpy(m_szPrefix, pPrefix);
        m_bNeedTimeStamp = bNeedTimeStamp;
        m_FlushFreq = FlushFreq;
        m_SavePeriod = SavePeriod;

        return 0;
    }
    //FUNC  ��¼��־��printf�汾
    //PARAM bConsoleShow���Ƿ���ʾ�ڿ���̨��
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
    //FUNC  ǿ��ˢ����־�ļ�
    void FlushData()
    {
        if (m_fpA != NULL)
        {            
            fflush(m_fpA);
        }
    }
protected:
    //FUNC  ����ˢ������
    void TryFlushingData()
    {
        time_t tNow = time(NULL);	
        if (m_tLastOpenTime+10*60 < tNow || m_SaveCount >= m_FlushFreq)
        {//���ļ�����ʮ���� || ��������Ѿ�����ˢ������
            
            fflush(m_fpA);      //ˢ��
            m_SaveCount = 0;    //�����������
            
            //�ж��Ƿ���Ҫ���´�һ���ļ�
            struct tm *pNow = localtime(&tNow);
            if (m_tLastOpenTime+10*60 < tNow || pNow->tm_mday != m_tLastOpenDay)
            {//���ļ�����ʮ���� || �Ѿ�����
                
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

                CleanOldLog();  //ÿ��һ�����ļ���������һ�ξ���־
            }
            assert(m_fpA != NULL);
        }
    }
    //FUNC  ����ɵ���־�ļ�
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
    char m_szLogDir[FILENAME_MAX];  //��־Ŀ¼
    char m_szPrefix[FILENAME_MAX];  //��־ǰ׺
    bool m_bNeedTimeStamp;          //�Ƿ���Ҫʱ���
    size_t m_FlushFreq;             //ˢ��Ƶ��
    size_t m_SavePeriod;            //��������
    nsWFLock::CLock m_lock;         //д�ļ�����
    FILE *m_fpA;                    //�ļ�׷��ָ��
    size_t m_SaveCount;             //�������
    time_t m_tLastOpenDay;          //�ϴδ��ļ���ʱ�䣬����Ϊ��λ
    time_t m_tLastOpenTime;         //�ϴδ��ļ���ʱ�䣬����Ϊ��λ
};

//���Լ�¼��־
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
    //FUNC  ��ʼ��
    //PARAM pPrefix����־ǰ׺
    //      bNeedTimeStamp���Ƿ���Ҫʱ���
    //      MaxLogSize����־�ļ�������С������־�ļ������˴�С�������������ǰ�ļ�����ʹ��һ�����ļ�
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ��
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
        {//Ŀ¼������
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

        //����־
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
        
        //��Ļ���
        if (bConsoleShow == true)
        {
            if (m_bNeedTimeStamp)
            {
                printf("[%s]", szTimeStamp);
            }
            vprintf(pFormat, ap);
        }		
                
        //��־�ļ��㹻�󣬾�������
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
    char m_szLogDir[FILENAME_MAX];  //��־Ŀ¼
    char m_szPrefix[FILENAME_MAX];  //��־ǰ׺
    bool m_bNeedTimeStamp;          //�Ƿ���Ҫʱ���
    size_t m_MaxLogSize;            //��־�ļ�����󳤶�
    nsWFLock::CLock m_lock;         //��־��
};

}   // end of namespace

#endif  // end of #ifndef

