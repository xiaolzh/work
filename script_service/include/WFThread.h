
#ifndef WF_THREAD_H
#define WF_THREAD_H

#include <stdio.h>

#ifdef WIN32
#define WINDOWS_PLATFORM
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#endif

#ifdef WINDOWS_PLATFORM //是WINDOWS平台
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
#endif

#include "./WFMem.h"

namespace nsWFThread
{

//线程管理
class CThread
{
public:
    CThread(size_t MaxThreadCount = 1024)
    {
        m_MaxThreadCount = MaxThreadCount;
        m_CurrThreadCount = 0;
        m_pThreadID = NULL;
    }
    //FUNC  启动线程
    //PARAM ThreadFunc：线程函数
    //      pParam：线程函数的参数
    //      ThreadCount：线程数量
    //RET   0：启动成功
    //      其他：启动失败，返回错误码
    int StartThread(void (*ThreadFunc)(void*), void *pParam = NULL, size_t ThreadCount = 1)
    {
        if (ThreadFunc == NULL)
        {
            return -1;
        }

        //申请线程ID数组，用于保存已启动的线程的信息
        if (m_pThreadID == NULL)
        {
            m_pThreadID = nsWFMem::New<void*>(m_MaxThreadCount);
            if (m_pThreadID == NULL)
            {
                return -10;
            }
        }

        //启动线程
        size_t i = 0;
        for (i = 0; i < ThreadCount && m_CurrThreadCount < m_MaxThreadCount; ++i, ++m_CurrThreadCount)
        {
#ifdef WINDOWS_PLATFORM
            unsigned long ulRet = _beginthreadex(NULL, 0, (unsigned int (__stdcall *)(void *))ThreadFunc, pParam, 0, NULL);
            if (ulRet == -1L)
            {
                return -100;
            }
            m_pThreadID[m_CurrThreadCount] = (void*)ulRet;
#else
            pthread_t pid;
            int iRet = pthread_create(&pid, NULL, (void *(*)(void *))ThreadFunc, pParam);
            if (iRet != 0)
            {
                return -100;
            }
            m_pThreadID[m_CurrThreadCount] = (void*)pid;
#endif
            //printf("thread %d create, id is %u\n", i, (int)m_pThreadID[i]);
        }

        return 0;
    }
    //FUNC  等待线程退出
    //RET   0：所有线程已经退出
    int WaitForQuit()
    {
#ifdef WINDOWS_PLATFORM
        for (size_t i = 0; i < m_CurrThreadCount; ++i)
        {
            WaitForSingleObject((HANDLE)m_pThreadID[i], INFINITE);
            CloseHandle((HANDLE)m_pThreadID[i]);
            //printf("thread %u quit, id is %u\n", i, (int)m_pThreadID[i]);
        }
        m_CurrThreadCount = 0;
#else
        for (size_t i = 0; i < m_CurrThreadCount; ++i)
        {
            //printf("Wait Thread %u : %u Quit\n", i, (pthread_t)m_pThreadID[i]);
            pthread_join((pthread_t)m_pThreadID[i],NULL);
            //printf("thread %u quit, id is %u\n", i, (int)m_pThreadID[i]);
        }
        m_CurrThreadCount = 0;
#endif
        return 0;
    }
    size_t GetThreadCount()
    {
        return m_CurrThreadCount;
    }
protected:
private:
    size_t m_MaxThreadCount;    //最大线程数量
    size_t m_CurrThreadCount;   //线程数量
    void **m_pThreadID;         //线程ID数组首地址，用于保存已启动的线程的信息
};

//FUNC  得到线程ID
static int GetThreadID()
{
#ifdef WINDOWS_PLATFORM
    return GetCurrentThreadId();
#else
    return pthread_self();
#endif
}

}   // end of namespace

#endif  // end of #ifndef

