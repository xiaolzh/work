
#ifndef WF_QUEUE_H
#define WF_QUEUE_H

#include "WFMem.h"

#include <assert.h>

namespace nsWFQueue
{

//���н�㣺Next|Prev|DataLen|Data
//���ݴӶ�β��ӣ��Ӷ�ͷ����
class CBlockListQueue
{
public:
    CBlockListQueue()
    {
        m_pHead = NULL;
        m_pTail = NULL;
        m_MaxQueueLen = 0;
        m_CurrQueueLen = 0;
        m_MaxDataSize = 0;
    }
    ~CBlockListQueue()
    {
        while (m_pHead != NULL)
        {
            char *pFree = m_pHead;
            m_pHead = *(char**)m_pHead;
            m_QueueNodeMem.Free(pFree);
        }
    }
    //FUNC  ��ʼ��
    //PARAM MaxDataSize�������д�ŵ����ݵ���󳤶�
    //      MaxQueueLen�������г���
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ�ܣ����ش�����
    //NOTE  ���н�������Next��Prev��DataLen��Data
    int Init(size_t MaxDataSize, size_t MaxQueueLen)
    {
        m_MaxDataSize = MaxDataSize;
        m_MaxQueueLen = MaxQueueLen;
        int iRet = m_QueueNodeMem.Init(m_MaxQueueLen, sizeof(char*)+sizeof(char*)+sizeof(size_t)+m_MaxDataSize);
        if (iRet != 0)
        {//�ڴ�����ʧ��
            return -10;
        }
        return 0;
    }
    //FUNC  �������
    //PARAM pData�������׵�ַ
    //      DataLen�����ݳ���
    //RET   0����ӳɹ�
    //      ���������ʧ��
    int EnQueue(void *pData, size_t DataLen)
    {
        if (pData == NULL || DataLen > m_MaxDataSize)
        {
            return -1;
        }

        if (m_CurrQueueLen >= m_MaxQueueLen)
        {//��������
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_lock);

        if (m_CurrQueueLen >= m_MaxQueueLen)
        {//��������
            return -10;
        }

        char *pNode = (char *)m_QueueNodeMem.Alloc();
        if (pNode == NULL)
        {//�ڴ治��
            assert(!"�ڴ治�᲻��");
            return -20;
        }
        char *p = pNode;
        *(char**)p = NULL;  p += sizeof(char*); //Next
        *(char**)p = m_pTail;   p += sizeof(char*); //Prev
        *(size_t*)p = DataLen;  p += sizeof(size_t);
        memcpy(p, pData, DataLen);

        if (m_pTail == NULL)
        {//����Ϊ��
            assert(m_pHead == NULL);
            m_pHead = m_pTail = pNode;
        }
        else
        {
            *(char**)m_pTail = pNode;
            m_pTail = pNode;
        }
        ++m_CurrQueueLen;

        return 0;
    }
    //FUNC  ���ݳ���
    //PARAM pData��������ݵ�Buf
    //      DataLen�����ݳ���
    //RET   0�����ӳɹ�����ʱDataLen�������ݳ���
    //      ����������ʧ�ܣ����ش�����
    int DeQueue(void *pData, size_t &DataLen)
    {
        if (pData == NULL)
        {
            return -1;
        }

        if (m_CurrQueueLen == 0)
        {//��ǰ����Ϊ��
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_lock);

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }
        assert(m_pTail != NULL);
        assert(m_pHead != NULL);

        char *p = m_pHead+sizeof(char*)+sizeof(char*);
        DataLen = *(size_t*)p;  p += sizeof(size_t);
        memcpy(pData, p, DataLen);

        char *pFree = m_pHead;
        m_pHead = *(char**)pFree; //Next
        
        if (m_pHead == NULL)
        {
            assert(m_CurrQueueLen == 1);
            m_pHead = m_pTail = NULL;
        }
        else
        {
            *(char**)(m_pHead+sizeof(char*)) = NULL; //Prev
        }

