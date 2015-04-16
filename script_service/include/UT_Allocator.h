// UT_Allocator.h

#ifndef _UT_ALLOCATOR_H_
#define _UT_ALLOCATOR_H_

#include "UH_Define.h"

#define DEFAULTMAXBLOCKNUM	2000
#define DEFAULTMAXBLOCKLEN	1048576

template <class T>
class UT_Allocator
{
public:
	UT_Allocator(var_8 lMaxBlockNum = DEFAULTMAXBLOCKNUM, var_8 lMaxBlockLen = DEFAULTMAXBLOCKLEN)
	{
		if(lMaxBlockNum <= 0 || lMaxBlockLen <= 0)
		{
			m_lMaxBlockNum = DEFAULTMAXBLOCKNUM;
			m_lMaxBlockLen = DEFAULTMAXBLOCKLEN;
		}
		else
		{
			m_lMaxBlockNum = lMaxBlockNum;
			m_lMaxBlockLen = lMaxBlockLen;
		}
		m_lCurBlockNum = -1;
		m_lCurBlockLen = m_lMaxBlockLen;
		m_tppBlockLibrary = NULL;

		m_lAllAllocLen = 0;
	};

	~UT_Allocator()
	{
		if(m_tppBlockLibrary == NULL)
			return;
		for(var_8 i = 0; i < m_lMaxBlockNum; i++)
		{
			if(m_tppBlockLibrary[i])
				delete[] m_tppBlockLibrary[i];
		}
		delete[] m_tppBlockLibrary;
	};

	T* Allocate(var_8 lAllocLen = 1)
	{
		if(m_tppBlockLibrary == NULL)
		{
			m_tppBlockLibrary = new T*[m_lMaxBlockNum];
			if(m_tppBlockLibrary == NULL)
				return NULL;
			memset(m_tppBlockLibrary, 0, m_lMaxBlockNum * sizeof(void*));
		}

		if(lAllocLen > m_lMaxBlockLen - m_lCurBlockLen)
		{
			if(lAllocLen > m_lMaxBlockLen)
				return NULL;

			if(++m_lCurBlockNum >= m_lMaxBlockNum)
			{
				m_lCurBlockNum--;
				return NULL;
			}
			if(m_tppBlockLibrary[m_lCurBlockNum] == NULL)
			{
				m_tppBlockLibrary[m_lCurBlockNum] = new T[m_lMaxBlockLen];
				if(m_tppBlockLibrary[m_lCurBlockNum] == NULL)
				{
					m_lCurBlockNum--;
					return NULL;
				}
			}
			m_lCurBlockLen = 0;
		}

		T* tpTmp = &m_tppBlockLibrary[m_lCurBlockNum][m_lCurBlockLen];
		m_lCurBlockLen += lAllocLen;
		
		m_lAllAllocLen += lAllocLen;

		return tpTmp;
	};

	var_8 GetAllocateLen()
	{
		return m_lAllAllocLen;
	};

	var_8 GetUseBufferLen()
	{
		return m_lMaxBlockLen * m_lCurBlockNum + m_lCurBlockLen;
	};

	var_8 ResetAllocator()
	{
		m_lCurBlockNum = -1;
		m_lCurBlockLen = m_lMaxBlockLen;
		
		return 0;
	};

	var_8 FreeAllocator()
	{
		this->~UT_Allocator();

		m_lCurBlockNum = -1;
		m_lCurBlockLen = m_lMaxBlockLen;
		m_tppBlockLibrary = NULL;

		return 0;
	};

	// 得到当前版本号
	var_1* GetVersion()
	{
		// v1.000 - 2008.08.26 - 初始版本
		// v1.100 - 2009.03.31 - 增加跨平台支持
		return "v1.100";
	}

private:
	var_8 m_lMaxBlockNum;
	var_8 m_lMaxBlockLen;	
	var_8 m_lCurBlockNum;
	var_8 m_lCurBlockLen;
	var_8 m_lAllAllocLen;

	T**   m_tppBlockLibrary;	
};

#endif // _UT_ALLOCATOR_H_
