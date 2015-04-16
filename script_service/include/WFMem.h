
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

//�����ڴ���������䳤���룬ȫ������
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
    //FUNC  ��ʼ��
    //PARAM MaxMemCount������ڴ������
    //      MemSize��ÿ���ڴ��Ĵ�С
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ��
    //NOTE  ÿ��������ڴ治�ܳ���MemSize
    int Init(size_t MaxMemCount, size_t MemSize)
    {
        if (MaxMemCount == 0 || MemSize == 0)
        {//��������
            return -1;
        }

        Reset();    //��ʼ��֮ǰ��Ҫ�������г�Ա������ֵ

        m_MaxMemIndex = MaxMemCount;
        m_CurrMemIndex = 0;

        //�����ڴ������
        m_pMemIndex = nsWFMem::New<char*>(m_MaxMemIndex);
        if (m_pMemIndex == NULL)
        {//����ʧ��
            return -10;
        }
        memset(m_pMemIndex, 0, sizeof(void*)*m_MaxMemIndex);

        //�����һ����
        m_MemSize = MemSize;
        m_CurrAllocSize = 0;

        m_pMemIndex[0] = nsWFMem::New<char>(m_MemSize);
        if (m_pMemIndex[0] == NULL)
        {//����ʧ��
            return -20;
        }

        return 0;
    }
    //FUNC  ����һ��ָ����С���ڴ�
    //PARAM AllocSize���ڴ�Ĵ�С
    //RET   NULL������ʧ��
    //      ����������ɹ���Ϊ�����ڴ���׵�ַ
    void *Alloc(size_t AllocSize)
    {
        if (AllocSize > m_MemSize)
        {//��Ҫ���ڴ����MemSize
            return NULL;
        }

        nsWFLock::CAutolock autolock(&m_lock);
        
        if (m_CurrMemIndex >= m_MaxMemIndex)
        {//���޿����ڴ�
            return NULL;
        }

        if (AllocSize + m_CurrAllocSize <= m_MemSize)
        {//��ǰ��ʣ��ռ��㹻
            void *pRet = m_pMemIndex[m_CurrMemIndex]+m_CurrAllocSize;
            m_CurrAllocSize += AllocSize;
            return pRet;
        }
        else
        {//��ǰ��ʣ��ռ䲻��
            //����ʹ����һ��
            ++m_CurrMemIndex;
            m_CurrAllocSize = 0;
            if (m_CurrMemIndex >= m_MaxMemIndex)
            {//û����һ��
                return NULL;
            }

            if (m_pMemIndex[m_CurrMemIndex] == NULL)
            {//������һ��
                m_pMemIndex[m_CurrMemIndex] = nsWFMem::New<char>(m_MemSize);
                if (m_pMemIndex[m_CurrMemIndex] == NULL)
                {//����ʧ��
                    return NULL;
                }
            }

            char *pRet = m_pMemIndex[m_CurrMemIndex];
            m_CurrAllocSize += AllocSize;
            return pRet;
        }
        return NULL;
    }
    //FUNC  ͳһ�ͷ�
    //NOTE  ֻ�ǽ�����ָ����㣬���ǽ��ڴ�黹ϵͳ
    void Clear()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        m_CurrMemIndex = 0;
        m_CurrAllocSize = 0;
        return;
    }
    //FUNC  �������г�Ա����
    //NOTE  �黹�����ڴ棬���б�����λĬ��ֵ
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
    char **m_pMemIndex;     //�ڴ������
    size_t m_MaxMemIndex;   //�ڴ�����������λ��
    size_t m_CurrMemIndex;  //��ǰ�ڴ�������
    size_t m_MemSize;       //ÿ���ڴ��Ĵ�С
    size_t m_CurrAllocSize; //��ǰ���ѷ�����ڴ��С
    nsWFLock::CLock m_lock; //������
};