        if (pFree == m_pTail)
        {
            assert(m_CurrQueueLen == 1);
            m_pTail = m_pHead;
        }

        m_QueueNodeMem.Free(pFree);
        --m_CurrQueueLen;

        return 0;
    }
    //FUNC  �õ���ͷ����
    //PARAM pData��ָ���ͷ���ݵ�ָ��
    //      DataLen����ͷ���ݵĳ���
    //RET   0����ͷ���ݻ�ȡ�ɹ�����ʱpDataָ���ͷ���ݣ�DataLenΪ��ͷ���ݳ���
    //      ��������ȡʧ�ܣ����ش�����
    //NOTE  ��ͷ�����ǽ�Ҫ���ӵ�����
    int GetHeadData(void *&pData, size_t &DataLen)
    {
        if (m_CurrQueueLen == 0)
        {//��ǰ����Ϊ��
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_lock);

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }

        char *p = m_pHead+sizeof(char*)+sizeof(char*);
        DataLen = *(size_t*)p;  p += sizeof(size_t);
        pData = p;

        return 0;
    }
    //FUNC  ���ݳ���
    //RET   0����ͷ�����Ѿ�����
    //      ����������ʧ�ܣ����ش�����
    //NOTE  ֱ��ժ�¶�ͷ���ݲ����ա�����GetHeadData()���ʹ�á�
    int DeQueue()
    {
        if (m_CurrQueueLen == 0)
        {//��ǰ����Ϊ��
            return -10;
        }
        
        nsWFLock::CAutolock autolock(&m_lock);
        
        if (m_CurrQueueLen == 0)
        {
            return -10;
        }
        
        assert(m_pTail != NULL);
        assert(m_pHead != NULL);

        char *pFree = m_pHead;
        m_pHead = *(char**)m_pHead; //Next
        *(char**)(m_pHead+sizeof(char*)) = NULL;    //Prev
        --m_CurrQueueLen;
        if (m_pHead == NULL)
        {
            assert(m_CurrQueueLen == 0);
            m_pHead = m_pTail = NULL;
        } 
        m_QueueNodeMem.Free(pFree);

        return 0;   //�ɹ�
    }
    int RemoveFromQueueByData(void *pData, size_t DataLen)
    {
        if (pData == NULL || DataLen == 0)
        {
            return -1;
        }

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_lock);

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }

        char *pCurr = m_pHead;
        while (pCurr != NULL)
        {
            char *p = pCurr+sizeof(char*)+sizeof(char*);
            size_t CurrLen = *(size_t*)p;   p += sizeof(size_t);
            char *pCurrData = p;

            if (CurrLen == DataLen
                && memcmp(pCurrData, pData, DataLen) == 0)
            {//hit
                char *pNext = *(char**)pCurr;
                char *pPrev = *(char**)(pCurr+sizeof(char*));

                if (pNext != NULL)
                {
                    //pNext->pPrev = pCurr->pPrev
                    *(char**)(pNext+sizeof(char*)) = pPrev;
                }

                if (pPrev != NULL)
                {
                    //pPrev->pNext = pCurr->pNext
                    *(char**)pPrev = pNext;
                }

                if (m_pHead == pCurr)
                {
                    m_pHead = pNext;
                }

                if (m_pTail == pCurr)
                {
                    m_pTail = pPrev;
                }

                --m_CurrQueueLen;

                //free
                m_QueueNodeMem.Free(pCurr);
                return 0;
            }

            pCurr = *(char**)pCurr;
        }

        return -20;   //miss
    }
    //FUNC  �õ�������󳤶�
    size_t GetMaxQueueLen()
    {
        return m_MaxQueueLen;
    }
    //FUNC  �õ���ǰ���г���
    size_t GetCurrQueueLen()
    {
        return m_CurrQueueLen;
    }

    bool IsEmpty()
    {
        return (m_CurrQueueLen == 0);
    }

    bool IsFull()
    {
        return (m_MaxQueueLen == m_CurrQueueLen);
    }
