
#ifndef WF_MEM_H
#define WF_MEM_H

#include "WFInt.h"
#include "WFLock.h"

#include <stdio.h>
#include <memory.h>
#include <time.h>
#include <assert.h>

#pragma warning(disable:4786)

namespace nsWFMem
{

template<typename T>
static T *New(size_t ObjCount = 1)
{
    T *pRet = NULL;
    
    try
    {
        if (ObjCount == 1)
        {
            pRet = new T;
        }
        else
        {
            pRet = new T[ObjCount];
        }        
    }
    catch(...)
    {
        pRet = NULL;
    }
    return pRet;    
}

//重用内存管理器：变长申请，全部重用
class CReuseMemory
{
public:
    CReuseMemory()
    {
        m_pMemIndex = NULL;
        m_MaxMemIndex = 0;
        m_CurrMemIndex = 0;
        m_MemSize = 0;
        m_CurrAllocSize = 0;
    }
    ~CReuseMemory()
    {
        Reset();
    }
    //FUNC  初始化
    //PARAM MaxMemCount：最大内存块数量
    //      MemSize：每个内存块的大小
    //RET   0：初始化成功
    //      其他：初始化失败
    //NOTE  每次申请的内存不能超过MemSize
    int Init(size_t MaxMemCount, size_t MemSize)
    {
        if (MaxMemCount == 0 || MemSize == 0)
        {//参数错误
            return -1;
        }

        Reset();    //初始化之前需要重置所有成员变量的值

        m_MaxMemIndex = MaxMemCount;
        m_CurrMemIndex = 0;

        //申请内存块索引
        m_pMemIndex = nsWFMem::New<char*>(m_MaxMemIndex);
        if (m_pMemIndex == NULL)
        {//申请失败
            return -10;
        }
        memset(m_pMemIndex, 0, sizeof(void*)*m_MaxMemIndex);

        //申请第一个块
        m_MemSize = MemSize;
        m_CurrAllocSize = 0;

        m_pMemIndex[0] = nsWFMem::New<char>(m_MemSize);
        if (m_pMemIndex[0] == NULL)
        {//申请失败
            return -20;
        }

        return 0;
    }
    //FUNC  分配一个指定大小的内存
    //PARAM AllocSize：内存的大小
    //RET   NULL：分配失败
    //      其他：分配成功，为可用内存的首地址
    void *Alloc(size_t AllocSize)
    {
        if (AllocSize > m_MemSize)
        {//需要的内存大于MemSize
            return NULL;
        }

        nsWFLock::CAutolock autolock(&m_lock);
        
        if (m_CurrMemIndex >= m_MaxMemIndex)
        {//已无可用内存
            return NULL;
        }

        if (AllocSize + m_CurrAllocSize <= m_MemSize)
        {//当前块剩余空间足够
            void *pRet = m_pMemIndex[m_CurrMemIndex]+m_CurrAllocSize;
            m_CurrAllocSize += AllocSize;
            return pRet;
        }
        else
        {//当前块剩余空间不足
            //尝试使用下一块
            ++m_CurrMemIndex;
            m_CurrAllocSize = 0;
            if (m_CurrMemIndex >= m_MaxMemIndex)
            {//没有下一块
                return NULL;
            }

            if (m_pMemIndex[m_CurrMemIndex] == NULL)
            {//申请下一块
                m_pMemIndex[m_CurrMemIndex] = nsWFMem::New<char>(m_MemSize);
                if (m_pMemIndex[m_CurrMemIndex] == NULL)
                {//申请失败
                    return NULL;
                }
            }

            char *pRet = m_pMemIndex[m_CurrMemIndex];
            m_CurrAllocSize += AllocSize;
            return pRet;
        }
        return NULL;
    }
    //FUNC  统一释放
    //NOTE  只是将分配指针归零，并非将内存归还系统
    void Clear()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        m_CurrMemIndex = 0;
        m_CurrAllocSize = 0;
        return;
    }
    //FUNC  重置所有成员变量
    //NOTE  归还所有内存，所有变量置位默认值
    void Reset()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        if (m_pMemIndex != NULL)
        {
            for (size_t i = 0; i < m_MaxMemIndex; ++i)
            {
                if (m_pMemIndex[i] != NULL)
                {
                    delete[] m_pMemIndex[i];
                }
            }

            delete[] m_pMemIndex;
            m_pMemIndex = NULL;
        }

