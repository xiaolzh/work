
#ifndef WF_QUEUE_H
#define WF_QUEUE_H

#include "WFMem.h"

#include <assert.h>

namespace nsWFQueue
{

//队列结点：Next|Prev|DataLen|Data
//数据从队尾入队，从队头出队
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
    //FUNC  初始化
    //PARAM MaxDataSize：队列中存放的数据的最大长度
    //      MaxQueueLen：最大队列长度
    //RET   0：初始化成功
    //      其他：初始化失败，返回错误码
    //NOTE  队列结点包括：Next，Prev，DataLen，Data
    int Init(size_t MaxDataSize, size_t MaxQueueLen)
    {
        m_MaxDataSize = MaxDataSize;
        m_MaxQueueLen = MaxQueueLen;
        int iRet = m_QueueNodeMem.Init(m_MaxQueueLen, sizeof(char*)+sizeof(char*)+sizeof(size_t)+m_MaxDataSize);
        if (iRet != 0)
        {//内存申请失败
            return -10;
        }
        return 0;
    }
    //FUNC  数据入队
    //PARAM pData：数据首地址
    //      DataLen：数据长度
    //RET   0：入队成功
    //      其他：入队失败
    int EnQueue(void *pData, size_t DataLen)
    {
        if (pData == NULL || DataLen > m_MaxDataSize)
        {
            return -1;
        }

        if (m_CurrQueueLen >= m_MaxQueueLen)
        {//队列已满
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_lock);

        if (m_CurrQueueLen >= m_MaxQueueLen)
        {//队列已满
            return -10;
        }

        char *pNode = (char *)m_QueueNodeMem.Alloc();
        if (pNode == NULL)
        {//内存不足
            assert(!"内存不会不够");
            return -20;
        }
        char *p = pNode;
        *(char**)p = NULL;  p += sizeof(char*); //Next
        *(char**)p = m_pTail;   p += sizeof(char*); //Prev
        *(size_t*)p = DataLen;  p += sizeof(size_t);
        memcpy(p, pData, DataLen);

        if (m_pTail == NULL)
        {//队列为空
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
    //FUNC  数据出队
    //PARAM pData：存放数据的Buf
    //      DataLen：数据长度
    //RET   0：出队成功，此时DataLen中是数据长度
    //      其他：出队失败，返回错误码
    int DeQueue(void *pData, size_t &DataLen)
    {
        if (pData == NULL)
        {
            return -1;
        }

        if (m_CurrQueueLen == 0)
        {//当前队列为空
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
    //FUNC  得到队头数据
    //PARAM pData：指向队头数据的指针
    //      DataLen：队头数据的长度
    //RET   0：队头数据获取成功，此时pData指向队头数据，DataLen为队头数据长度
    //      其他：获取失败，返回错误码
    //NOTE  队头数据是将要出队的数据
    int GetHeadData(void *&pData, size_t &DataLen)
    {
        if (m_CurrQueueLen == 0)
        {//当前队列为空
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
    //FUNC  数据出队
    //RET   0：队头数据已经出队
    //      其他：出队失败，返回错误码
    //NOTE  直接摘下队头数据并回收。常与GetHeadData()配合使用。
    int DeQueue()
    {
        if (m_CurrQueueLen == 0)
        {//当前队列为空
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

        return 0;   //成功
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
    //FUNC  得到队列最大长度
    size_t GetMaxQueueLen()
    {
        return m_MaxQueueLen;
    }
    //FUNC  得到当前队列长度
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
    char *m_pHead;  //队头指针，用于取结点出队
    char *m_pTail;  //队尾指针，用于加结点入队
    size_t m_MaxQueueLen;   //最大队列长度
    size_t m_CurrQueueLen;  //当前队列长度
    size_t m_MaxDataSize;   //队列保存数据的最大长度
    nsWFMem::CBlockMemory m_QueueNodeMem;   //队列结点内存管理对象
    nsWFLock::CLock m_lock; //队列锁
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
		{//若已有队列，则返回它
			return pQueue;
		}
        nsWFLock::CAutolock autolock(&m_lockArray);
		pQueue = m_aQueue[iQueueSeq];
		if (pQueue != NULL)
		{//若已有队列，则返回它
			return pQueue;
		}
		//若不存在，则创建
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
    //FUNC  数据入队
    //PARAM pData：数据首地址
    //      DataLen：数据长度
    //RET   0：入队成功
    //      其他：入队失败
    int EnQueue(int iPriority, void *pData, size_t DataLen)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return -100;
        }
        
        return pQueue->EnQueue(pData, DataLen);
    }
    //FUNC  数据出队
    //PARAM pData：存放数据的Buf
    //      DataLen：数据长度
    //RET   0：出队成功，此时DataLen中是数据长度
    //      其他：出队失败，返回错误码
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
    //FUNC  得到队头数据
    //PARAM pData：指向队头数据的指针
    //      DataLen：队头数据的长度
    //RET   0：队头数据获取成功，此时pData指向队头数据，DataLen为队头数据长度
    //      其他：获取失败，返回错误码
    //NOTE  队头数据是将要出队的数据
    int GetHeadData(int iPriority, void *&pData, size_t &DataLen)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return -100;
        }

        return pQueue->GetHeadData(pData, DataLen);
    }
    //FUNC  数据出队
    //RET   0：队头数据已经出队
    //      其他：出队失败，返回错误码
    //NOTE  直接摘下队头数据并回收。常与GetHeadData()配合使用。
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
    //FUNC  得到队列最大长度
    size_t GetMaxQueueLen(int iPriority)
    {
        return CBlockListSet::GetMaxQueueLen();
    }
    //FUNC  得到当前队列长度
    size_t GetCurrQueueLen(int iPriority)
    {
        CBlockListQueue *pQueue = GetQueueBySeq(iPriority-m_iMinPriority);
        if (pQueue == NULL)
        {
            return 0;
        }
        return pQueue->GetCurrQueueLen();
    }
    //FUNC  得到当前队列长度
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
        {//队列已满
            return -10;
        }

        nsWFLock::CAutolock autolock(&m_Lock);

        if (m_CurrQueueLen >= m_MaxQueueLen)
        {//队列已满
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

        //加在pCurr之前
        if (pPrev != NULL)
        {
            pPrev->pNext = pNode;
        }
        pNode->pNext = pCurr;
        
        //修改首指针
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
        {//未有过期的
            return -20;
        }

        SNode *pExpired = m_pHead;
        m_pHead = m_pHead->pNext;

        //复制数据
        memcpy(pData, &pExpired->Data, sizeof(DataType));

        //释放结点
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

                //释放结点
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
    //FUNC  得到队列最大长度
    size_t GetMaxQueueLen()
    {
        return m_MaxQueueLen;
    }
    //FUNC  得到当前队列长度
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