//���ڴ������
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
    //FUNC  ��ʼ��
    //PARAM MaxBlockCount����������
    //      BlockSize�����С
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ��
    int Init(size_t MaxBlockCount, size_t BlockSize)
    {
        Reset();    //��ʼ��֮ǰ�����ó�Ա����

        size_t MaxMemCount = 0;
        size_t MemSize = 0;

        if (BlockSize >= ALLOC_GRANULARITY)
        {//���С������Ĭ�ϵķ�������
            MaxMemCount = MaxBlockCount;
            MemSize = BlockSize;
        }
        else if ((uint64_t)MaxBlockCount*BlockSize < ALLOC_GRANULARITY)
        {//�����ڴ�������С
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
    //FUNC  ��ʼ��
    //PARAM MaxBatchCount���ڴ����������
    //      BlockCountInEachBatch��ÿ�η�����ڴ��п������ɵĿ�����
    //      BlockSize�����С
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ��
    int Init(size_t MaxBatchCount, size_t BlockCountInEachBatch, size_t BlockSize)
    {
        m_BlockSize = BlockSize;
        return m_Memory.Init(MaxBatchCount, BlockCountInEachBatch*BlockSize);
    }
    //FUNC  ����һ����
    //RET   NULL������ʧ��
    //      ����������ɹ���Ϊ���õ��ڴ��׵�ַ
    void *Alloc()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        if (m_pIdle != NULL)
        {//���ȴӿ�������ȡ
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
    //FUNC  �ͷ�һ����
    //PARAM pBlock�����ַ
    void Free(void *pBlock)
    {
        if (pBlock != NULL)
        {//�����������
            nsWFLock::CAutolock autolock(&m_lock);
            *(void**)pBlock = m_pIdle;
            m_pIdle = pBlock;
            assert(m_32AllocCount > 0);
            --m_32AllocCount;
        }
        return;
    }
    //FUNC  �ͷ����п�
    void FreeAll()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        m_Memory.Clear();
        m_pIdle = NULL;
        m_32AllocCount = 0;
    }
    //FUNC  ���ó�Ա����
    //NOTE  �黹�Ѿ�������ڴ�
    void Reset()
    {
        nsWFLock::CAutolock autolock(&m_lock);
        m_BlockSize = 0;
        m_Memory.Reset();
        m_pIdle = NULL;
        m_32AllocCount = 0;
    }
    //FUNC  �õ��ѷ���Ŀ�����
    size_t GetAllocCount()
    {
        return m_32AllocCount;
    }

    //FUNC  �õ����С
    size_t GetBlockSize()
    {
        return m_BlockSize;
    }
protected:
private:
    size_t m_BlockSize;         //ÿ����Ĵ�С
    CReuseMemory m_Memory;      //�ڴ������
    void *m_pIdle;              //�������׵�ַ
    nsWFLock::CLock m_lock;     //������
    size_t m_32AllocCount;      //�ѷ���Ŀ�����
};

//��ʱ����̭���ڴ�
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
    //FUNC  ��ʼ��
    //PARAM PeriodLen��ʱ��γ���
    //      MaxPeriodCount�����ʱ�������
    //      FreeDelaySec���ͷ�ʱ����ʱ����λΪ��
    //      MaxDataBlockCount��������ݿ�����
    //      DataBlockSize�����ݿ��С
    //RET   0����ʼ���ɹ�
    //      ��������ʼ��ʧ�ܣ����ش�����
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
    //FUNC  ����
    //PARAM DataSize��������ڴ��С
    //      tExpiredTime�����ݵĹ���ʱ��
    //RET   NULL������ʧ��
    //      ����������ɹ���Ϊ�����ڴ���׵�ַ
    char *Alloc(size_t DataSize, time_t tExpiredTime)
    {
        if (DataSize+sizeof(SDataBlockMeta) > m_DataBlockSize)
        {//�������Ҳ�治��
            return NULL;
        }
        size_t ExpiredPeriod = tExpiredTime/m_PeriodLen;

        nsWFLock::CAutolock autolock(&m_lock);

        SPeriodHead *pCurrPeriod = m_pPeriodList;
        SPeriodHead *pPrePeriod = NULL;
        while (pCurrPeriod != NULL)
        {
            if (ExpiredPeriod == pCurrPeriod->ExpiredPeriod)
            {//��ǰ�������ڵ�ǰ����
                return AddInCurrPeriod(pCurrPeriod, DataSize);
            }
            else if (ExpiredPeriod > pCurrPeriod->ExpiredPeriod)
            {//��ǰ����Ӧ�ü��ڵ�ǰ����֮ǰ
                break;
            }
            pPrePeriod = pCurrPeriod;
            pCurrPeriod = pPrePeriod->pNext;
        }

        //�в����ڵ�ǰ���ݶ�Ӧ������
        SPeriodHead *pPeriodHead = (SPeriodHead *)m_PeriodHeadMem.Alloc();
        if (pPeriodHead == NULL)
        {
            return NULL;
        }
        pPeriodHead->pBlock = NULL;
        pPeriodHead->ExpiredPeriod = ExpiredPeriod;
        pPeriodHead->pNext = NULL;

        if (pCurrPeriod == NULL)
        {//��ǰ�����m_pPeriodList�Ķ�β
            if (pPrePeriod == NULL)
            {//����Ϊ��
                m_pPeriodList = pPeriodHead;
            }
            else
            {
                pPrePeriod->pNext = pPeriodHead;
            }
        }
        else if (pPrePeriod == NULL)
        {//��ǰ�����m_pPeriodList�Ķ���
            pPeriodHead->pNext = m_pPeriodList;
            m_pPeriodList = pPeriodHead;
        }
        else
        {//��ǰ�����pPrePeriod��pCurrPeriod֮��
            pPeriodHead->pNext = pCurrPeriod;
            pPrePeriod->pNext = pPeriodHead;
        }

        return AddInCurrPeriod(pPeriodHead, DataSize);
    }
    //FUNC  ����ͷ�
    //NOTE  ÿ����һ�Σ��ͼ��һ��
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
            {//��ǰ�����Ѿ�ʧЧ���Ӵ��������ݶ�Ҫ�ͷ�
                break;
            }
            pPrePeriod = pCurrPeriod;
            pCurrPeriod = pPrePeriod->pNext;
        }

        if (pCurrPeriod == NULL)
        {//û����Ҫ�ͷŵ�
            return;
        }
        
        if (pPrePeriod == NULL)
        {//ȫ����Ҫ�ͷ�
            FreePeriodList(m_pPeriodList);
            m_pPeriodList = NULL;
        }
        else
        {//��pCurrPeriod��ʼ�ͷ�
            FreePeriodList(pCurrPeriod);
            pPrePeriod->pNext = NULL;
        }

        return;
    }
