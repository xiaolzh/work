#ifndef _IDALLOC_H_
#define _IDALLOC_H_
#include "UH_Define.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class IDAlloc
{
public:
	IDAlloc()
	{
		m_BMap=NULL;
		m_total=0;
		m_current=0;
		m_alloced=0;
	}

	~IDAlloc()
	{
		if(NULL!=m_BMap)
		{
			free(m_BMap);
			m_BMap=NULL;
		}
		m_total=0;
		m_current=1;
		m_alloced=0;
	}

	int init(unsigned int maxID)
	{
		if(maxID<=0)
		{
			return -1;
		}
		m_maxid = maxID + 1;// !!!!!!!!!【注意这里+ 1 】!!!!!!!!!!!
		/* add one long */
		// 使m_maxid 向上变成64 (32) 的整数倍( 若m_maxid 已经是整数倍，则再向上加1)
		m_total = (m_maxid + BITS_PER_LONG) & (~0UL << SHIFT_BITS_PER_LONG);
		// 开m_total 位大小的内存，即64 (32) 个m_total/64 (m_total/32) 的内存
		m_BMap=(unsigned long *)calloc(m_total >> SHIFT_BITS_PER_LONG,BYTES_PER_LONG);
		if(NULL==m_BMap)
		{
			return -2;
		}
		/* set last word */
		// 使最后面多余m_maxid 的位均置为1
		m_BMap[(m_total >> SHIFT_BITS_PER_LONG) - 1] = BIT_MASK(m_maxid);
		/* first bit */	
		set_bit(0);
		m_current=1;
		return 1;
	}

	IDAlloc *copy()
	{
		int ret=0;

		IDAlloc *pID=NULL;
		pID=new IDAlloc();
		if(NULL==pID)
		{
			return NULL;
		}
		// 两个【注意这里】使这个新的pID 内的m_maxid == this->m_maxid
		// 这样，他们的m_total 也相等，下面的memcpy 就是完全拷贝了
		ret=pID->init(m_maxid - 1);// !!!!!!!!!!!【注意这里-1 】!!!!!!!!!!!!!!
		
		if(ret <=0)
		{
			delete pID;
			pID=NULL;
			return NULL;
		}
		memcpy(pID->m_BMap,this->m_BMap,m_total >> 3);
		pID->m_current = this->m_current;
		pID->m_alloced = this->m_alloced;
		return pID;
	}

	int resize(unsigned int newMaxid)
	{
		/* only support larger,and if multithread,outer should lock */
		unsigned long newtotal = 0;
		if(newMaxid <= (m_maxid + BYTES_PER_LONG))
		{
			return 0;
		}
		/* new paras */
		newMaxid += 1;
		/* add one long */
		newtotal = (newMaxid + BITS_PER_LONG) & (~0UL << SHIFT_BITS_PER_LONG);
		m_BMap=(unsigned long *)realloc(m_BMap,(newtotal >> SHIFT_BITS_PER_LONG) * BYTES_PER_LONG);
		if(NULL==m_BMap)
		{
			/* alloc failed,nothing changed,return */
			return -2;
		}
		/* clear old last word */
		m_BMap[(m_total >> SHIFT_BITS_PER_LONG) - 1] &= BIT_CLEAR(m_maxid);
		/* clear left words */
		memset(&m_BMap[m_total >> SHIFT_BITS_PER_LONG],0,(newtotal - m_total) / BITS_PER_LONG);
		/* set last word */
		m_BMap[(newtotal >> SHIFT_BITS_PER_LONG) - 1] = BIT_MASK(newMaxid);
		/* alter parameters */
		m_total = newtotal;
		m_maxid = newMaxid;
		return 1;
	}

	int getID(unsigned int &nid)
	{
		unsigned int nextidx;
		if(m_alloced >= m_maxid)
		{
			return -1;
		}
		if(m_current>=m_maxid)
		{
			m_current=1;
		}
		if(!is_set(m_current))
		{
			nid = m_current;
			set_bit(m_current);
			m_current ++;
			m_alloced ++;
			return 1;
		}
		else
		{
			if(find_next_bit(m_current+1,nextidx))
			{
				nid = nextidx;
				m_current = nextidx;
				set_bit(m_current);
				m_current ++;
				m_alloced ++;
				return 1;
			}
			else
			{
				return -2;
			}
		}
		return 0;
	}

	int freeID(unsigned int id)
	{
		if(id>=m_maxid || id==0 )
		{
			return 0;
		}
		if(!is_set(id))
		{
			return 0;
		}
		clear_bit(id);
		m_alloced --;
		return 1;
	}

	int reset()
	{
		if(m_BMap)
		{
			memset(m_BMap,0,m_total >> 3);
		}
		/* set last word */
		m_BMap[(m_total >> SHIFT_BITS_PER_LONG) - 1] = BIT_MASK(m_maxid);
		set_bit(0);
		m_current=1;
		m_alloced=0;
		return 1;
	}

	void stat(unsigned int &total,unsigned int &current,unsigned int &alloced,unsigned int &maxid)
	{
		total=m_total;
		current=m_current;
		alloced=m_alloced;	
		maxid=m_maxid;
	}

	int is_set(unsigned int idx)
	{
		if(idx>=m_maxid)
		{
			return 0;
		}
		return 1UL & (m_BMap[BIT_WORD(idx)] >> (idx & (BITS_PER_LONG-1)));
	}

	int set_bit(unsigned int idx)
	{
		if(idx>=m_maxid)
		{
			return 0;
		}
		asm volatile("bts %1,%0" : "+m"(*(volatile long *)m_BMap) : "Ir" (idx) : "memory");
		return 1;
	}

	static inline unsigned long ffz(unsigned long word)
	{
			asm("bsf %1,%0" : "=r" (word) : "r" (~word));
			return word;
	}

	int save(char *filename)
	{
		FILE *fp=NULL;
		int j=0;
		if(NULL==filename)
		{
			return -1;
		}
		fp=fopen(filename,"wb");
		if(NULL==fp)
		{
			return -2;
		}
		//write header
		j=fwrite("####",4,1,fp);
		if(j==1)
		{
			j=fwrite(&m_total,sizeof(int),1,fp);
		}
		if(j==1)
		{
			j=fwrite(&m_maxid,sizeof(int),1,fp);
		}
		if(j==1)
		{
			j=fwrite(&m_current,sizeof(int),1,fp);
		}
		if(j==1)
		{
			j=fwrite(&m_alloced,sizeof(int),1,fp);
		}
		//write body
		if(j==1)
		{
			j=fwrite(m_BMap,m_total >> 3,1,fp);
		}
		fclose(fp);
		fp=NULL;
		return j;
	}

	int restore(char *filename)
	{
		FILE *fp=NULL;
		int j=0;
		int ptr;
		if(NULL==filename)
		{
			return -1;
		}
		fp=fopen(filename,"rb");
		if(NULL==fp)
		{
			return -2;
		}
		//read header
		j=fread(&ptr,4,1,fp);
		if(j==1)
		{
			j=fread(&m_total,sizeof(int),1,fp);
		}
		if(j==1)
		{
			j=fread(&m_maxid,sizeof(int),1,fp);
		}
		if(j==1)
		{
			j=fread(&m_current,sizeof(int),1,fp);
		}
		if(j==1)
		{
			j=fread(&m_alloced,sizeof(int),1,fp);
		}
		if((m_total & (~0UL >> (BITS_PER_LONG - SHIFT_BITS_PER_LONG)))!=0 || m_total<m_maxid || m_alloced>m_maxid || m_current>m_maxid || m_total==0 )
		{
			fclose(fp);
			fp=NULL;
			return -3;
		}
		//alloc memory
		if(NULL!=m_BMap)
		{
			free(m_BMap);
			m_BMap=NULL;
		}
		m_BMap=(unsigned long *)calloc(m_total >> SHIFT_BITS_PER_LONG,BYTES_PER_LONG);
		if(NULL==m_BMap)
		{
			return -4;
		}
		//read body
		if(j==1)
		{
			j=fread(m_BMap,m_total >> 3,1,fp);
		}
		fclose(fp);
		fp=NULL;
		return j;
	}
private:
	int clear_bit(unsigned int idx)
	{
		if(idx>=m_maxid)
		{
			return 0;
		}
		asm volatile("btr %1,%0" : "+m"(*(volatile long *)m_BMap) : "Ir" (idx));
		return 1;
	}

	int find_next_bit(unsigned int off,unsigned int &idx)
	{
		//first calculate current wrd
		unsigned int i=0;
		unsigned int startidx = 0;
		unsigned long val = 0;
		unsigned int offset = 0;
		//check parameters
		if(off>=m_maxid)
		{
			idx=m_total;
			return 0;
		}
		//first calculate current wrd
		startidx = BIT_WORD(off);
		val = m_BMap[startidx];
		offset = off % BITS_PER_LONG;

		if(offset)
		{
			val |= (~0UL) >> (BITS_PER_LONG - offset);
			if(~val)
			{
				//found in same word
				idx = (startidx << SHIFT_BITS_PER_LONG) + ffz(val);
				//return idx
				return 1;
			}
			startidx++;
		}
		for(i=startidx;i<(m_total >> SHIFT_BITS_PER_LONG);i++)
		{
			if(~m_BMap[i])
			{
				//found in middle
				idx = (i << SHIFT_BITS_PER_LONG) + ffz(m_BMap[i]);
				//return idx
				return 1;
			}
		}
		//not found,round again
		for(i=0;i<startidx;i++)
		{
			if(~m_BMap[i])
			{
				//found in middle
				idx = (i << SHIFT_BITS_PER_LONG) + ffz(m_BMap[i]);
				//return idx
				return 1;
			}
		}
		//still not found
		idx=m_total;
		return 0;
	}

	unsigned long *m_BMap;
	unsigned int m_total; 	// alloc memory size
	unsigned int m_maxid; 	// the max number of memory available
	unsigned int m_current;	// cycle using memory, like circular queue, current point
	unsigned int m_alloced; // represent the number of memory block used
};
#endif
