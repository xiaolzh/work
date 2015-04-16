#ifndef _HEAP_HASH_PACK_H_
#define _HEAP_HASH_PACK_H_

//////////////////////////////////////////////////////////////////////
//文件名称:  heaphashpack.h
//摘要:     最大、最小堆类及hashtable关连类
//当前版本: 1.0
//作者:     qinfei
//完成日期: 20120223
///////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>

//#define TEST_HEAP_HEAP

template<class TYPE1, class TYPE2, class TYPE3> 
class CHeapHashPack
{
public:
	typedef struct hash_table_node
	{
		hash_table_node    *ptr_list;	
		int                 suf;
	}HASH_TABLE_NODE;

public:
	CHeapHashPack()
	{
		m_iCur = 0;
		m_iMax = 0; 
		m_tpKey = NULL; 
		m_tpData1 = NULL;
		m_spHI = NULL;
		m_spHash = NULL;
		m_hash_size = 0;
		m_ptr_free_mem = NULL;
	}

	~CHeapHashPack()
	{

	}

	int Init(const int iHeapSize)
	{
		if(iHeapSize<=0)
			return -1;

		m_iMax = iHeapSize; 
		m_tpKey = new TYPE1[iHeapSize+1]; 
		if(m_tpKey == NULL)
			return -2;

		m_tpData1 = new TYPE2[iHeapSize+1];
		if(m_tpData1 == NULL)
			return -3;

		m_tpData2 = new TYPE3[iHeapSize + 1];
		if(m_tpData2 == NULL)
			return -4;

		m_spHI = new HASH_TABLE_NODE * [iHeapSize+1];
		if(m_spHI == NULL)
			return -4;

		m_hash_size = iHeapSize * 2;

		m_spHash = new HASH_TABLE_NODE * [m_hash_size];
		if(m_spHash == NULL)
			return -5;
		
		memset(m_spHash, 0, sizeof(HASH_TABLE_NODE*) * m_hash_size);

		HASH_TABLE_NODE *ptr = NULL;
		m_ptr_free_mem = NULL;

		for(int i = 0; i < iHeapSize + 1; i++)
		{
			ptr = new HASH_TABLE_NODE;
			if(ptr == NULL)
				return -6;

			ptr->ptr_list = m_ptr_free_mem;
			m_ptr_free_mem = ptr;
		}
		return 0;
	}

	//最小堆Insert: 
	int Insert_Min(const TYPE1& tKey, const TYPE2& tData1, const TYPE3 &tData2)
	{
		if (m_iCur >= m_iMax) 
		{
			printf("Insert_Min failed!\n");
			return -1;
		}

		int suf = (int)(tData2 % m_hash_size);

		HASH_TABLE_NODE *ptr_ht = m_spHash[suf];

		while(ptr_ht && m_tpData2[ptr_ht->suf] != tData2)
		{
			ptr_ht = ptr_ht->ptr_list;
		}	

		int ret = 0;
#ifdef TEST_HEAP_HEAP
		int i = 0, t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] > m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] >= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] > m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] >= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif

		if(ptr_ht)
		{
			if(m_tpKey[ptr_ht->suf] > tKey || (m_tpKey[ptr_ht->suf] == tKey && m_tpData1[ptr_ht->suf] >= tData1) )
			{			
				return 1;
			}

			if(Delete_Min(ptr_ht->suf) != 0)
			{
				printf("delete_min failed!\n");
				return -2;	
			}

			ret = 2;
		}
		else
		{			
			if(m_ptr_free_mem == NULL)
				return -3;

			ret = Pop_Min();
			if(ret != 0)
			{
				printf("pop_min:%d\n", ret);
				return ret - 10;
			}

			ptr_ht = m_ptr_free_mem;
			m_ptr_free_mem = m_ptr_free_mem->ptr_list;

			ptr_ht->ptr_list = m_spHash[suf];
			m_spHash[suf] = ptr_ht;		

			//	ptr_ht->ptr_list = NULL;
		}

		//为tData寻找应插入的位置 
		//iCurrent从新的叶节点开始，并沿着树上升 
		int iCurrent = ++m_iCur, tmp = iCurrent>>1; 
		while (iCurrent != 1 && (tKey < m_tpKey[tmp] || (tKey == m_tpKey[tmp] && tData1 < m_tpData1[tmp] ) ) )  
		{ 
			m_tpKey[iCurrent] = m_tpKey[tmp]; // 将元素下移 
			m_tpData2[iCurrent] = m_tpData2[tmp];

			m_spHI[iCurrent] = m_spHI[tmp]; //最后一个元素
			m_spHI[iCurrent]->suf = iCurrent;

			m_tpData1[iCurrent] = m_tpData1[tmp]; //最后一个元素	

			iCurrent>>=1;     // 移向父节点 
			tmp = iCurrent>>1;
		} 


		m_tpKey[iCurrent] = tKey; 

		m_tpData2[iCurrent] = tData2;

		m_spHI[iCurrent] = ptr_ht; //最后一个元素
		m_spHI[iCurrent]->suf = iCurrent;

		m_tpData1[iCurrent] = tData1; //最后一个元素	
