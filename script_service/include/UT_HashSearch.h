// UT_HashSearch.h

#ifndef _UT_HASH_SEARCH_H_
#define _UT_HASH_SEARCH_H_

#include "UH_Define.h"

#include "UT_Allocator.h"
#include "UC_Allocator_Recycle.h"

template <class T_Key>
class UT_HashSearch
{
public:
	UT_HashSearch()
	{
		m_lDataLen = -1;
		m_lAllDataNum = 0;
		
		m_lHashTableSize = 0;
		m_lpHashTable = NULL;

		m_lTravelIndex = -1;
		m_szpTravelPos = NULL;
				
		m_cpAllocator_FL = NULL;
		m_cpAllocator_VL = NULL;

		m_lIsInitSystem = 0;		
	}

	~UT_HashSearch()
	{
		if(m_lpHashTable)
			delete m_lpHashTable;

		if(m_cpAllocator_FL)
			delete m_cpAllocator_FL;
		if(m_cpAllocator_VL)
			delete m_cpAllocator_VL;
	}

	// pointer key (len) data
	var_4 InitHashSearch(var_4 lHashTableSize, var_4 lDataLen = 0)
	{
		m_lDataLen = lDataLen; // 附加数据长度，小于0表示非定长数据
		m_lAllDataNum = 0;

		m_lHashTableSize = lHashTableSize;
		m_lpHashTable = new var_1*[m_lHashTableSize];
		if(m_lpHashTable == NULL)
			return -1;

		memset(m_lpHashTable, 0, m_lHashTableSize * sizeof(var_vd*));
		
		if(m_lDataLen < 0)
		{
			m_cpAllocator_VL = new UT_Allocator<var_1>;
			if(m_cpAllocator_VL == NULL)
				return -1;
		}
		else
		{
			m_cpAllocator_FL = new UC_Allocator_Recycle;
			if(m_cpAllocator_FL == NULL)
				return -1;
			if(m_cpAllocator_FL->initMem(sizeof(var_vd*) + sizeof(T_Key) + m_lDataLen))
				return -1;
		}

		m_lTravelIndex = 0;
		m_szpTravelPos = m_lpHashTable[m_lTravelIndex];

		m_lIsInitSystem = 1;

		return 0;
	}

	var_4 AddKey_FL(T_Key tKey, var_vd* vpData = NULL, var_vd** vppRetBuf = NULL)
	{
		// 计算哈希表位置
		var_4 lIndex = (var_4)(tKey % m_lHashTableSize);
		if(lIndex < 0)
			lIndex *= -1;
		
		// 查找并放入哈希表
		var_1* p = NULL;
		if(m_lpHashTable[lIndex] == 0)
		{
			p = m_cpAllocator_FL->AllocMem();
			if(p == NULL)
				return -1;

			*(var_1**)p = 0;
			m_lpHashTable[lIndex] = p;
		}
		else
		{
			p = m_lpHashTable[lIndex];
			while(p)
			{
				if(*(T_Key*)(p + sizeof(var_vd*)) == tKey)
				{
					if(vppRetBuf)
					{
						if(m_lDataLen == 0)
							*vppRetBuf = NULL;
						else
							*vppRetBuf = (var_vd*)(p + sizeof(var_vd*) + sizeof(T_Key));
					}

					return 1;
				}
				p = *(var_1**)p;
			}

			p = m_cpAllocator_FL->AllocMem();
			if(p == NULL)
				return -1;

			*(var_1**)p = m_lpHashTable[lIndex];
			m_lpHashTable[lIndex] = p;
		}

		*(T_Key*)(p + sizeof(var_vd*)) = tKey;
		if(m_lDataLen > 0 && vpData)
			memcpy(p + sizeof(var_vd*) + sizeof(T_Key), vpData, m_lDataLen);
		
		m_lAllDataNum++;

		if(vppRetBuf)
		{
			if(m_lDataLen == 0)
				*vppRetBuf = NULL;
			else
				*vppRetBuf = (var_vd*)(p + sizeof(var_vd*) + sizeof(T_Key));
		}

		return 0;	
	}

