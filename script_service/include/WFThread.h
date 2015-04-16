
#ifndef WF_THREAD_H
#define WF_THREAD_H

#include <stdio.h>

#ifdef WIN32
#define WINDOWS_PLATFORM
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#endif

#ifdef WINDOWS_PLATFORM //��WINDOWSƽ̨
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
    #include <process.h>
#else
    #include <pthread.h>
#endif

#include "./WFMem.h"

namespace nsWFThread
{

//�̹߳���
class CThread
{
public:
    CThread(size_t MaxThreadCount = 1024)
    {
        m_MaxThreadCount = MaxThreadCount;
        m_CurrThreadCount = 0;
        m_pThreadID = NULL;
    }
    //FUNC  �����߳�
    //PARAM ThreadFunc���̺߳���
    //      pParam���̺߳����Ĳ���
    //      ThreadCount���߳�����
    //RET   0�������ɹ�
    //      ����������ʧ�ܣ����ش�����
    int StartThread(void (*ThreadFunc)(void*), void *pParam = NULL, size_t ThreadCount = 1)
    {
        if (ThreadFunc == NULL)
        {
            return -1;
        }

        //�����߳�ID���飬���ڱ������������̵߳���Ϣ
        if (m_pThreadID == NULL)
        {
            m_pThreadID = nsWFMem::New<void*>(m_MaxThreadCount);
            if (m_pThreadID == NULL)
            {
                return -10;
            }
        }

        //�����߳�
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
    //FUNC  �ȴ��߳��˳�
    //RET   0�������߳��Ѿ��˳�
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
    size_t m_MaxThreadCount;    //����߳�����
    size_t m_CurrThreadCount;   //�߳�����
    void **m_pThreadID;         //�߳�ID�����׵�ַ�����ڱ������������̵߳���Ϣ
};

//FUNC  �õ��߳�ID
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