protected:
private:
    char *m_pHead;  //��ͷָ�룬����ȡ������
    char *m_pTail;  //��βָ�룬���ڼӽ�����
    size_t m_MaxQueueLen;   //�����г���
    size_t m_CurrQueueLen;  //��ǰ���г���
    size_t m_MaxDataSize;   //���б������ݵ���󳤶�
    nsWFMem::CBlockMemory m_QueueNodeMem;   //���н���ڴ�������
    nsWFLock::CLock m_lock; //������
};

//
class CBlockListSet
{
	enum
	{
		MAX_QUEUE_COUNT = 1024,
	};
public:
	CBlockListSet()
	{
		m_iQueueCount = 0;
		m_iDefaultQueueDataSize = 0;
		m_iDefaultQueueLen = 0; 
		m_aQueue = NULL;
	}
	//
	~CBlockListSet()
	{
        int i = 0;
		for (i = 0; i < m_iQueueCount; ++i)
		{
			CBlockListQueue *pQueue = m_aQueue[i];
			if (pQueue != NULL)
			{
				delete pQueue;
			}
		}
	}
	//
	int Init(int iQueueCount, int iDefaultQueueDataSize, 
		int iDefaultQueueLen)
	{
		if (iQueueCount <= 0 || iQueueCount > MAX_QUEUE_COUNT
			|| iDefaultQueueDataSize <= 0
			|| iDefaultQueueLen <= 0)
		{
			return -1;
		}
		m_iQueueCount = iQueueCount;
		m_iDefaultQueueDataSize = iDefaultQueueDataSize;
		m_iDefaultQueueLen = iDefaultQueueLen;
		//
        m_aQueue = nsWFMem::New<CBlockListQueue*>(m_iQueueCount);
        if (m_aQueue == NULL)
        {
            return -2;
        }
        memset(m_aQueue, 0, sizeof(CBlockListQueue*)*m_iQueueCount);
		return 0;
	}
	//
	CBlockListQueue *GetQueueBySeq(int iQueueSeq)
	{
		if (iQueueSeq < 0 || iQueueSeq >= m_iQueueCount)
		{
			return NULL;
		}
		CBlockListQueue *pQueue = m_aQueue[iQueueSeq];
		if (pQueue != NULL)
		{//�����ж��У��򷵻���
			return pQueue;
		}
        nsWFLock::CAutolock autolock(&m_lockArray);
		pQueue = m_aQueue[iQueueSeq];
		if (pQueue != NULL)
		{//�����ж��У��򷵻���
			return pQueue;
		}
		//�������ڣ��򴴽�
        pQueue = nsWFMem::New<CBlockListQueue>();
        if (pQueue == NULL)
        {
            return NULL;
        }
        int iRet = pQueue->Init(m_iDefaultQueueDataSize, m_iDefaultQueueLen);
		if (iRet != 0)
		{
            delete pQueue;
            return NULL;
		}
		m_aQueue[iQueueSeq] = pQueue;
		return pQueue;
	}
	//
	int GetQueueCount()
	{
		return m_iQueueCount;
	}
    //
    int GetMaxQueueLen()
    {
        return m_iDefaultQueueLen;
    }
    //
    int GetCurrQueueLen(int iQueueSeq)
    {
        if (iQueueSeq < 0 || iQueueSeq >= m_iQueueCount)
        {
            return -1;
        }
        CBlockListQueue *pQueue = m_aQueue[iQueueSeq];
		if (pQueue == NULL)
        {
            return 0;
        }
		return pQueue->GetCurrQueueLen();
    }
protected:
private:
	int m_iQueueCount;
	int m_iDefaultQueueDataSize;
	int m_iDefaultQueueLen; 
	//
	nsWFLock::CLock m_lockArray;
	CBlockListQueue **m_aQueue;
};