#ifdef TEST_HEAP_HEAP
		i = 0;
		t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] > m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] >= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] > m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] >= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif
		return ret; 
	}
	
	//最小堆PUSH: 
	int Push_Min(const TYPE1& tKey, const TYPE2& tData1, const TYPE3 &tData2)
	{
		if (m_iCur >= m_iMax) 
		{
			printf("Push_Min failed!\n");
			return -1;
		}
			
		int suf = (int)(tData2 % m_hash_size);

		HASH_TABLE_NODE *ptr_ht = m_spHash[suf];

		while(ptr_ht && m_tpData2[ptr_ht->suf] != tData2)
		{
			ptr_ht = ptr_ht->ptr_list;
		}	

		int ret = 0;
#ifdef TEST_HEAP_HEAP
		int i = 0, t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] > m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] >= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] > m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] >= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif

		if(ptr_ht)
		{
			if(m_tpKey[ptr_ht->suf] > tKey || (m_tpKey[ptr_ht->suf] == tKey && m_tpData1[ptr_ht->suf] >= tData1) )
				return 1;
						
			if(Delete_Min(ptr_ht->suf) != 0)
				return -2;	

			ret = 2;
		}
		else
		{
			ptr_ht = m_ptr_free_mem;
			if(ptr_ht == NULL)
				return -3;

			m_ptr_free_mem = m_ptr_free_mem->ptr_list;

			ptr_ht->ptr_list = m_spHash[suf];
			m_spHash[suf] = ptr_ht;

		//	ptr_ht->ptr_list = NULL;
		}
		
		//为tData寻找应插入的位置 
		//iCurrent从新的叶节点开始，并沿着树上升 
		int iCurrent = ++m_iCur, tmp = iCurrent>>1; 
		
		while (iCurrent != 1 && (tKey < m_tpKey[tmp] || (tKey == m_tpKey[tmp] && tData1 < m_tpData1[tmp] ) ) )  
		{ 
			m_tpKey[iCurrent] = m_tpKey[tmp]; // 将元素下移 
			m_tpData2[iCurrent] = m_tpData2[tmp];

			m_spHI[iCurrent] = m_spHI[tmp]; //最后一个元素
			m_spHI[iCurrent]->suf = iCurrent;

			m_tpData1[iCurrent] = m_tpData1[tmp]; //最后一个元素	

			iCurrent>>=1;     // 移向父节点 
			tmp = iCurrent>>1;
		} 
		

		m_tpKey[iCurrent] = tKey; 

		m_tpData2[iCurrent] = tData2;

		m_spHI[iCurrent] = ptr_ht; //最后一个元素
		m_spHI[iCurrent]->suf = iCurrent;

		m_tpData1[iCurrent] = tData1; //最后一个元素	

#ifdef TEST_HEAP_HEAP
		i = 0;
		t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] > m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] >= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] > m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] >= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif
		return ret; 
	}

	
	//最大堆Insert: 
	int Insert_Max(const TYPE1& tKey, const TYPE2& tData1, const TYPE3 &tData2)
	{
		if (m_iCur == m_iMax) 
			return -1; 

		int suf = (int)(tData2 % m_hash_size);

		HASH_TABLE_NODE *ptr_ht = m_spHash[suf];

		while(ptr_ht && m_tpData2[ptr_ht->suf] != tData2)
		{
			ptr_ht = ptr_ht->ptr_list;
		}	

		int ret = 0;

#ifdef TEST_HEAP_HEAP
		int i = 0, t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] < m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] <= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] < m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] <= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif
		if(ptr_ht)
		{
			if(m_tpKey[ptr_ht->suf] < tKey || (m_tpKey[ptr_ht->suf] == tKey && m_tpData1[ptr_ht->suf] <= tData1) )
				return 1;

			if(Delete_Max(ptr_ht->suf) != 0)
				return -2;		

			ret = 2;
		}
		else
		{			
			if(m_ptr_free_mem == NULL)
				return -3;
			
			ret = Pop_Max();
			if(ret != 0)
				return ret - 10;

			ptr_ht = m_ptr_free_mem;
			m_ptr_free_mem = m_ptr_free_mem->ptr_list;

			ptr_ht->ptr_list = m_spHash[suf];
			m_spHash[suf] = ptr_ht;

		}

		//为tData寻找应插入的位置 
		//iCurrent从新的叶节点开始，并沿着树上升 

		int iCurrent = ++m_iCur, tmp = iCurrent>>1; 
		while (iCurrent != 1 && (tKey > m_tpKey[tmp] || (tKey == m_tpKey[tmp] && tData1 > m_tpData1[tmp])) )  
		{ 
			m_tpKey[iCurrent] = m_tpKey[tmp]; // 将元素下移 
			m_tpData2[iCurrent] = m_tpData2[tmp];

			m_spHI[iCurrent] = m_spHI[tmp]; //最后一个元素
			m_spHI[iCurrent]->suf = iCurrent;

			m_tpData1[iCurrent] = m_tpData1[tmp]; //最后一个元素	

			iCurrent>>=1;              // 移向父节点 
			tmp =iCurrent>>1;
		} 

		m_tpKey[iCurrent] = tKey; 

		m_tpData2[iCurrent] = tData2;

		m_spHI[iCurrent] = ptr_ht; //最后一个元素
		m_spHI[iCurrent]->suf = iCurrent;

		m_tpData1[iCurrent] = tData1; //最后一个元素