        m_MaxMemIndex = 0;
        m_CurrMemIndex = 0;
        m_MemSize = 0;
        m_CurrAllocSize = 0;
    }
protected:
private:
    char **m_pMemIndex;     //内存块索引
    size_t m_MaxMemIndex;   //内存块索引的最大位置
    size_t m_CurrMemIndex;  //当前内存块的索引
    size_t m_MemSize;       //每个内存块的大小
    size_t m_CurrAllocSize; //当前块已分配的内存大小
    nsWFLock::CLock m_lock; //分配锁
};

//块内存管理器
class CBlockMemory
{
#define ALLOC_GRANULARITY (10<<20)
public:
    CBlockMemory()
    {
        m_BlockSize = 0;
        m_pIdle = NULL;
        m_32AllocCount = 0;
    }
    ~CBlockMemory()
    {
        Reset();
    }
    //FUNC  初始化
    //PARAM MaxBlockCount：最大块数量
    //      BlockSize：块大小
    //RET   0：初始化成功
    //      其他：初始化失败
    int Init(size_t MaxBlockCount, size_t BlockSize)
    {
        Reset();    //初始化之前先重置成员变量

        size_t MaxMemCount = 0;
        size_t MemSize = 0;

        if (BlockSize >= ALLOC_GRANULARITY)
        {//块大小超过了默认的分配粒度
            MaxMemCount = MaxBlockCount;
            MemSize = BlockSize;
        }
        else if ((uint64_t)MaxBlockCount*BlockSize < ALLOC_GRANULARITY)
        {//所需内存总量很小
            MaxMemCount = 1;
            MemSize = MaxBlockCount*BlockSize;
        }
        else
        {
            size_t BlockCountInEachBatch = ALLOC_GRANULARITY/BlockSize;
            MaxMemCount = MaxBlockCount/BlockCountInEachBatch+1;
            MemSize = BlockCountInEachBatch*BlockSize;
        }
        m_BlockSize = BlockSize;
        return m_Memory.Init(MaxMemCount, MemSize);
    }
    //FUNC  初始化
    //PARAM MaxBatchCount：内存最大分配次数
    //      BlockCountInEachBatch：每次分配的内存中可以容纳的块数量
    //      BlockSize：块大小
    //RET   0：初始化成功
    //      其他：初始化失败
    int Init(size_t MaxBatchCount, size_t BlockCountInEachBatch, size_t BlockSize)
    {
        m_BlockSize = BlockSize;
        return m_Memory.Init(MaxBatchCount, BlockCountInEachBatch*BlockSize);
    }
    //FUNC  申请一个块
    //RET   NULL：申请失败
    //      其他：申请成功，为可用的内存首地址
    void *Alloc()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        if (m_pIdle != NULL)
        {//首先从空闲链表取
            void *pRet = m_pIdle;
            m_pIdle = *(void**)pRet;
            ++m_32AllocCount;
            return pRet;
        }
        else
        {
            void *pRet = m_Memory.Alloc(m_BlockSize);
            if (pRet != NULL)
            {
                ++m_32AllocCount;
            }            
            return pRet;
        }
    }
    //FUNC  释放一个块
    //PARAM pBlock：块地址
    void Free(void *pBlock)
    {
        if (pBlock != NULL)
        {//加入空闲链表
            nsWFLock::CAutolock autolock(&m_lock);
            *(void**)pBlock = m_pIdle;
            m_pIdle = pBlock;
            assert(m_32AllocCount > 0);
            --m_32AllocCount;
        }
        return;
    }
    //FUNC  释放所有块
    void FreeAll()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        m_Memory.Clear();
        m_pIdle = NULL;
        m_32AllocCount = 0;
    }
    //FUNC  重置成员变量
    //NOTE  归还已经申请的内存
    void Reset()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        m_BlockSize = 0;
        m_Memory.Reset();
        m_pIdle = NULL;
        m_32AllocCount = 0;
    }
    //FUNC  得到已分配的块数量
    size_t GetAllocCount()
    {
        return m_32AllocCount;
    }

    //FUNC  得到块大小
    size_t GetBlockSize()
    {
        return m_BlockSize;
    }
protected:
private:
    size_t m_BlockSize;         //每个块的大小
    CReuseMemory m_Memory;      //内存管理器
    void *m_pIdle;              //空闲链首地址
    nsWFLock::CLock m_lock;     //分配锁
    size_t m_32AllocCount;      //已分配的块数量
};