protected:
    //FUNC  �ӵ�ǰ�����з���
    //PARAM pPeriod����ǰ���ڽ��
    //      DataSize�����ݴ�С
    //RET   NULL������ʧ��
    //      ����������ɹ������ؿ����ڴ���׵�ַ
    char *AddInCurrPeriod(SPeriodHead *pPeriod, size_t DataSize)
    {
        SDataBlockMeta *pBlockMeta = NULL;
        if (pPeriod->pBlock == NULL)
        {
            pPeriod->pBlock = (char *)m_DataBlockMem.Alloc();
            if (pPeriod->pBlock == NULL)
            {//�ڴ治�㣬�����ͷ�
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
                {//�ҵ���һ���ܹ����ɵ�ǰ���ݵĿ�
                    break;
                }
                pCurrBlock = pBlockMeta->pNext;
            }
            
            if (pCurrBlock == NULL)
            {//û�������ɵ�ǰ���ݵĿ飬�����һ��
                char *pNewBlock = (char *)m_DataBlockMem.Alloc();
                if (pNewBlock == NULL)
                {//�ڴ治�㣬�����ͷ�
                    CheckFree();
                    return NULL;
                }
                pBlockMeta = (SDataBlockMeta *)pNewBlock;
                pBlockMeta->CurrAllocSize = sizeof(SDataBlockMeta);
                pBlockMeta->pNext = pPeriod->pBlock;
                pPeriod->pBlock = pNewBlock;                
            }
        }
            
        //���ˣ�pBlockMeta���ڵĿ�����ܹ����ɵ�ǰ���ݵĿ�
        char *pRet = (char *)pBlockMeta+pBlockMeta->CurrAllocSize;
        pBlockMeta->CurrAllocSize += DataSize;
        return pRet;            
    }
    //FUNC  �ӵ�ǰ����ͷ���������
    //PARAM pPeriod����һ�����ڽ��
    //RET   0
    int FreePeriodList(SPeriodHead *pPeriod)
    {
        SPeriodHead *pCurrPeriod = pPeriod;
        while (pCurrPeriod != NULL)
        {
            SPeriodHead *pFree = pCurrPeriod;
            pCurrPeriod = pFree->pNext;

            //�ͷ�pFree
            if (pFree->pBlock != NULL)
            {//�ͷ�pFree�µ�����Block
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
    size_t m_PeriodLen;             //���ڴ�С����λΪ��
    size_t m_FreeDelaySec;          //����ͷ�ʱ���ӳ�ƫ��
    SPeriodHead *m_pPeriodList;     //����ExpiredPeriod�ݼ���˳�������֯
    CBlockMemory m_PeriodHeadMem;   //����ͷ����ڴ�
    CBlockMemory m_DataBlockMem;    //���ݿ��ڴ�
    size_t m_DataBlockSize;         //���ݿ��С
    nsWFLock::CLock m_lock;         //������
};

}   // end of namespace

#endif  // end of #ifndef