#ifdef TEST_HEAP_HEAP
		i = 0;
		t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] < m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] <= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] < m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] <= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif

		return ret;
	}
			
	//最大堆PUSH: 
	int Push_Max(const TYPE1& tKey, const TYPE2& tData1, const TYPE3 &tData2)
	{
		if (m_iCur == m_iMax) 
			return -1; 
	
		int suf = (int)(tData2 % m_hash_size);

		HASH_TABLE_NODE *ptr_ht = m_spHash[suf];
		
		while(ptr_ht && m_tpData2[ptr_ht->suf] != tData2)
		{
			ptr_ht = ptr_ht->ptr_list;
		}	

		int ret = 0;

#ifdef TEST_HEAP_HEAP
		int i = 0, t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] < m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] <= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] < m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] <= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif

		if(ptr_ht)
		{
			if(m_tpKey[ptr_ht->suf] < tKey || (m_tpKey[ptr_ht->suf] == tKey && m_tpData1[ptr_ht->suf] <= tData1) )
				return 1;

			if(Delete_Max(ptr_ht->suf) != 0)
				return -2;		

			ret = 2;
		}
		else
		{
			ptr_ht = m_ptr_free_mem;
			if(ptr_ht == NULL)
				return -3;

			m_ptr_free_mem = m_ptr_free_mem->ptr_list;

			ptr_ht->ptr_list = m_spHash[suf];
			m_spHash[suf] = ptr_ht;
		}
		
		//为tData寻找应插入的位置 
		//iCurrent从新的叶节点开始，并沿着树上升 
		
		int iCurrent = ++m_iCur, tmp = iCurrent>>1; 
		while (iCurrent != 1 && (tKey > m_tpKey[tmp] || (tKey == m_tpKey[tmp] && tData1 > m_tpData1[tmp])) )  
		{ 
			m_tpKey[iCurrent] = m_tpKey[tmp]; // 将元素下移 
			m_tpData2[iCurrent] = m_tpData2[tmp];

			m_spHI[iCurrent] = m_spHI[tmp]; //最后一个元素
			m_spHI[iCurrent]->suf = iCurrent;

			m_tpData1[iCurrent] = m_tpData1[tmp]; //最后一个元素	

			iCurrent>>=1;              // 移向父节点 
			tmp =iCurrent>>1;
		} 

		m_tpKey[iCurrent] = tKey; 

		m_tpData2[iCurrent] = tData2;

		m_spHI[iCurrent] = ptr_ht; //最后一个元素
		m_spHI[iCurrent]->suf = iCurrent;

		m_tpData1[iCurrent] = tData1; //最后一个元素

#ifdef TEST_HEAP_HEAP
		i = 0;
		t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] < m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] <= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] < m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] <= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif

		return ret;
	}

	void Output() const
	{
		printf("The heap %d elements are\n", m_iCur);

		for (int i = 1; i <= m_iCur; i++) 
		{
			printf(" [%d,%d,%d]",m_tpKey[i], m_tpData1[i], m_tpData2[i]); 
		}
		printf("\n");
	}

	void Clear()
	{
		for(int i = 1; i <= m_iCur; i++)
		{
			if(m_spHI[i])
			{
				m_spHI[i]->ptr_list = m_ptr_free_mem;
				m_ptr_free_mem = m_spHI[i];	

				m_spHI[i] = NULL;
			}
		}

		memset(m_spHash, 0, sizeof(HASH_TABLE_NODE*) * m_hash_size);

		m_iCur=0;
	}