//按时间淘汰的内存
class CFreeByTimeMemory
{

#pragma pack(4)
    struct SDataBlockMeta 
    {
        char *pNext;
        size_t CurrAllocSize;
    };
    struct SPeriodHead 
    {
        SPeriodHead *pNext;
        size_t ExpiredPeriod;
        char *pBlock; 
    };
#pragma pack()

public:
    CFreeByTimeMemory()
    {
        m_PeriodLen = 0;
        m_FreeDelaySec = 0;
        m_pPeriodList = NULL;
        m_DataBlockSize = 0;
    }
    //FUNC  初始化
    //PARAM PeriodLen：时间段长度
    //      MaxPeriodCount：最大时间段数量
    //      FreeDelaySec：释放时的延时，单位为秒
    //      MaxDataBlockCount：最大数据块数量
    //      DataBlockSize：数据块大小
    //RET   0：初始化成功
    //      其他：初始化失败，返回错误码
    int Init(size_t PeriodLen, size_t MaxPeriodCount, size_t FreeDelaySec, size_t MaxDataBlockCount, size_t DataBlockSize)
    {
        m_PeriodLen = PeriodLen;
        m_FreeDelaySec = FreeDelaySec;
        m_DataBlockSize = DataBlockSize;
        int iRet = m_PeriodHeadMem.Init(MaxPeriodCount, sizeof(SPeriodHead));
        if (iRet != 0)
        {
            return -10;
        }
        iRet = m_DataBlockMem.Init(MaxDataBlockCount, m_DataBlockSize);
        if (iRet != 0)
        {
            return -20;
        }

        return 0;
    }
    //FUNC  分配
    //PARAM DataSize：分配的内存大小
    //      tExpiredTime：数据的过期时间
    //RET   NULL：分配失败
    //      其他：分配成功，为可用内存的首地址
    char *Alloc(size_t DataSize, time_t tExpiredTime)
    {
        if (DataSize+sizeof(SDataBlockMeta) > m_DataBlockSize)
        {//无论如何也存不下
            return NULL;
        }
        size_t ExpiredPeriod = tExpiredTime/m_PeriodLen;

        nsWFLock::CAutolock autolock(&m_lock);

        SPeriodHead *pCurrPeriod = m_pPeriodList;
        SPeriodHead *pPrePeriod = NULL;
        while (pCurrPeriod != NULL)
        {
            if (ExpiredPeriod == pCurrPeriod->ExpiredPeriod)
            {//当前数据属于当前周期
                return AddInCurrPeriod(pCurrPeriod, DataSize);
            }
            else if (ExpiredPeriod > pCurrPeriod->ExpiredPeriod)
            {//当前数据应该加在当前周期之前
                break;
            }
            pPrePeriod = pCurrPeriod;
            pCurrPeriod = pPrePeriod->pNext;
        }

        //尚不存在当前数据对应的周期
        SPeriodHead *pPeriodHead = (SPeriodHead *)m_PeriodHeadMem.Alloc();
        if (pPeriodHead == NULL)
        {
            return NULL;
        }
        pPeriodHead->pBlock = NULL;
        pPeriodHead->ExpiredPeriod = ExpiredPeriod;
        pPeriodHead->pNext = NULL;

        if (pCurrPeriod == NULL)
        {//当前结点在m_pPeriodList的队尾
            if (pPrePeriod == NULL)
            {//队列为空
                m_pPeriodList = pPeriodHead;
            }
            else
            {
                pPrePeriod->pNext = pPeriodHead;
            }
        }
        else if (pPrePeriod == NULL)
        {//当前结点在m_pPeriodList的队首
            pPeriodHead->pNext = m_pPeriodList;
            m_pPeriodList = pPeriodHead;
        }
        else
        {//当前结点在pPrePeriod和pCurrPeriod之间
            pPeriodHead->pNext = pCurrPeriod;
            pPrePeriod->pNext = pPeriodHead;
        }

        return AddInCurrPeriod(pPeriodHead, DataSize);
    }
    //FUNC  检查释放
    //NOTE  每调用一次，就检查一次
    void CheckFree()
    {
        time_t tNow = time(NULL);

        nsWFLock::CAutolock autolock(&m_lock);

        time_t tCurrTime = tNow-m_FreeDelaySec;
        size_t LastValidPeriod = tCurrTime/m_PeriodLen;

        SPeriodHead *pCurrPeriod = m_pPeriodList;
        SPeriodHead *pPrePeriod = NULL;
        while (pCurrPeriod != NULL)
        {
            if (pCurrPeriod->ExpiredPeriod < LastValidPeriod)
            {//当前周期已经失效，从此向后的数据都要释放
                break;
            }
            pPrePeriod = pCurrPeriod;
            pCurrPeriod = pPrePeriod->pNext;
        }

        if (pCurrPeriod == NULL)
        {//没有需要释放的
            return;
        }
        
        if (pPrePeriod == NULL)
        {//全部都要释放
            FreePeriodList(m_pPeriodList);
            m_pPeriodList = NULL;
        }
        else
        {//从pCurrPeriod开始释放
            FreePeriodList(pCurrPeriod);
            pPrePeriod->pNext = NULL;
        }

        return;
    }
protected:
    //FUNC  从当前周期中分配
    //PARAM pPeriod：当前周期结点
    //      DataSize：数据大小
    //RET   NULL：分配失败
    //      其他：分配成功，返回可用内存的首地址
    char *AddInCurrPeriod(SPeriodHead *pPeriod, size_t DataSize)
    {
        SDataBlockMeta *pBlockMeta = NULL;
        if (pPeriod->pBlock == NULL)
        {
            pPeriod->pBlock = (char *)m_DataBlockMem.Alloc();
            if (pPeriod->pBlock == NULL)
            {//内存不足，尝试释放
                CheckFree();
                return NULL;
            }
            pBlockMeta = (SDataBlockMeta *)pPeriod->pBlock;
            pBlockMeta->CurrAllocSize = sizeof(SDataBlockMeta);
            pBlockMeta->pNext = NULL;
        }
        else
        {
            char *pCurrBlock = pPeriod->pBlock;
            while (pCurrBlock != NULL)
            {
                pBlockMeta = (SDataBlockMeta *)pCurrBlock;
                if (pBlockMeta->CurrAllocSize+DataSize < m_DataBlockSize)
                {//找到了一块能够容纳当前数据的块
                    break;
                }
                pCurrBlock = pBlockMeta->pNext;
            }
            
            if (pCurrBlock == NULL)
            {//没有能容纳当前数据的块，则分配一块
                char *pNewBlock = (char *)m_DataBlockMem.Alloc();
                if (pNewBlock == NULL)
                {//内存不足，尝试释放
                    CheckFree();
                    return NULL;
                }
                pBlockMeta = (SDataBlockMeta *)pNewBlock;
                pBlockMeta->CurrAllocSize = sizeof(SDataBlockMeta);
                pBlockMeta->pNext = pPeriod->pBlock;
                pPeriod->pBlock = pNewBlock;                
            }
        }
            
        //至此，pBlockMeta所在的块就是能够容纳当前数据的块
        char *pRet = (char *)pBlockMeta+pBlockMeta->CurrAllocSize;
        pBlockMeta->CurrAllocSize += DataSize;
        return pRet;            
    }
    //FUNC  从当前结点释放周期链表
    //PARAM pPeriod：第一个过期结点
    //RET   0
    int FreePeriodList(SPeriodHead *pPeriod)
    {
        SPeriodHead *pCurrPeriod = pPeriod;
        while (pCurrPeriod != NULL)
        {
            SPeriodHead *pFree = pCurrPeriod;
            pCurrPeriod = pFree->pNext;

            //释放pFree
            if (pFree->pBlock != NULL)
            {//释放pFree下的所有Block
                char *pBlock = pFree->pBlock;
                while (pBlock != NULL)
                {
                    char *pFreeBlock = pBlock;
                    SDataBlockMeta *pBlockMeta = (SDataBlockMeta *)pFreeBlock;
                    pBlock = pBlockMeta->pNext;

                    m_DataBlockMem.Free(pFreeBlock);
                }                
            }

            m_PeriodHeadMem.Free(pFree);
        }

        return 0;
    }
private:
    size_t m_PeriodLen;             //周期大小，单位为秒
    size_t m_FreeDelaySec;          //检查释放时的延迟偏移
    SPeriodHead *m_pPeriodList;     //按照ExpiredPeriod递减的顺序进行组织
    CBlockMemory m_PeriodHeadMem;   //周期头结点内存
    CBlockMemory m_DataBlockMem;    //数据块内存
    size_t m_DataBlockSize;         //数据块大小
    nsWFLock::CLock m_lock;         //分配锁
};

}   // end of namespace

#endif  // end of #ifndef

