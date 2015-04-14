// UC_Allocator_Recycle.h

#ifndef _UC_ALLOCATOR_RECYCLE_H_
#define _UC_ALLOCATOR_RECYCLE_H_

#include "UH_Define.h"

#define DEF_MAX_BUF_SEG_NUM		1000000
#define DEF_PRE_BUF_BLK_NUM		1000

class UC_Allocator_Recycle
{
public:
	UC_Allocator_Recycle()
	{
		m_lUseMemNum = 0;
		m_lMaxMemNum = 0;

		m_lCurUseLen = 0;
		m_lBlockSize = 0;
		m_lBufMemLen = 0;

		m_lUseBufNum = 0;
		m_lMaxBufNum = 0;

		m_lStat_RecycleNum = 0;
		m_lStat_AllocNum = 0;

		m_szppBlockLibrary = NULL;
		m_szpRecycleLibrary = NULL;
	};

	~UC_Allocator_Recycle()
	{
		if(m_szppBlockLibrary == NULL)
			return;

		for(var_8 i = 0; i < m_lMaxBufNum; i++)
		{
			if(m_szppBlockLibrary[i])
			{
				delete[] m_szppBlockLibrary[i];
				m_szppBlockLibrary[i] = NULL;
			}
		}
		delete m_szppBlockLibrary;
		m_szppBlockLibrary = NULL;

		m_szpRecycleLibrary = NULL;
	};

	var_4 initMem(var_8 lBlockSize/* 每块buf大小 */, var_8 lMaxBlockNum = 0/* 最大buf块数 */, var_8 lPerAllocBlockNum = 0/* 每段"buffer段"buf块数 */)
	{
#ifdef CP_DEBUG_ZHL
		printf("#*#*#*#*  cp_debug: UC_Allocator_Recycle is open  #*#*#*#*\n");
#endif

		if(lBlockSize <= 0)
			return -1;

#ifdef CP_DEBUG_ZHL
		lBlockSize += sizeof(var_vd*) + sizeof(var_4) + sizeof(var_4);		
#else
		lBlockSize += sizeof(var_vd*);		
#endif

		if(lMaxBlockNum && lPerAllocBlockNum == 0)
		{
			m_lMaxBufNum = DEF_MAX_BUF_SEG_NUM; // 最大分配"buffer段"数量
			lPerAllocBlockNum = (lMaxBlockNum + m_lMaxBufNum - 1) / m_lMaxBufNum;			
		}
		else if(lMaxBlockNum == 0 && lPerAllocBlockNum)
		{
			m_lMaxBufNum = DEF_MAX_BUF_SEG_NUM; // 最大分配"buffer段"数量
		}
		else if(lMaxBlockNum == 0 && lPerAllocBlockNum == 0)
		{
			m_lMaxBufNum = DEF_MAX_BUF_SEG_NUM; // 最大分配"buffer段"数量
			lPerAllocBlockNum = DEF_PRE_BUF_BLK_NUM;
		}
		else
		{
			m_lMaxBufNum = (lMaxBlockNum + lPerAllocBlockNum - 1) / lPerAllocBlockNum;
		}

		m_lUseBufNum = -1;

		m_lMaxBlockNum = lMaxBlockNum;

		m_lUseMemNum = lPerAllocBlockNum;
		m_lMaxMemNum = lPerAllocBlockNum;

		m_lCurUseLen = 0;
		m_lBlockSize = lBlockSize;
		m_lBufMemLen = lPerAllocBlockNum * lBlockSize; // 每段"buffer段"大小
		
		m_szppBlockLibrary = NULL;
		m_szpRecycleLibrary = NULL;

		m_lStat_RecycleNum = 0;
		m_lStat_AllocNum = 0;

		return 0;
	};