private:
	//最小堆POP: 
	int Pop_Min()
	{
		if (m_iCur == 0) 
			return -1; 	

		HASH_TABLE_NODE *ptr_ht = NULL;	

		TYPE1 tLastKey = m_tpKey[m_iCur];//最后一个元素 
		TYPE2 tLastDat1 = m_tpData1[m_iCur]; //最后一个元素 
		HASH_TABLE_NODE *spLastHI = m_spHI[m_iCur]; //最后一个元素 

		TYPE3 tLastData2 = m_tpData2[m_iCur];

		m_iCur--;

		if(m_iCur < 1)
		{
			if(delete_hash(tLastData2, ptr_ht) != 0)
			{
				return -2;			
			}

			ptr_ht->ptr_list = m_ptr_free_mem;
			m_ptr_free_mem = ptr_ht;

			return 0;
		}

		// 从根开始, 为tLast寻找合适的位置 
		int iCurrent = 1,  // 堆的当前节点 
			iChild = 2; // i的子节点 

		bool flg = false;

		while (iChild <= m_iCur)  
		{ 
			// 使tHeapData[iChild] 是i较小的子节点 
			if (iChild < m_iCur && (m_tpKey[iChild] > m_tpKey[iChild+1] || (m_tpKey[iChild] == m_tpKey[iChild+1] && m_tpData1[iChild] > m_tpData1[iChild+1] ) ))  
				iChild++; 

			// 能把y放入tHeapData[iCurrent]吗? 
			if (tLastKey < m_tpKey[iChild] || ((tLastKey == m_tpKey[iChild] && tLastDat1 < m_tpData1[iChild] )) )  
				break;  // 能 
			
			// 不能 
			if(!flg)
			{
				if(delete_hash(m_tpData2[iCurrent], ptr_ht) != 0)
				{
					return -3;			
				}

				ptr_ht->ptr_list = m_ptr_free_mem;
				m_ptr_free_mem = ptr_ht;

				flg = true;
			}

			m_tpKey[iCurrent] = m_tpKey[iChild]; // 子节点上移 
			m_tpData2[iCurrent] = m_tpData2[iChild];

			m_spHI[iCurrent] = m_spHI[iChild]; //最后一个元素
			m_spHI[iCurrent]->suf = iCurrent;

			m_tpData1[iCurrent] = m_tpData1[iChild]; //最后一个元素


			iCurrent = iChild;    // 下移一层 
			iChild <<=1; 
		} 		

		if(!flg)
		{
			if(delete_hash(m_tpData2[iCurrent], ptr_ht) != 0)
			{
				return -4;			
			}

			ptr_ht->ptr_list = m_ptr_free_mem;
			m_ptr_free_mem = ptr_ht;
		}

		m_tpKey[iCurrent] = tLastKey; 
		m_tpData2[iCurrent] = tLastData2;

		m_spHI[iCurrent] = spLastHI; 
		m_spHI[iCurrent]->suf = iCurrent;

		m_tpData1[iCurrent] = tLastDat1; 

		return 0; 
	}	
	
	//最大堆POP: 
	int Pop_Max()
	{
		if (m_iCur == 0) 
			return -1; 

		HASH_TABLE_NODE *ptr_ht = NULL;	

		TYPE1 tLastKey = m_tpKey[m_iCur];//最后一个元素 
		TYPE2 tLData1 = m_tpData1[m_iCur]; //最后一个元素 
		HASH_TABLE_NODE *spLastHI = m_spHI[m_iCur]; //最后一个元素 

		TYPE3 tLData2 = m_tpData2[m_iCur];

		m_iCur--;

		if(m_iCur < 1)
		{
			if(delete_hash(tLData2, ptr_ht) != 0)
			{
				return -2;			
			}

			ptr_ht->ptr_list = m_ptr_free_mem;
			m_ptr_free_mem = ptr_ht;

			return 0;
		}

		// 从根开始, 为tLast寻找合适的位置 
		int iCurrent = 1,  // 堆的当前节点 
			iChild = 2; // i的子节点 

		bool flg = false;

		while (iChild <= m_iCur) 
		{ 
			// 使m_tHeapData[iChild] 是i较大的子节点 
			if (iChild < m_iCur && (m_tpKey[iChild] < m_tpKey[iChild+1] || (m_tpKey[iChild] == m_tpKey[iChild+1] && m_tpData1[iChild] < m_tpData1[iChild+1]) ))  
				iChild++; 

			// 能把y放入m_tHeapData[iCurrent]吗? 
			if (tLastKey > m_tpKey[iChild] || (tLastKey == m_tpKey[iChild] && tLData1 > m_tpData1[iChild]))  
				break;//能 		

			//不能 
			if(!flg)
			{
				if(delete_hash(m_tpData2[iCurrent], ptr_ht) != 0)
				{
					return -3;			
				}

				ptr_ht->ptr_list = m_ptr_free_mem;
				m_ptr_free_mem = ptr_ht;

				flg = true;
			}

			m_tpKey[iCurrent] = m_tpKey[iChild]; // 子节点上移 

			m_tpData2[iCurrent] = m_tpData2[iChild];

			m_spHI[iCurrent] = m_spHI[iChild]; //最后一个元素
			m_spHI[iCurrent]->suf = iCurrent;

			m_tpData1[iCurrent] = m_tpData1[iChild]; //最后一个元素	

			iCurrent = iChild;             // 下移一层 
			iChild <<=1; 
		} 		

		if(!flg)
		{
			if(delete_hash(m_tpData2[iCurrent], ptr_ht) != 0)
			{
				return -4;			
			}

			ptr_ht->ptr_list = m_ptr_free_mem;
			m_ptr_free_mem = ptr_ht;
		}


		m_tpKey[iCurrent] = tLastKey; 

		m_tpData2[iCurrent] = tLData2;

		m_spHI[iCurrent] = spLastHI; 
		m_spHI[iCurrent]->suf = iCurrent;

		m_tpData1[iCurrent] = tLData1; 

		return 0;
	}

	//最小堆删除: 
	int Delete_Min(int suf)
	{
		if(suf < 0 || suf > m_iCur)
			return -2;

		bool flg = true;

		if(m_tpKey[m_iCur] > m_tpKey[suf])
			flg = false;
		else if(m_tpKey[m_iCur] == m_tpKey[suf] && m_tpData1[m_iCur] > m_tpData1[suf])
			flg = false;

		m_spHI[suf] = m_spHI[m_iCur]; //最后一个元素
		m_spHI[suf]->suf = suf;

		m_tpData2[suf] = m_tpData2[m_iCur]; //最后一个元素		

		m_tpData1[suf] = m_tpData1[m_iCur]; //最后一个元素		

		m_tpKey[suf] = m_tpKey[m_iCur--]; //最后一个元素

		if(suf > m_iCur)
		{
#ifdef TEST_HEAP_HEAP
			int i = 0, t = 0;
			for(i = 1; i <= m_iCur; i++)
			{
				t = i * 2;
				if(t > m_iCur)
					break;
				if(m_tpKey[i] > m_tpKey[t])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] >= m_tpData1[t])
					assert(0);

				if(t + 1 <= m_iCur)
				{
					if(m_tpKey[i] > m_tpKey[t + 1])
						assert(0);
					else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] >= m_tpData1[t + 1])
						assert(0);
				}
			}