	var_4 AddKey_VL(T_Key tKey, var_vd* vpData, var_4 lDataLen, var_vd** vppRetBuf = NULL, var_4* lpRetDataLen = NULL)
	{
		// 长度检查
		if(lDataLen < 0)
			return -1;

		// 计算哈希表位置
		var_4 lIndex = (var_4)(tKey % m_lHashTableSize);
		if(lIndex < 0)
			lIndex *= -1;
		
		// 查找并放入哈希表
		var_1* p = NULL;
		if(m_lpHashTable[lIndex] == 0)
		{
			p = m_cpAllocator_VL->Allocate(sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4) + lDataLen);
			if(p == NULL)
				return -1;

			*(var_1**)p = 0;
			m_lpHashTable[lIndex] = p;
		}
		else
		{
			p = m_lpHashTable[lIndex];
			while(p)
			{
				if(*(T_Key*)(p + sizeof(var_vd*)) == tKey)
				{
					if (vppRetBuf)
					{
						*lpRetDataLen = *(var_4*)(p + sizeof(var_vd*) + sizeof(T_Key));

						if(*lpRetDataLen == 0)
							*vppRetBuf = NULL;
						else
							*vppRetBuf = (var_vd*)(p + sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4));
					}
					return 1;
				}
				p = *(var_1**)p;
			}