//
class CPriorityBlockQueue : public CBlockListSet
{
public:
    CPriorityBlockQueue()
    {
        m_iMinPriority = 0;
        m_iMaxPriority = 0;
    }
    ~CPriorityBlockQueue()
    {

    }
public:
    int Init(int iMinPriority, int iMaxPriority,
        int iDefaultQueueDataSize, int iDefaultQueueLen)
    {
        m_iMinPriority = iMinPriority;
        m_iMaxPriority = iMaxPriority;
        return CBlockListSet::Init(iMaxPriority-iMinPriority+1, iDefaultQueueDataSize, iDefaultQueueLen);
    }
    //FUNC  �������
    //PARAM pData�������׵�ַ
    //      DataLen�����ݳ���
    //RET   0����ӳɹ�
    //      ���������ʧ��
    int EnQueue(int iPriority, void *pData, size_t DataLen)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return -100;
        }
        
        return pQueue->EnQueue(pData, DataLen);
    }
    //FUNC  ���ݳ���
    //PARAM pData��������ݵ�Buf
    //      DataLen�����ݳ���
    //RET   0�����ӳɹ�����ʱDataLen�������ݳ���
    //      ����������ʧ�ܣ����ش�����
    int DeQueue(void *pData, size_t &DataLen)
    {
        CBlockListQueue *pQueue = NULL;
        for (int iPriorty = m_iMaxPriority; iPriorty >= m_iMinPriority; --iPriorty)
        {
            int iRet = DeQueue(iPriorty, pData, DataLen);
            if (iRet == 0)
            {//succeed
                return 0;
            }
        }

        return -10000;
    }
    int DeQueue(int iPriority, void *pData, size_t &DataLen)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return -100;
        }

        return pQueue->DeQueue(pData, DataLen);
    }
    //FUNC  �õ���ͷ����
    //PARAM pData��ָ���ͷ���ݵ�ָ��
    //      DataLen����ͷ���ݵĳ���
    //RET   0����ͷ���ݻ�ȡ�ɹ�����ʱpDataָ���ͷ���ݣ�DataLenΪ��ͷ���ݳ���
    //      ��������ȡʧ�ܣ����ش�����
    //NOTE  ��ͷ�����ǽ�Ҫ���ӵ�����
    int GetHeadData(int iPriority, void *&pData, size_t &DataLen)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return -100;
        }

        return pQueue->GetHeadData(pData, DataLen);
    }
    //FUNC  ���ݳ���
    //RET   0����ͷ�����Ѿ�����
    //      ����������ʧ�ܣ����ش�����
    //NOTE  ֱ��ժ�¶�ͷ���ݲ����ա�����GetHeadData()���ʹ�á�
    int DeQueue(int iPriority)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return -100;
        }

        return pQueue->DeQueue();
    }
    //
    int RemoveFromQueueByData(int iPriority, void *pData, size_t DataLen)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return -100;
        }

        return pQueue->RemoveFromQueueByData(pData, DataLen);
    }
    //FUNC  �õ�������󳤶�
    size_t GetMaxQueueLen(int iPriority)
    {
        return CBlockListSet::GetMaxQueueLen();
    }
    //FUNC  �õ���ǰ���г���
    size_t GetCurrQueueLen(int iPriority)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return 0;
        }
        return pQueue->GetCurrQueueLen();
    }
    //FUNC  �õ���ǰ���г���
    bool IsFull(int iPriority)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return 0;
        }
        return pQueue->IsFull();
    }
protected:
private:
    int m_iMinPriority;
    int m_iMaxPriority;
};