#endif
			return 0;
		}

		TYPE1 tTmpKey;
		HASH_TABLE_NODE *spTmpHT = NULL;
		TYPE2 tTmpDat1;
		int tmp = 0;
		TYPE3 tTmpDat2;

		if(flg)
		{			
			while(suf > 1)  
			{  
				tmp = suf >> 1;

				if (m_tpKey[suf] < m_tpKey[tmp] || (m_tpKey[suf] == m_tpKey[tmp] && m_tpData1[suf] < m_tpData1[tmp]))  
				{  
					tTmpKey = m_tpKey[suf];  
					m_tpKey[suf] = m_tpKey[tmp];  
					m_tpKey[tmp] = tTmpKey;  

					spTmpHT = m_spHI[suf];  
					m_spHI[suf] = m_spHI[tmp]; 
					m_spHI[suf]->suf = suf;

					m_spHI[tmp] = spTmpHT;  
					m_spHI[tmp]->suf = tmp;

					tTmpDat1 = m_tpData1[suf];  
					m_tpData1[suf] = m_tpData1[tmp];  
					m_tpData1[tmp] = tTmpDat1;  	

					tTmpDat2 = m_tpData2[suf];  
					m_tpData2[suf] = m_tpData2[tmp];  
					m_tpData2[tmp] = tTmpDat2; 

				}  
				else  
				{  
					break;
				}  

				suf = tmp;  
			}  
		}
		else
		{
			suf <<= 1;

			while(suf <= m_iCur)  
			{  
				if (suf + 1 <= m_iCur && (m_tpKey[suf + 1] < m_tpKey[suf] || (m_tpKey[suf + 1] == m_tpKey[suf] && m_tpData1[suf + 1] < m_tpData1[suf]) ))  
					suf += 1;  

				tmp = suf >> 1;

				if (m_tpKey[suf] < m_tpKey[tmp] || (m_tpKey[suf] == m_tpKey[tmp] && m_tpData1[suf] < m_tpData1[tmp]))  
				{  
					tTmpKey = m_tpKey[suf];  
					m_tpKey[suf] = m_tpKey[tmp];  
					m_tpKey[tmp] = tTmpKey;  

					spTmpHT = m_spHI[suf];  
					m_spHI[suf] = m_spHI[tmp];  
					m_spHI[suf]->suf = suf;

					m_spHI[tmp] = spTmpHT;  
					m_spHI[tmp]->suf = tmp;					

					tTmpDat1 = m_tpData1[suf];  
					m_tpData1[suf] = m_tpData1[tmp];  
					m_tpData1[tmp] = tTmpDat1;  

					tTmpDat2 = m_tpData2[suf];  
					m_tpData2[suf] = m_tpData2[tmp];  
					m_tpData2[tmp] = tTmpDat2; 
				}  
				else  
				{  
					break;  
				}  

				suf <<= 1;
			}
		}
		