			p = m_cpAllocator_VL->Allocate(sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4) + lDataLen);
			if(p == NULL)
				return -1;

			*(var_1**)p = m_lpHashTable[lIndex];
			m_lpHashTable[lIndex] = p;
		}

		*(T_Key*)(p + sizeof(var_vd*)) = tKey;		
		*(var_4*)(p + sizeof(var_vd*) + sizeof(T_Key)) = lDataLen;
		if(lDataLen > 0)
			memcpy(p + sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4), vpData, lDataLen);
		
		m_lAllDataNum++;
		if (vppRetBuf)
		{
			*lpRetDataLen = *(var_4*)(p + sizeof(var_vd*) + sizeof(T_Key));

			if(*lpRetDataLen == 0)
				*vppRetBuf = NULL;
			else
				*vppRetBuf = (var_vd*)(p + sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4));
		}
		return 0;
	}

	var_4 SearchKey_FL(T_Key tKey, var_vd** vppRetBuf = NULL)
	{
		// 计算哈希表位置
		var_4 lIndex = (var_4)(tKey % m_lHashTableSize);
		if(lIndex < 0)
			lIndex *= -1;

		// 查找哈希表
		var_1* p = m_lpHashTable[lIndex];
		while(p)
		{
			if(*(T_Key*)(p + sizeof(var_vd*)) == tKey)
			{
				if(vppRetBuf)
				{
					if(m_lDataLen == 0)
						*vppRetBuf = NULL;
					else
						*vppRetBuf = (var_vd*)(p + sizeof(var_vd*) + sizeof(T_Key));
				}
					
				return 0;
			}
			p = *(var_1**)p;
		}

		return -1;
	}

	var_4 SearchKey_VL(T_Key tKey, var_vd** vppRetBuf, var_4* lpRetDataLen)
	{
		// 计算哈希表位置
		var_4 lIndex = (var_4)(tKey%m_lHashTableSize);
		if(lIndex < 0)
			lIndex *= -1;

		// 查找哈希表
		var_1* p = (var_1*)m_lpHashTable[lIndex];
		while(p)
		{
			if(*(T_Key*)(p + sizeof(var_vd*)) == tKey)
			{
				*lpRetDataLen = *(var_4*)(p + sizeof(var_vd*) + sizeof(T_Key));

				if(*lpRetDataLen == 0)
					*vppRetBuf = NULL;
				else
					*vppRetBuf = (var_vd*)(p + sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4));
				
				return 0;
			}
			p = *(var_1**)p;
		}

		return -1;
	}

	var_4 DeleteKey_FL(T_Key tKey)
	{
		// 判断是否为定长数据
		if(m_lDataLen < 0)
			return -1;
		
		// 计算哈希表位置
		var_4 lIndex = (var_4)(tKey%m_lHashTableSize);
		if(lIndex < 0)
			lIndex *= -1;

		// 查找哈希表
		var_1* p = (var_1*)m_lpHashTable[lIndex];
		var_1* q = (var_1*)(m_lpHashTable + lIndex);
		while(p)
		{
			if(*(T_Key*)(p + sizeof(var_vd*)) == tKey)
			{
				*(var_1**)q = *(var_1**)p;
				*(var_1**)p = 0;
				m_cpAllocator_FL->FreeMem(p);

				m_lAllDataNum--;

				return 0;
			}
			q = p;
			p = *(var_1**)p;
		}

		return -1;
	}

	// 得到当前数据总量
	var_4 GetKeyNum()
	{
		return m_lAllDataNum;
	}

	// 清除所有数据
	var_4 ClearHashSearch()
	{
		m_lAllDataNum = 0;

		if(m_cpAllocator_VL)
			m_cpAllocator_VL->ResetAllocator();
		if(m_cpAllocator_FL)
			m_cpAllocator_FL->ResetAllocator();
		
		memset(m_lpHashTable, 0, m_lHashTableSize * sizeof(var_vd*));

		m_lTravelIndex = 0;
		m_szpTravelPos = m_lpHashTable[m_lTravelIndex];

		return 0;
	}

	// 遍历数据
	var_4 PreTravelKey()
	{
		m_lTravelIndex = 0;
		m_szpTravelPos = (var_1*)m_lpHashTable[m_lTravelIndex];

		return 0;
	}

	var_4 TravelKey(T_Key& trKey, var_vd*& vprRetBuf, var_4& lDataLen)
	{
		if(m_szpTravelPos == NULL)
		{
			for(m_lTravelIndex++; m_lTravelIndex < m_lHashTableSize; m_lTravelIndex++)
			{
				m_szpTravelPos = (var_1*)m_lpHashTable[m_lTravelIndex];
				if(m_szpTravelPos == NULL)
					continue;
				else
					break;
			}
			if(m_lTravelIndex == m_lHashTableSize)
				return -1;
		}

		trKey = *(T_Key*)(m_szpTravelPos + sizeof(var_vd*));
		if(m_lDataLen < 0)
		{
			lDataLen = *(var_4*)(m_szpTravelPos + sizeof(var_vd*) + sizeof(T_Key));
			vprRetBuf = (var_vd*)(m_szpTravelPos + sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4));
		}
		else if(m_lDataLen > 0)
		{
			lDataLen = m_lDataLen;
			vprRetBuf = (var_vd*)(m_szpTravelPos + sizeof(var_vd*) + sizeof(T_Key));
		}

		m_szpTravelPos = *(var_1**)m_szpTravelPos;

		return 0;
	}

	var_4 SaveHashSearch(var_1* filename)
	{
		if(m_lIsInitSystem != 1)
			return -1;

		FILE* fp = fopen(filename, "wb");
		if(fp == NULL)
			return -1;

		var_4 lFlag = 0xEEEEEEEE;
		if(fwrite(&lFlag, sizeof(var_4), 1, fp) != 1)
			goto SAVE_ERROR;

		if(fwrite(&m_lHashTableSize, sizeof(var_4), 1, fp) != 1)
			goto SAVE_ERROR;
		if(fwrite(&m_lAllDataNum, sizeof(var_4), 1, fp) != 1)
			goto SAVE_ERROR;
		if(fwrite(&m_lDataLen, sizeof(var_4), 1, fp) != 1)
			goto SAVE_ERROR;

		if(m_lDataLen < 0)
		{
			for(var_4 i = 0; i < m_lHashTableSize; i++)
			{
				if(m_lpHashTable[i] == NULL)
					continue;
				else
				{
					var_1* p = (var_1*)m_lpHashTable[i];
					while(p)
					{
						if(fwrite(p + sizeof(var_vd*), sizeof(T_Key), 1, fp) != 1)
							goto SAVE_ERROR;
						if(fwrite(p + sizeof(var_vd*) + sizeof(T_Key), sizeof(var_4), 1, fp) != 1)
							goto SAVE_ERROR;
						if(fwrite(p + sizeof(var_vd*) + sizeof(T_Key) + sizeof(var_4), *(var_4*)(p + sizeof(var_vd*) + sizeof(T_Key)), 1, fp) != 1)
							goto SAVE_ERROR;
						p = *(var_1**)p;
					}
				}
			}
		}
		else
		{
			for(var_4 i = 0; i < m_lHashTableSize; i++)
			{
				if(m_lpHashTable[i] == NULL)
					continue;
				else
				{
					var_1* p = (var_1*)m_lpHashTable[i];
					while(p)
					{
						if(fwrite(p + sizeof(var_vd*), sizeof(T_Key), 1, fp) != 1)
							goto SAVE_ERROR;
						if(m_lDataLen && fwrite(p + sizeof(var_vd*) + sizeof(T_Key), m_lDataLen, 1, fp) != 1)
							goto SAVE_ERROR;
						p = *(var_1**)p;
					}
				}
			}
		}

		fseek(fp, 0, SEEK_SET);
		lFlag = 0xFFFFFFFF;
		if(fwrite(&lFlag, sizeof(var_4), 1, fp) != 1)
			goto SAVE_ERROR;

SAVE_ERROR:
		fclose(fp);

		return 0;
	}

	var_4 LoadHashSearch(var_1* filename)
	{
		if(m_lIsInitSystem != 0)
			return -1;

		FILE* fp = fopen(filename, "rb");
		if(fp == NULL)
			return -1;

		var_1* buffer = new var_1[1<<20];
		if(buffer == NULL)
		{
			fclose(fp);
			return -1;
		}
		
		var_1 head[32];
		var_vd* vpRetBuf = NULL;
		var_4 lRetLen = 0;

		var_4 lFlag = 0;		
		var_4 lHashTableSize = 0;
		var_4 lAllDataNum = 0;
		var_4 lDataLen = 0;

		if(fread(&lFlag, sizeof(var_4), 1, fp) != 1 || lFlag != 0xFFFFFFFF)
			goto LOAD_ERROR;
				
		if(fread(&lHashTableSize, sizeof(var_4), 1, fp) != 1)
			goto LOAD_ERROR;
				
		if(fread(&lAllDataNum, sizeof(var_4), 1, fp) != 1)
			goto LOAD_ERROR;
				
		if(fread(&lDataLen, sizeof(var_4), 1, fp) != 1)
			goto LOAD_ERROR;

		if(InitHashSearch(lHashTableSize, lDataLen))
			goto LOAD_ERROR;

		if(m_lDataLen < 0)
		{
			var_4 lOneLen = sizeof(T_Key) + sizeof(var_4);
			while(fread(head, lOneLen, 1, fp))
			{
				if(fread(buffer, *(var_4*)(head + sizeof(T_Key)), 1, fp) != 1)
					goto LOAD_ERROR;
				if(AddKey_VL(*(T_Key*)head, buffer, *(var_4*)(head + sizeof(T_Key)), &vpRetBuf, &lRetLen))
					goto LOAD_ERROR;
			}
		}
		else
		{
			T_Key tKey;
			while(fread(&tKey, sizeof(T_Key), 1, fp))
			{
				if(m_lDataLen && fread(buffer, m_lDataLen, 1, fp) != 1)
					goto LOAD_ERROR;
				if(AddKey_FL(tKey, buffer, &vpRetBuf))
					goto LOAD_ERROR;
			}
		}

LOAD_ERROR:
		delete buffer;
		fclose(fp);

		if(GetKeyNum() != lAllDataNum)
			return -1;

		return 0;
	}

	// 得到当前版本号
	var_1* GetVersion()
	{
		// v1.000 - 2008.08.26 - 初始版本
		// v1.001 - 2008.09.03 - 修改ClearHashSearch函数中没memset哈希表的错误, 
		// v1.010 - 2008.09.04 - 增加持久化函数SaveHashSearch, LoadHashSearch
		// v1.011 - 2008.09.10 - 修改支持单关键字,不带附加值域检索
		// v1.100 - 2009.03.31 - 增加跨平台支持
		return "v1.100";
	}

public:
	var_4 m_lDataLen;
	var_4 m_lAllDataNum;

	var_4   m_lHashTableSize;
	var_1** m_lpHashTable;
		
	UT_Allocator<var_1>*  m_cpAllocator_VL;
	UC_Allocator_Recycle* m_cpAllocator_FL;

	var_4  m_lTravelIndex;
	var_1* m_szpTravelPos;

	var_4 m_lIsInitSystem;
};

#endif // _UT_HASH_SEARCH_H_

