#ifndef WF_MONITOR_H
#define WF_MONITOR_H

#include "WFPub.h"
#include "WFLock.h"
#include "WFSocket.h"

namespace nsWFMonitor
{
    
class CMonitor
{
    #define MAX_THREAD_DESCRIPTION_LEN 128
    #define MONITOR_LOG_DIR "MonitorDebugLog"
    
    enum ErrorType
    {
        TYPE_OK = 0,    //��������
        TYPE_MONITOR,   //monitor����
        TYPE_NETWORK,   //�������
        TYPE_SERVICE,   //�������
        TYPE_OTHER,     //��������
    };
    
    enum ErrorLevel
    {
        LEVEL_A = 1,    //����
        LEVEL_B,        //��Ҫ
        LEVEL_C,        //һ��
        LEVEL_D,        //����
        LEVEL_E,        //�ɺ���
    };

#pragma pack(4)
    struct SThreadInfo 
    {
        int iThreadID;
        time_t tUpdate;
        int iLineNum;
        char szThreadDesc[MAX_THREAD_DESCRIPTION_LEN+4];
    };
#pragma pack()
public:
    enum
    {
        INVALID_THREAD_ID   = 0,
            INVALID_MONITOR_ID  = -100,
    };

    CMonitor()
    {
        m_iMaxThreadCount = 0;
        m_paThreadInfo = NULL;
        m_iMaxOutTime = 0;
        m_bRun = false;
    }
    ~CMonitor()
    {
        Cleanup();
    }
    //FUNC  ���������
    //IN    iMaxThreadCount����Ҫ����ص�����߳�����
    //      iMonitorPort����ط���ļ����˿�
    //      iMaxOutTime����ط���ı�����ֵ����λΪ��
    //RET   0�������ɹ�������������ʧ�ܣ����ش������
    int Startup(int iMaxThreadCount, int iMonitorPort, int iMaxOutTime, const char *pLogDir = NULL)
    {
        const char *pDir = pLogDir;
        if (pDir == NULL)
        {
            pDir = ".";
        }
        int iRet = m_Log.Init(pDir, "monitor");
        if (iRet != 0)
        {
            printf("Init Monitor Log Failed, code=%d\n", iRet);
            return -1;
        }
        
        if (iMaxThreadCount <= 0)
        {
            m_Log.LPrintf(true, "MaxThreadCount Error: count=%d!\n", iMaxThreadCount);
            return -10;
        }
        m_iMaxThreadCount = iMaxThreadCount+1;  //+1 for MonitorThread
        m_paThreadInfo = nsWFMem::New<SThreadInfo>(m_iMaxThreadCount);
        if (m_paThreadInfo == NULL)
        {
            m_Log.LPrintf(true, "Alloc ThreadInfo Failed!\n");
            return -11;
        }
        memset(m_paThreadInfo, 0, sizeof(SThreadInfo)*m_iMaxThreadCount);
        
        if (iMonitorPort <= 0)
        {
            m_Log.LPrintf(true, "MonitorPort Error: port=%d!\n", iMonitorPort);
            return -20;
        }
        
        if (iMaxOutTime <= 0)
        {
            m_Log.LPrintf(true, "MaxOutTime Error: outtime=%d!\n", iMaxOutTime);
            return -30;
        }
        m_iMaxOutTime = iMaxOutTime;
        
//         m_bRun =  true;
//         iRet = m_MonitorThread.StartThread(MonitorThread, this);
//         if (iRet != 0)
//         {
//             m_Log.LPrintf(true, "Startup Monitor Thread Failed: ret=%d\n", iRet);
//             return -100;
//         }
        
//         iRet = m_MonitorServer.Startup(iMonitorPort);
//         if(iRet != 0)
//         {
//             m_Log.LPrintf(true, "Startup Monitor Port Failed: port=%ld, ret=%ld\n",
//                 iMonitorPort, nsWFPub::LastError());
//             return -200;
//         }
        
        m_Log.LPrintf(true, "Startup Monitor Succeed: Port=%d, MaxThread=%d, OutTime=%ds!\n", 
            iMonitorPort, iMaxThreadCount, m_iMaxOutTime);
        return 0;
    }
    //FUNC  ����
    int Cleanup()
    {
        m_bRun = false;
        m_MonitorServer.Cleanup();
        m_MonitorThread.WaitForQuit();
        
        if (m_paThreadInfo != NULL)
        {
            delete[] m_paThreadInfo;
            m_paThreadInfo = NULL;
        }
        
        return 0;
    }
    //FUNC  ��¼
    //IN    iThreadID���߳�ID
    //      pThreadDesc��
    //RET   >=0����¼�ɹ�������MonitorID������UpdateStatus��LogOut
    //      <0����¼ʧ�ܣ����ش������
    int LogIn(int iThreadID, const char *pThreadDesc = "Thread")
    {
        if (iThreadID == INVALID_THREAD_ID)
        {//�߳�ID�Ƿ�
            return -10;
        }
        
        nsWFLock::CAutolock autolock(&m_lock);
        
        //����һ����߳��Ƿ��Ѿ�ע���
        int i = 0;
        for (i = 0; i < m_iMaxThreadCount; ++i)
        {
            SThreadInfo *pCurrInfo = &m_paThreadInfo[i];
            if (pCurrInfo->iThreadID == iThreadID)
            {//ThreadID�Ѵ��ڣ�ֱ�ӷ���MonitorID
                pCurrInfo->tUpdate = time(NULL);
                return i;   //���ص�ǰ�±���ΪMonitorID
            }
        }
        
        //Ѱ��һ�����е���Ϣ�ṹ�������ȥ
        for (i = 0; i < m_iMaxThreadCount; ++i)
        {
            SThreadInfo *pCurrInfo = &m_paThreadInfo[i];
            if (pCurrInfo->iThreadID == INVALID_THREAD_ID)
            {//ThreadID��Ч����ʾ��Ϣ�ṹ����
                pCurrInfo->iThreadID = iThreadID;
                pCurrInfo->tUpdate = time(NULL);
                strncpy(pCurrInfo->szThreadDesc, pThreadDesc, MAX_THREAD_DESCRIPTION_LEN);
                pCurrInfo->szThreadDesc[MAX_THREAD_DESCRIPTION_LEN] = '\0';
                return i;   //���ص�ǰ�±���ΪMonitorID
            }
        }
        
        return INVALID_MONITOR_ID;
    }
    //FUNC  �̸߳���״̬
    //IN    iMonitorID���߳���LogInʱ��ȡ��MonitorID
    //      iThreadID���߳�ID
    //RET   0�����³ɹ�������������ʧ�ܣ����ش������
    #define UpdateStatus(iMonitorID, iThreadID) _UpdateStatus(iMonitorID, iThreadID, __LINE__)
    int _UpdateStatus(int iMonitorID, int iThreadID, int iLineNum)
    {
        if (iMonitorID < 0)
        {//MonitorID�Ƿ�
            printf("Update:MonitorID is Error: ID=%d\n", iMonitorID);
            return -10;
        }
        
        if (iMonitorID >= m_iMaxThreadCount)
        {//MonitorID
            printf("Update:MonitorID is Error: ID=%d\n", iMonitorID);
            return -20;
        }
        
        nsWFLock::CAutolock autolock(&m_lock);
        
        SThreadInfo *pCurrInfo = &m_paThreadInfo[iMonitorID];
        if (pCurrInfo->iThreadID != iThreadID)
        {//ThreadID�Ѿ���Ч
            printf("Update:ThreadID is Invalid: ThreadID=%d, UpdateID=%d\n", pCurrInfo->iThreadID, iThreadID);
            return -30;
        }
        
        //ͨ��У�飬����ʱ��
        pCurrInfo->tUpdate = time(NULL);
        pCurrInfo->iLineNum = iLineNum;
        
        return 0;
    }
    //FUNC  �˳�
    //IN    iMonitorID���߳���LogInʱ��ȡ��MonitorID
    //      iThreadID���߳�ID
    //RET   0���˳��ɹ����������˳�ʧ�ܣ����ش������
    int LogOut(int iMonitorID, int iThreadID)
    {
        if (iMonitorID < 0)
        {//MonitorID�Ƿ�
            printf("LogOut:MonitorID is Error: ID=%d\n", iMonitorID);
            return -10;
        }
        
        if (iMonitorID >= m_iMaxThreadCount)
        {//MonitorID
            printf("LogOut:MonitorID is Error: ID=%d\n", iMonitorID);
            return -20;
        }
        
        nsWFLock::CAutolock autolock(&m_lock);
        
        SThreadInfo *pCurrInfo = &m_paThreadInfo[iMonitorID];
        if (pCurrInfo->iThreadID != iThreadID)
        {//ThreadID�Ѿ���Ч
            printf("LogOut:ThreadID is Invalid: ThreadID=%d, LogOutID=%d\n", pCurrInfo->iThreadID, iThreadID);
            return -30;
        }
        
        //��Ϊ��Ч
        pCurrInfo->iThreadID = INVALID_THREAD_ID;

        pCurrInfo->tUpdate = 0;
        pCurrInfo->iLineNum = 0;
        pCurrInfo->szThreadDesc[0] = '\0';
        
        return 0;
    }
protected:
    //FUNC  ����߳�
    static void MonitorThread(void *pParam)
    {
        CMonitor *pThis = (CMonitor *)pParam;
        
        const int32_t I_STATEBUFSIZE = 1<<20;
        char *lpszStateBuf = nsWFMem::New<char>(I_STATEBUFSIZE+4);
        if(lpszStateBuf == NULL)
        {
            pThis->m_Log.LPrintf(true, "Startup Thread Failed @ malloc memory\n");
            return;
        }
        memset(lpszStateBuf, 0, I_STATEBUFSIZE+4);
        char *lpszStateEnd = lpszStateBuf+I_STATEBUFSIZE;
        
        uint32_t uStateLen = 0;
        
        int iThreadID = nsWFThread::GetThreadID();
        int iMonitorID = pThis->LogIn(iThreadID, "MonitorThread");
        pThis->m_Log.LPrintf(true, "Startup Thread(%d) Succeed, MonitorID=%d\n", iThreadID, iMonitorID);
        
        size_t MonitorCount = 0;
        while (pThis->m_bRun)
        {
            pThis->UpdateStatus(iMonitorID, iThreadID);
            sock_descriptor sockConn = pThis->m_MonitorServer.AcceptConn();
            if (sockConn == sock_invalid)
            {
                nsWFPub::SleepMilliSecond(1);
                continue;
            }
            
			if (-2 == sockConn)
			{
				pThis->m_Log.LPrintf(false, "Monitor.Failed to AcceptConn, invalid sockconn\n");					
				nsWFPub::SleepMilliSecond(1);
                continue;
			}

            pThis->m_MonitorServer.SetTimeout(sockConn, 15000, 15000);
            
            char szTime[128] = "";
            nsWFPub::GetDateTimeStr(szTime);
            int iStatus = TYPE_OK;
            
            int iRet = pThis->m_MonitorServer.BlockingRecv(sockConn, lpszStateBuf, 4);
            if (iRet != 4)
            {
                pThis->m_MonitorServer.Close(sockConn);
                pThis->m_Log.LPrintf(false, "Monitor.Failed to BlockingRecv, errno=%d\n", nsWFPub::LastError());
                continue;
            }
            pThis->UpdateStatus(iMonitorID, iThreadID);

            char *lpszStr = lpszStateBuf;
            //��ʼ��
            memcpy(lpszStr, "MonitorP1 ", 10);
            lpszStr += 10;
            //�ճ�����
            lpszStr += 4;
            //�ճ�����
            char *lpszType = lpszStr;
            lpszStr += 4;
            //�ճ��ȼ�    
            char *lpszLevel = lpszStr;
            lpszStr += 4;
            //    
            pThis->UpdateStatus(iMonitorID, iThreadID);
            iRet = pThis->GetThreadState(lpszStr, lpszStateEnd-lpszStr, uStateLen);
            if(iRet > 0)
            {//���쳣
                if (uStateLen > 0)
                {
                    lpszStr += uStateLen;
                }
                    
                *(int32_t*)lpszType = TYPE_SERVICE;
                *(int32_t*)lpszLevel = LEVEL_A;
                    
                iStatus = TYPE_SERVICE;
            }
            else
            {//����
                *(int32_t*)lpszType = TYPE_OK;
                *(int32_t*)lpszLevel = LEVEL_B;
                    
                iStatus = TYPE_OK;
            }
            //�����    
            *(int32_t*)(lpszStateBuf+10) = lpszStr-lpszStateBuf-14;           
            pThis->UpdateStatus(iMonitorID, iThreadID);
            
            int32_t send_len = lpszStr-lpszStateBuf;
            int32_t ret = pThis->m_MonitorServer.BlockingSend(sockConn, lpszStateBuf, send_len);
            if (send_len != ret)
            {
                pThis->m_Log.LPrintf(false, "Monitor.Failed to BlockingSend[%d/%d], errno=%d\n", ret, send_len, nsWFPub::LastError());
            }

            pThis->UpdateStatus(iMonitorID, iThreadID);

            pThis->m_MonitorServer.Close(sockConn);
            pThis->UpdateStatus(iMonitorID, iThreadID);
            
            const char *c_apErrorStr[] = {"Running OK", "Monitor Error", "Network Fault", "Service Error", "Other Error"};
            
            pThis->m_Log.LPrintf(false, "Monitor.%u.Status=%s(%d)\n", 
                ++MonitorCount, c_apErrorStr[iStatus], iStatus);
        }
        
        delete[] lpszStateBuf;
        pThis->LogOut(iMonitorID, iThreadID);
        pThis->m_Log.LPrintf(true, "Monitor Thread Quit\n");
        return;
    }
    //FUNC  ��ȡ�߳�״̬
    //IN    lpszOutBuf�������ϢBuf
    //      i32OutBufSize�������ϢBuf����󳤶�
    //      u32OutBufLen�������Ϣ��ʵ�ʳ���
    //RET   �����쳣�̵߳�����
    int32_t GetThreadState(char * const lpszOutBuf, const int32_t i32OutBufSize, uint32_t &u32OutBufLen)
    {
        uint32_t u32Len=0;
        u32OutBufLen=0;
       
        int iErrorCount = 0;
        time_t tNow = time(NULL);
        nsWFLock::CAutolock autolock(&m_lock);
        for (int i = 0; i < m_iMaxThreadCount; ++i)
        {
            SThreadInfo *pCurrInfo = &m_paThreadInfo[i];
            if (pCurrInfo->iThreadID != INVALID_THREAD_ID
                && pCurrInfo->tUpdate+m_iMaxOutTime < tNow)
            {
                ++iErrorCount;
                m_Log.LPrintf(true, "[%d|Thread (%d|%s) after Line %d @ %d].",
                    iErrorCount, pCurrInfo->iThreadID, pCurrInfo->szThreadDesc,
                    pCurrInfo->iLineNum, pCurrInfo->tUpdate);
                                
                int iLen = snprintf(lpszOutBuf+u32OutBufLen, i32OutBufSize-u32OutBufLen,
                    "[%d|Thread (%d|%s) after Line %d @ %d].",
                    iErrorCount, pCurrInfo->iThreadID, pCurrInfo->szThreadDesc,
                    pCurrInfo->iLineNum, pCurrInfo->tUpdate);
                if (iLen > 0)
                {
                    u32OutBufLen += iLen;
                }
            }
        }

        return iErrorCount;
    }
private:
    int m_iMaxThreadCount;          //��ط���֧�ֵ�����߳���
    SThreadInfo *m_paThreadInfo;    //���ڱ����߳�״̬�������׵�ַ
    int m_iMaxOutTime;              //��ط���ı�����ֵ����λΪ��

    bool m_bRun;                            //�߳����б�־
    nsWFSocket::CTcpServer m_MonitorServer; //��ط���
    nsWFThread::CThread m_MonitorThread;    //����߳�
    nsWFLock::CLock m_lock;                 //������

    nsWFLog::CDailyLog m_Log;               //��־
};

}   // end of namespace

#endif  // end of #ifndef