#ifdef TEST_HEAP_HEAP
		int i = 0, t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] > m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] >= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] > m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] >= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif
		return 0; 
	}

	//最大堆删除: 
	int Delete_Max(int suf)
	{
		if(suf < 0 || suf > m_iCur)
			return -2;
		
		bool flg = false;

		if(m_tpKey[m_iCur] > m_tpKey[suf])
			flg = true;
		else if(m_tpKey[m_iCur] == m_tpKey[suf] && m_tpData1[m_iCur] > m_tpData1[suf])
			flg = true;

		m_spHI[suf] = m_spHI[m_iCur]; //最后一个元素
		m_spHI[suf]->suf = suf;

		m_tpData2[suf] = m_tpData2[m_iCur]; //最后一个元素		

		m_tpData1[suf] = m_tpData1[m_iCur]; //最后一个元素		

		m_tpKey[suf] = m_tpKey[m_iCur--]; //最后一个元素

		if(suf > m_iCur)
		{
#ifdef TEST_HEAP_HEAP

			int i = 0, t = 0;
			for(i = 1; i <= m_iCur; i++)
			{
				t = i * 2;
				if(t > m_iCur)
					break;
				if(m_tpKey[i] < m_tpKey[t])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] <= m_tpData1[t])
					assert(0);

				if(t + 1 <= m_iCur)
				{
					if(m_tpKey[i] < m_tpKey[t + 1])
						assert(0);
					else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] <= m_tpData1[t + 1])
						assert(0);
				}
			}
#endif
			return 0;
		}

		TYPE1 tTmpKey;
		HASH_TABLE_NODE *spTmpHT = NULL;
		TYPE2 tTmpDat1;
		TYPE3 tTmpDat2;

		int tmp = 0;

		if(flg)
		{
			while(suf > 1)  
			{  
				tmp = suf >> 1;

				if (m_tpKey[suf] > m_tpKey[tmp] || (m_tpKey[suf] == m_tpKey[tmp] && m_tpData1[suf] > m_tpData1[tmp]) )  
				{  
					tTmpKey = m_tpKey[suf];  
					m_tpKey[suf] = m_tpKey[tmp];  
					m_tpKey[tmp] = tTmpKey;  

					spTmpHT = m_spHI[suf];  
					m_spHI[suf] = m_spHI[tmp];
					m_spHI[suf]->suf = suf;

					m_spHI[tmp] = spTmpHT;  
					m_spHI[tmp]->suf = tmp;					

					tTmpDat1 = m_tpData1[suf];  
					m_tpData1[suf] = m_tpData1[tmp];  
					m_tpData1[tmp] = tTmpDat1;  

					tTmpDat2 = m_tpData2[suf];  
					m_tpData2[suf] = m_tpData2[tmp];  
					m_tpData2[tmp] = tTmpDat2; 
				}  
				else  
				{  
					break;
				}  

				suf = tmp;  
			}  
		}
		else
		{
			suf <<= 1;

			while(suf <= m_iCur)  
			{   
				if (suf + 1 <= m_iCur && (m_tpKey[suf + 1] > m_tpKey[suf] || (m_tpKey[suf + 1] == m_tpKey[suf] && m_tpData1[suf + 1] > m_tpData1[suf]) ))  
					suf += 1;  

				tmp = suf >> 1;

				if (m_tpKey[suf] > m_tpKey[tmp] || (m_tpKey[suf] == m_tpKey[tmp] && m_tpData1[suf] > m_tpData1[tmp]))  
				{  
					tTmpKey = m_tpKey[suf];  
					m_tpKey[suf] = m_tpKey[tmp];  
					m_tpKey[tmp] = tTmpKey;  

					spTmpHT = m_spHI[suf];  
					m_spHI[suf] = m_spHI[tmp];  
					m_spHI[suf]->suf = suf;

					m_spHI[tmp] = spTmpHT;  
					m_spHI[tmp]->suf = tmp;		

					tTmpDat1 = m_tpData1[suf];  
					m_tpData1[suf] = m_tpData1[tmp];  
					m_tpData1[tmp] = tTmpDat1;  

					tTmpDat2 = m_tpData2[suf];  
					m_tpData2[suf] = m_tpData2[tmp];  
					m_tpData2[tmp] = tTmpDat2; 
				}  
				else  
				{  
					break;  
				}  

				suf <<= 1;
			}
		}

