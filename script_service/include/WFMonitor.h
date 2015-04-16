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
        TYPE_OK = 0,    //运行正常
        TYPE_MONITOR,   //monitor错误
        TYPE_NETWORK,   //网络故障
        TYPE_SERVICE,   //服务错误
        TYPE_OTHER,     //其它错误
    };
    
    enum ErrorLevel
    {
        LEVEL_A = 1,    //严重
        LEVEL_B,        //重要
        LEVEL_C,        //一般
        LEVEL_D,        //调试
        LEVEL_E,        //可忽略
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
    //FUNC  启动监控器
    //IN    iMaxThreadCount：需要被监控的最大线程数量
    //      iMonitorPort：监控服务的监听端口
    //      iMaxOutTime：监控服务的报警阈值，单位为秒
    //RET   0：启动成功；其他：启动失败，返回错误代码
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
    //FUNC  清理
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
    //FUNC  登录
    //IN    iThreadID：线程ID
    //      pThreadDesc：
    //RET   >=0：登录成功，返回MonitorID，用于UpdateStatus和LogOut
    //      <0：登录失败，返回错误代码
    int LogIn(int iThreadID, const char *pThreadDesc = "Thread")
    {
        if (iThreadID == INVALID_THREAD_ID)
        {//线程ID非法
            return -10;
        }
        
        nsWFLock::CAutolock autolock(&m_lock);
        
        //先找一遍此线程是否已经注册过
        int i = 0;
        for (i = 0; i < m_iMaxThreadCount; ++i)
        {
            SThreadInfo *pCurrInfo = &m_paThreadInfo[i];
            if (pCurrInfo->iThreadID == iThreadID)
            {//ThreadID已存在，直接返回MonitorID
                pCurrInfo->tUpdate = time(NULL);
                return i;   //返回当前下标作为MonitorID
            }
        }
        
        //寻找一个空闲的信息结构，分配出去
        for (i = 0; i < m_iMaxThreadCount; ++i)
        {
            SThreadInfo *pCurrInfo = &m_paThreadInfo[i];
            if (pCurrInfo->iThreadID == INVALID_THREAD_ID)
            {//ThreadID无效，表示信息结构空闲
                pCurrInfo->iThreadID = iThreadID;
                pCurrInfo->tUpdate = time(NULL);
                strncpy(pCurrInfo->szThreadDesc, pThreadDesc, MAX_THREAD_DESCRIPTION_LEN);
                pCurrInfo->szThreadDesc[MAX_THREAD_DESCRIPTION_LEN] = '\0';
                return i;   //返回当前下标作为MonitorID
            }
        }
        
        return INVALID_MONITOR_ID;
    }
    //FUNC  线程更新状态
    //IN    iMonitorID：线程在LogIn时获取的MonitorID
    //      iThreadID：线程ID
    //RET   0：更新成功；其他：更新失败，返回错误代码
    #define UpdateStatus(iMonitorID, iThreadID) _UpdateStatus(iMonitorID, iThreadID, __LINE__)
    int _UpdateStatus(int iMonitorID, int iThreadID, int iLineNum)
    {
        if (iMonitorID < 0)
        {//MonitorID非法
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
        {//ThreadID已经无效
            printf("Update:ThreadID is Invalid: ThreadID=%d, UpdateID=%d\n", pCurrInfo->iThreadID, iThreadID);
            return -30;
        }
        
        //通过校验，更新时间
        pCurrInfo->tUpdate = time(NULL);
        pCurrInfo->iLineNum = iLineNum;
        
        return 0;
    }
    //FUNC  退出
    //IN    iMonitorID：线程在LogIn时获取的MonitorID
    //      iThreadID：线程ID
    //RET   0：退出成功；其他：退出失败，返回错误代码
    int LogOut(int iMonitorID, int iThreadID)
    {
        if (iMonitorID < 0)
        {//MonitorID非法
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
        {//ThreadID已经无效
            printf("LogOut:ThreadID is Invalid: ThreadID=%d, LogOutID=%d\n", pCurrInfo->iThreadID, iThreadID);
            return -30;
        }
        
        //置为无效
        pCurrInfo->iThreadID = INVALID_THREAD_ID;

        pCurrInfo->tUpdate = 0;
        pCurrInfo->iLineNum = 0;
        pCurrInfo->szThreadDesc[0] = '\0';
        
        return 0;
    }
protected:
    //FUNC  监控线程
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
            //起始串
            memcpy(lpszStr, "MonitorP1 ", 10);
            lpszStr += 10;
            //空出长度
            lpszStr += 4;
            //空出类型
            char *lpszType = lpszStr;
            lpszStr += 4;
            //空出等级    
            char *lpszLevel = lpszStr;
            lpszStr += 4;
            //    
            pThis->UpdateStatus(iMonitorID, iThreadID);
            iRet = pThis->GetThreadState(lpszStr, lpszStateEnd-lpszStr, uStateLen);
            if(iRet > 0)
            {//有异常
                if (uStateLen > 0)
                {
                    lpszStr += uStateLen;
                }
                    
                *(int32_t*)lpszType = TYPE_SERVICE;
                *(int32_t*)lpszLevel = LEVEL_A;
                    
                iStatus = TYPE_SERVICE;
            }
            else
            {//正常
                *(int32_t*)lpszType = TYPE_OK;
                *(int32_t*)lpszLevel = LEVEL_B;
                    
                iStatus = TYPE_OK;
            }
            //回填长度    
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
    //FUNC  获取线程状态
    //IN    lpszOutBuf：输出信息Buf
    //      i32OutBufSize：输出信息Buf的最大长度
    //      u32OutBufLen：输出信息的实际长度
    //RET   返回异常线程的数量
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
    int m_iMaxThreadCount;          //监控服务支持的最大线程数
    SThreadInfo *m_paThreadInfo;    //用于保存线程状态的数组首地址
    int m_iMaxOutTime;              //监控服务的报警阈值，单位为秒

    bool m_bRun;                            //线程运行标志
    nsWFSocket::CTcpServer m_MonitorServer; //监控服务
    nsWFThread::CThread m_MonitorThread;    //监控线程
    nsWFLock::CLock m_lock;                 //互斥锁

    nsWFLog::CDailyLog m_Log;               //日志
};

}   // end of namespace

#endif  // end of #ifndef