	var_1* AllocMem()
	{
		m_cMutexLock.lock();

		if(m_szpRecycleLibrary)
		{
			var_1* p = m_szpRecycleLibrary + sizeof(void*);
			m_szpRecycleLibrary = *(var_1**)m_szpRecycleLibrary;

			m_lStat_RecycleNum--;
			
#ifdef CP_DEBUG_ZHL
			assert(*(var_4*)p == 0xFAFAFAFA && *(var_4*)(p + m_lBlockSize - sizeof(var_vd*) - sizeof(var_4)) == 0xFCFCFCFC);
			p += sizeof(var_4);
#endif

			m_cMutexLock.unlock();
			return p;
		}
		if(m_szppBlockLibrary == NULL)
		{
			m_szppBlockLibrary = new var_1*[m_lMaxBufNum];
			if(m_szppBlockLibrary == NULL)
			{
				m_cMutexLock.unlock();
				return NULL;
			}
			memset(m_szppBlockLibrary, 0, m_lMaxBufNum * sizeof(void*));
		}
		if(m_lUseMemNum == m_lMaxMemNum)
		{
			if(++m_lUseBufNum >= m_lMaxBufNum)
			{
				m_lUseBufNum--;

				m_cMutexLock.unlock();
				return NULL;
			}
			if(m_szppBlockLibrary[m_lUseBufNum] == NULL)
			{
				m_szppBlockLibrary[m_lUseBufNum] = new var_1[m_lBufMemLen];
				if(m_szppBlockLibrary[m_lUseBufNum] == NULL)
				{
					m_lUseBufNum--;

					m_cMutexLock.unlock();
					return NULL;
				}
			}
			m_lCurUseLen = 0;
			m_lUseMemNum = 0;
		}

		var_1* p = &m_szppBlockLibrary[m_lUseBufNum][m_lCurUseLen] + sizeof(void*);
		m_lCurUseLen += m_lBlockSize;

		m_lUseMemNum++;
		m_lStat_AllocNum++;

#ifdef CP_DEBUG_ZHL
		*(var_4*)p = 0xFAFAFAFA;
		*(var_4*)(p + m_lBlockSize - sizeof(var_vd*) - sizeof(var_4)) = 0xFCFCFCFC;
		p += sizeof(var_4);
#endif

		m_cMutexLock.unlock();
		return p;
	};

	void FreeMem(var_1* buffer)
	{
#ifdef CP_DEBUG_ZHL
		buffer -= sizeof(var_4);
		assert(*(var_4*)buffer == 0xFAFAFAFA && *(var_4*)(buffer + m_lBlockSize - sizeof(var_vd*) - sizeof(var_4)) == 0xFCFCFCFC);
#endif

		m_cMutexLock.lock();

		buffer -= sizeof(void*);
		*(var_1**)buffer = m_szpRecycleLibrary;
		m_szpRecycleLibrary = buffer;

		m_lStat_RecycleNum++;

		m_cMutexLock.unlock();
	};

	void ResetAllocator()
	{
		m_cMutexLock.lock();

		m_szpRecycleLibrary = NULL;

		m_lUseMemNum = m_lMaxMemNum;

		m_lCurUseLen = 0;
		m_lUseBufNum = -1;
				
		m_lStat_RecycleNum = 0;
		m_lStat_AllocNum = 0;

		m_cMutexLock.unlock();
	};

	void FreeAllocator()
	{
		m_cMutexLock.lock();

		if(m_szppBlockLibrary)
		{
			for(var_8 i = 0; i < m_lMaxBufNum; i++)
			{
				if(m_szppBlockLibrary[i])
				{
					delete[] m_szppBlockLibrary[i];
					m_szppBlockLibrary[i] = NULL;
				}
			}
			delete m_szppBlockLibrary;
			m_szppBlockLibrary = NULL;
		}
			
		m_szpRecycleLibrary = NULL;		

		m_lUseMemNum = m_lMaxMemNum;

		m_lCurUseLen = 0;
		m_lUseBufNum = -1;

		m_lStat_RecycleNum = 0;
		m_lStat_AllocNum = 0;

		m_cMutexLock.unlock();
	};
	
	var_8 GetUseMemNum()
	{
		return m_lStat_AllocNum - m_lStat_RecycleNum;
	}

	// 得到当前版本号
	const var_1* GetVersion()
	{
		// v1.000 - 2008.08.26 - 初始版本
		// v1.100 - 2009.03.31 - 增加跨平台支持
		// v1.200 - 2011.04.25 - 增加线程安全
		return "v1.200";
	}

private:
	var_8 m_lUseMemNum;
	var_8 m_lMaxMemNum;
	
	var_8 m_lCurUseLen;
	var_8 m_lBlockSize;
	var_8 m_lBufMemLen;

	var_8 m_lUseBufNum;
	var_8 m_lMaxBufNum;

	var_8 m_lMaxBlockNum;

	var_8 m_lStat_RecycleNum;
	var_8 m_lStat_AllocNum;

	var_1** m_szppBlockLibrary;
	var_1*  m_szpRecycleLibrary;

	CP_MUTEXLOCK m_cMutexLock;
};

#endif // _UC_ALLOCATOR_RECYCLE_H_