#ifdef TEST_HEAP_HEAP

		int i = 0, t = 0;
		for(i = 1; i <= m_iCur; i++)
		{
			t = i * 2;
			if(t > m_iCur)
				break;
			if(m_tpKey[i] < m_tpKey[t])
				assert(0);
			else if(m_tpKey[i] == m_tpKey[t] && m_tpData1[i] <= m_tpData1[t])
				assert(0);

			if(t + 1 <= m_iCur)
			{
				if(m_tpKey[i] < m_tpKey[t + 1])
					assert(0);
				else if(m_tpKey[i] == m_tpKey[t + 1] && m_tpData1[i] <= m_tpData1[t + 1])
					assert(0);
			}
		}
#endif
		return 0; 
	}

	const int delete_hash(const TYPE3 key, HASH_TABLE_NODE *& ptr_ret)
	{
		int suf = (int)(key % m_hash_size);

		HASH_TABLE_NODE *ptr = m_spHash[suf], *ptr_pre = NULL;	

		while(ptr)
		{
			if(m_tpData2[ptr->suf] == key)
			{		
				if(ptr_pre == NULL)
				{
					m_spHash[suf] = ptr->ptr_list;
				}
				else
				{
					ptr_pre->ptr_list = ptr->ptr_list;					
				}

				ptr_ret = ptr;
				return 0;
			}

			ptr_pre = ptr;
			ptr = ptr->ptr_list;
		}		

		return 1;
	}

