/*
*by liugang 2011.12.20
*key type:   string type || number type or simple struct which length << 64 bit
*for get key mapped continus id;
*one put multi get lockfree
*/

#ifndef COUNTER_NOLOCK_H
#define COUNTER_NOLOCK_H

#include <stdio.h>
#include "hash_common.h"

using namespace std;

#define mod2pow_num(num, buck_num) ((num) & ((buck_num) - 1)) 
#define get_ele_ptr(off, slices, slice_pow, slice_sz) (slices[off>>slice_pow] + mod2pow_num(off, slice_sz))


static const  int SLICE_POW = 16;
static const  int SLICE_SZ = (1<<16);
static const  int MAX_SLICE = 65536;

struct ele_s 
{
	u64 hasher;
	u32 id;
	u32 off;
};

template <class Kty>
class map_number_nolock
{
public:

	
	bool init(u32 bucket_cnt)
	{
		m_buck_cnt = get_2pow_size(bucket_cnt);
		m_buck_cnt < PAGE_MIN_SIZE? bucket_cnt = PAGE_MIN_SIZE:true;
		m_buck_cnt > BUCK_MAX_SIZE? bucket_cnt = BUCK_MAX_SIZE:true;
		m_element_cnt = 0;
		m_ptr_bucket = new u32[m_buck_cnt];
		m_ptr_slices = new ele_s*[MAX_SLICE];
		memset(m_ptr_slices, 0, sizeof(ele_s*)*MAX_SLICE);
		memset(m_ptr_bucket, 0, sizeof(u32)*m_buck_cnt);

		m_ptr_slices[0] = new ele_s[SLICE_SZ];
		m_init_ok = true;
		return true;
	}


	inline u32 get(const Kty& k)
	{
		u64 hasher = hash_wrap(k);
		u32 id = mod2pow_num(hasher, m_buck_cnt);
		u32 off = m_ptr_bucket[id];
		if (off)
		{
			ele_s* p_ele = get_ele_ptr(off, m_ptr_slices, SLICE_POW, SLICE_SZ);
			if (p_ele->hasher == hasher)
			{
				return p_ele->id;
			}
			else
			{
				off = p_ele->off;
			}

			while(off)
			{
				p_ele = get_ele_ptr(off, m_ptr_slices, SLICE_POW, SLICE_SZ);
				if (p_ele->hasher == hasher)
				{
					return p_ele->id;
				}
				else
				{
					off = p_ele->off;
				}
			}
		}
		return -1;

	}
	


	u32 put(const Kty& k)
	{
		u64 hasher = hash_wrap(k);
		ele_s* p_ele;
		u32 id = mod2pow_num(hasher, m_buck_cnt);
		u32 off = m_ptr_bucket[id];

		while(off)
		{
			p_ele = get_ele_ptr(off, m_ptr_slices, SLICE_POW, SLICE_SZ);
			if (p_ele->hasher == hasher)
			{
				break;
			}
			off = p_ele->off;
		}

		if (off == 0) //find none
		{
			if ((m_element_cnt + 1) & (SLICE_SZ - 1))
			{
				;
			}
			else
			{
				int alloc_id = ((m_element_cnt + 1) >> SLICE_POW);
				if (alloc_id >= MAX_SLICE)
				{
					return -1;
				}
				m_ptr_slices[alloc_id] = new ele_s[SLICE_SZ];
			}
			p_ele = get_ele_ptr(m_element_cnt+1, m_ptr_slices, SLICE_POW, SLICE_SZ);
			p_ele->hasher = hasher;
			p_ele->off = m_ptr_bucket[id];
			p_ele->id = m_element_cnt;
			++m_element_cnt;
			m_ptr_bucket[id] = m_element_cnt;
		}
		return p_ele->id;
	}
	
	inline u32 size()
	{
		return m_element_cnt;
	}

	map_number_nolock()
	{
		m_init_ok = false;
	}

	~map_number_nolock()
	{
		if (!m_init_ok)
			return;
		delete[] m_ptr_bucket;
		for (int i = 0; i < MAX_SLICE; ++i)
		{
			delete[] m_ptr_slices[i];
		}
		delete[] m_ptr_slices;
	}
	
	void view_header()
	{
		printf("bucket_num:%d, slice_size:%d, slice_count:%d, element_cnt:%d\n",
			m_buck_cnt, SLICE_SZ, ((m_element_cnt+1) >>SLICE_POW) + 1, m_element_cnt);
	}

	void view_stat_info()
	{
		printf("bucket_num:%d, slice_size:%d, slice_count:%d, element_cnt:%d\n",
			m_buck_cnt, SLICE_SZ, ((m_element_cnt+1) >>SLICE_POW) + 1, m_element_cnt);

		int stat[16] = {0};
		int off;
		int cnt;
		ele_s* p_hasher;
		for(int i = 0; i < m_buck_cnt; ++i)
		{
			cnt = 0;
			off = m_ptr_bucket[i];
			while(off)
			{

				++cnt;
				p_hasher = get_ele_ptr(off, m_ptr_slices, SLICE_POW, SLICE_SZ);
				off = p_hasher->off;
			}
			if (cnt == 0) continue;
			(cnt < 15) ? ++stat[cnt]:++stat[15];
		}
		for(int i = 0; i < 15; ++i)
			printf("%-10d bucket has  %-10d node\n", stat[i], i);
		printf("%-10d bucket has  >=15 node\n", stat[15]);
	}

private:

	u32*   m_ptr_bucket;
	ele_s** m_ptr_slices;
	int   m_buck_cnt;
	u32   m_element_cnt;
	bool  m_init_ok;
};

#endif