template<typename DataType>
class CCheckTimeoutList
{
#pragma pack(1)
    struct SNode 
    {
        time_t tExpired;
        DataType Data;
        SNode *pNext;
    };
#pragma pack()
public:
    CCheckTimeoutList()
    {
        m_CurrQueueLen = 0;
        m_MaxQueueLen = 0;
        m_pIdleNode = NULL;
        m_pHead = NULL;
    }
    ~CCheckTimeoutList()
    {

    }
public:
    int Init(size_t MaxQueueLen)
    {
        m_MaxQueueLen = MaxQueueLen;
        return 0;
    }
    int EnQueue(time_t tExpired, const DataType *pData)
    {
        if (pData == NULL)
        {
            return -1;
        }

        if (m_CurrQueueLen >= m_MaxQueueLen)
        {//��������
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_Lock);

        if (m_CurrQueueLen >= m_MaxQueueLen)
        {//��������
            return -10;
        }

        SNode *pNode = m_pIdleNode;
        if (pNode == NULL)
        {
            pNode = nsWFMem::New<SNode>();
            if (pNode == NULL)
            {
                return -20;
            }
        }
        else
        {
            m_pIdleNode = pNode->pNext;
        }

        pNode->tExpired = tExpired;
        memcpy(&pNode->Data, pData, sizeof(DataType));

        SNode *pCurr = m_pHead;
        SNode *pPrev = NULL;
        while (pCurr != NULL)
        {
            if (pCurr->tExpired > pNode->tExpired)
            {
                break;
            }

            pPrev = pCurr;
            pCurr = pPrev->pNext;
        }

        //����pCurr֮ǰ
        if (pPrev != NULL)
        {
            pPrev->pNext = pNode;
        }
        pNode->pNext = pCurr;
        
        //�޸���ָ��
        if (pCurr == m_pHead)
        {
            assert(pPrev == NULL);
            m_pHead = pNode;
        }

        ++m_CurrQueueLen;

        return 0;
    }
    int Check(time_t tNow, DataType *pData)
    {
        if (pData == NULL)
        {
            return -1;
        }

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_Lock);

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }

        assert(m_pHead != NULL);

        if (tNow <= m_pHead->tExpired)
        {//δ�й��ڵ�
            return -20;
        }

        SNode *pExpired = m_pHead;
        m_pHead = m_pHead->pNext;

        //��������
        memcpy(pData, &pExpired->Data, sizeof(DataType));

        //�ͷŽ��
        pExpired->pNext = m_pIdleNode;
        m_pIdleNode = pExpired;

        --m_CurrQueueLen;

        return 0;
    }
    int RemoveFromQueueByData(DataType *pData)
    {
        if (pData == NULL)
        {
            return -1;
        }

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_Lock);

        if (m_CurrQueueLen == 0)
        {
            return -10;
        }

        SNode *pCurr = m_pHead;
        SNode *pPrev = NULL;
        while (pCurr != NULL)
        {
            if (memcmp(&pCurr->Data, pData, sizeof(DataType)) == 0)
            {//hit
                if (pPrev == NULL)
                {
                    assert(pCurr == m_pHead);
                    m_pHead = pCurr->pNext;
                }
                else
                {
                    pPrev->pNext = pCurr->pNext;
                }

                //�ͷŽ��
                pCurr->pNext = m_pIdleNode;
                m_pIdleNode = pCurr;
                
                --m_CurrQueueLen;
                return 0;
            }

            pPrev = pCurr;
            pCurr = pPrev->pNext;
        }

        return -20;   //miss
    }
	//
    //FUNC  �õ�������󳤶�
    size_t GetMaxQueueLen()
    {
        return m_MaxQueueLen;
    }
    //FUNC  �õ���ǰ���г���
    size_t GetCurrQueueLen()
    {
        return m_CurrQueueLen;
    }
protected:
private:
    nsWFLock::CLock m_Lock;
    size_t m_CurrQueueLen;
    size_t m_MaxQueueLen;
private:
    SNode *m_pIdleNode;
    SNode *m_pHead;
};

}   // end of namespace


#endif  // end of #ifndef