public:
	const int QuickSort(int iBegin, int iEnd)
	{
		if(iBegin >= iEnd)
			return 0;

		TYPE1 ulTmpKey = 0, *tpKey = m_tpKey + 1;
		TYPE2 tData1 = 0, *tpData1 = m_tpData1 + 1;
		TYPE3 tData2 = 0, *tpData2 = m_tpData2 + 1;
		HASH_TABLE_NODE *spTmpHT = NULL, **spHT = m_spHI + 1;

		if(iBegin + 1 == iEnd)
		{
			if(tpKey[iBegin]>tpKey[iEnd] || (tpKey[iBegin] == tpKey[iEnd] && tpData1[iBegin] > tpData1[iEnd] ) )
			{
				ulTmpKey = tpKey[iBegin];
				tpKey[iBegin] = tpKey[iEnd];
				tpKey[iEnd] = ulTmpKey;

				tData1 = tpData1[iBegin];
				tpData1[iBegin] = tpData1[iEnd];
				tpData1[iEnd] = tData1;

				tData2 = tpData2[iBegin];
				tpData2[iBegin] = tpData2[iEnd];
				tpData2[iEnd] = tData2;

				spTmpHT = spHT[iBegin];  
				spHT[iBegin] = spHT[iEnd];  
				spHT[iBegin]->suf = iBegin;

				spHT[iEnd] = spTmpHT;  
				spHT[iEnd]->suf = iEnd;	

			}
			return 0;
		}

		int iMid = (iBegin + iEnd)>>1;
		int m = iBegin, n = iEnd;

		TYPE1 tMidKey = tpKey[iMid];
		TYPE2 tMidDat1 = tpData1[iMid];

		while(iBegin < iEnd)
		{
			while(iBegin < iEnd && (tpKey[iBegin] < tMidKey  || (tpKey[iBegin] == tMidKey && tpData1[iBegin] < tMidDat1 ))) iBegin++;
			while(iBegin < iEnd && (tpKey[iEnd] > tMidKey || (tpKey[iEnd] == tMidKey && tpData1[iEnd] > tMidDat1) )) iEnd--;
			if(iBegin < iEnd)
			{
				ulTmpKey = tpKey[iBegin];
				tpKey[iBegin] = tpKey[iEnd];
				tpKey[iEnd] = ulTmpKey;

				tData1 = tpData1[iBegin];
				tpData1[iBegin] = tpData1[iEnd];
				tpData1[iEnd] = tData1;

				tData2 = tpData2[iBegin];
				tpData2[iBegin] = tpData2[iEnd];
				tpData2[iEnd] = tData2;

				spTmpHT = spHT[iBegin];  
				spHT[iBegin] = spHT[iEnd];  
				spHT[iBegin]->suf = iBegin;

				spHT[iEnd] = spTmpHT;  
				spHT[iEnd]->suf = iEnd;	

				if(++iBegin < iEnd)
					iEnd--;
			}
		}

		if(tpKey[iBegin]< tMidKey  || (tpKey[iBegin] == tMidKey && tpData1[iBegin] < tMidDat1 ))
			iBegin++;

		if(iBegin > m)
			QuickSort(m, iBegin - 1);
		if(iEnd < n)
			QuickSort(iEnd, n);

		return 0;
	};	
	
	const int QuickSort_Desc(int iBegin, int iEnd)
	{
		if(iBegin >= iEnd)
			return 0;

		TYPE1 ulTmpKey = 0, *tpKey = m_tpKey + 1;
		TYPE2 tData1 = 0, *tpData1 = m_tpData1 + 1;
		TYPE3 tData2 = 0, *tpData2 = m_tpData2 + 1;
		HASH_TABLE_NODE *spTmpHT = NULL, **spHT = m_spHI + 1;

		if(iBegin + 1 == iEnd)
		{
			if(tpKey[iBegin]<tpKey[iEnd] || (tpKey[iBegin] == tpKey[iEnd] && tpData1[iBegin] < tpData1[iEnd] ) )
			{
				ulTmpKey = tpKey[iBegin];
				tpKey[iBegin] = tpKey[iEnd];
				tpKey[iEnd] = ulTmpKey;

				tData1 = tpData1[iBegin];
				tpData1[iBegin] = tpData1[iEnd];
				tpData1[iEnd] = tData1;

				tData2 = tpData2[iBegin];
				tpData2[iBegin] = tpData2[iEnd];
				tpData2[iEnd] = tData2;


				spTmpHT = spHT[iBegin];  
				spHT[iBegin] = spHT[iEnd];  
				spHT[iBegin]->suf = iBegin;

				spHT[iEnd] = spTmpHT;  
				spHT[iEnd]->suf = iEnd;	

			}
			return 0;
		}

		int iMid = (iBegin + iEnd)>>1;
		int m = iBegin, n = iEnd;

		TYPE1 tMidKey = tpKey[iMid];
		TYPE2 tMidDat1 = tpData1[iMid];

		while(iBegin < iEnd)
		{
			while(iBegin < iEnd && (tpKey[iBegin] > tMidKey  || (tpKey[iBegin] == tMidKey && tpData1[iBegin] > tMidDat1 ))) iBegin++;
			while(iBegin < iEnd && (tpKey[iEnd] < tMidKey || (tpKey[iEnd] == tMidKey && tpData1[iEnd] < tMidDat1) )) iEnd--;
			if(iBegin < iEnd)
			{
				ulTmpKey = tpKey[iBegin];
				tpKey[iBegin] = tpKey[iEnd];
				tpKey[iEnd] = ulTmpKey;

				tData1 = tpData1[iBegin];
				tpData1[iBegin] = tpData1[iEnd];
				tpData1[iEnd] = tData1;

				tData2 = tpData2[iBegin];
				tpData2[iBegin] = tpData2[iEnd];
				tpData2[iEnd] = tData2;

				spTmpHT = spHT[iBegin];  
				spHT[iBegin] = spHT[iEnd];  
				spHT[iBegin]->suf = iBegin;

				spHT[iEnd] = spTmpHT;  
				spHT[iEnd]->suf = iEnd;	

				if(++iBegin < iEnd)
					iEnd--;
			}
		}

		if(tpKey[iBegin]> tMidKey  || (tpKey[iBegin] == tMidKey && tpData1[iBegin] > tMidDat1 ))
			iBegin++;

		if(iBegin > m)
			QuickSort_Desc(m, iBegin - 1);
		if(iEnd < n)
			QuickSort_Desc(iEnd, n);

		return 0;
	};	

private:
	HASH_TABLE_NODE    **m_spHash;
	int                  m_hash_size;

	HASH_TABLE_NODE	   **m_spHI;	
	HASH_TABLE_NODE     *m_ptr_free_mem;

public:
	int                 m_iCur, m_iMax; 
	TYPE1			   *m_tpKey; 
	TYPE2			   *m_tpData1;
	TYPE3              *m_tpData2;
};

#endif//

