/*
*by liugang 2011.07.04
*key type:   string type || number type or simple struct which length << 64 bit
*value type: number type or simple struct
*/

#ifndef REAL_TIME_IDX_H
#define REAL_TIME_IDX_H

#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "hash_common.h"
#ifdef _WIN32
#include <io.h>
#define O_LARGEFILE 0
#define  ftruncate _chsize
#define  read _read
#define  write _write
#define  lseek _lseek
#define  open _open
#else
#define _O_BINARY 0
#include <unistd.h>
#endif
//#include <sys/mman.h>

using namespace std;

#define get_value_ptr(p_hasher) ((char*)p_hasher + sizeof(u64))
#define get_off_ptr(p_hasher, next_off) ((u32*)((char*)p_hasher + next_off))
#define get_node_ptr(off, slices, slice_pow, slice_sz, row_pow) ( slices[off>>slice_pow] + (mod2pow_num(off, slice_sz)<<row_pow) )
#define get_free_ptr(off, slices, slice_pow, slice_sz, row_sz) ((u32*)(slices[off>>slice_pow] + mod2pow_num(off, slice_sz)*row_sz))
#define get_slice_buf_len(slice_sz, value_pow) (slice_sz << value_pow)
#define get_inod_ptr(off) ((*)(slices[off>>slice_pow] + mod2pow_num(off, slice_sz)<<row_pow))

static const int SLICE_MAX_CNT = 8192;
static const int DUMP_SIZE = 4*1024*1024; //1M

#define FALSE_RETURN(clause)     if(clause) {fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));return false;}
#define ZERO_RETURN(clause)     if(clause) {fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));return 0;}

#define  MAGIC_NUM  1234567890
#define  MAX_PATH  255
#define  INVERT_NOT_END(p)      (*(u32*)(p+1))

enum {INIT_ING = 0, INIT_OK, UPDATE_FAIL};
enum {SYNC_OK = 0, FILE_INVALID, SYNC_FAIL};



struct hash_node_s 
{
	u64 hasher;
	u32 node_off;
	u32 next;
};

struct table_header_s
{
	u64 time_stamp;
	u32 inc_sequence;
	u32 bucket_cnt;
	u32 ivt_node_pow;
	u32 slice_sz;
	u32 slice_cnt;
	u32 inod_slice_sz;
	u32 inod_slice_cnt;//32
	u32 key_cnt;
	u32 key_alloc_cnt;
	u32 inod_cnt;
	u32 inod_alloc_cnt;
	u32 key_free_beg;
	u32 key_free_end;
	u32 inod_free_beg;
	u32 inod_free_end;
	u32 file_mark;
};



inline u32 get_check_sum(char* start, int len)
{
	u32 left = len%4;
	len = len/4;
	u32 sum = 0;
	u32 *p = (u32*)start;

	int i;
	for(i=0;i<len;++i)
	{
		sum ^= *(p+i);
	}
	if(left>0)
	{
		u32 tmp = 0;
		memcpy(&tmp, p+i, left);
		sum ^= tmp;
	}
	return sum;
}


// the implement of serialized dy_hash

struct change_mark_s
{
	char* ptr;
	u64 off;
	u32 len;
	u32 id;

	bool operator < (const change_mark_s& r) const
	{
		return this->off < r.off || 
			this->off == r.off&& this->id < r.id;
	}

	bool operator == (const change_mark_s& r)const
	{
		return this->off == r.off && this->len == r.len;
	}
};

inline bool continus_io(const change_mark_s& l, const change_mark_s& r)
{
	return l.ptr + l.len == r.ptr;
}

inline void merge_io(change_mark_s& l, change_mark_s& r)
{
	l.len += r.len; 
}

template<class _FwdIt,
class _PrEqual,class _PrAdd> inline
	_FwdIt GroupByMember(_FwdIt _First, _FwdIt _Last, _PrEqual _PredEqual,_PrAdd _PredAdd)
{	// remove each satisfying _PredEqual with previous

	for (_FwdIt _Firstb; (_Firstb = _First) != _Last && ++_First != _Last; )
		if (_PredEqual(*_Firstb, *_First))
		{	// copy down
			_PredAdd(*_Firstb, *_First);
			for (; ++_First != _Last; )				
			{
				if (!_PredEqual(*_Firstb, *_First))
					*++_Firstb = *_First;
				else
					_PredAdd(*_Firstb, *_First);
			}			

			return (++_Firstb);
		}
		return (_Last);
}

inline bool batch_write(int fd, const char *buf, size_t size)
{
	assert(fd >= 0 && buf && size >= 0);
	const char *rp = (const char*)buf;
	do 
	{
		int wb = write(fd, rp, size);
		switch(wb)
		{
		case -1:  return false;
		case 0: break;
		default:
			rp += wb;
			size -= wb;
			break;
		}
	} while(size > 0);
	return true;
}


/* Read data from a file. */
inline bool batch_read(int fd, void *buf, size_t size)
{
	assert(fd >= 0 && buf && size >= 0);
	char *wp = (char*)buf;
	do 
	{
		int rb = read(fd, wp, size);
		switch(rb)
		{
		case -1: return false;
		case 0: return size < 1;
		default:
			wp += rb;
			size -= rb;
		}
	} while(size > 0);
	return true;
}


template <class Kty,class Vty>
class real_time_idx
{
public:
	real_time_idx()
	{
		m_init_ok = 0;
		m_ptr_bucket = NULL;
		for (int i = 0; i < SLICE_MAX_CNT; ++i)
		{
			m_ptr_slices[i] = NULL;
			m_ptr_ivt_slices[i] = NULL;
		}

	}

	~real_time_idx()
	{
		if (!m_init_ok)
			return;
#ifndef _WIN32
		pthread_mutex_destroy(&m_mutexlock);
#endif
		for (int i = 0; i < m_ptr_header->slice_cnt; ++i)
		{
			delete[] m_ptr_slices[i];
		}
		for (int i = 0; i < m_ptr_header->inod_slice_cnt; ++i)
		{
			delete[] m_ptr_ivt_slices[i];
		}
		delete[] m_ptr_header;
		delete[] m_ptr_bucket;
		close(m_fd);
		close(m_ivt_fd);
	}


	bool init(u32 bucket_cnt, u32 slice_sz, const char* idx_file, const char* ivt_file)
	{
		m_mgnum = MAGIC_NUM;
		m_idx_name = idx_file;
		m_ivt_name = ivt_file;

		size_t sz;
		table_header_s st;
		m_fd = open(idx_file, O_RDWR|O_CREAT|O_LARGEFILE|_O_BINARY, 0644);
		FALSE_RETURN(m_fd == -1)

		struct stat statInfo;
		FALSE_RETURN(fstat(m_fd, &statInfo ) < 0);

		sz = statInfo.st_size;
		int need_init = 0;
		if (sz < PAGE_MIN_SIZE) //need re initial
		{
			need_init = 1;
		}
		else	
		{
			FALSE_RETURN(read(m_fd, &st, sizeof(st)) != sizeof(st));
			if (st.file_mark == INIT_ING) //init last action assign bucket_cnt
			{
				need_init = 1;
			}
		}

		m_ivt_fd = ::open(ivt_file, O_RDWR|O_CREAT|O_LARGEFILE|_O_BINARY, 0644);
		FALSE_RETURN(m_ivt_fd == -1);

		if (need_init == 1)
		{
			bucket_cnt = get_2pow_size(bucket_cnt);
			bucket_cnt < PAGE_MIN_SIZE? bucket_cnt = PAGE_MIN_SIZE:true;
			bucket_cnt > BUCK_MAX_SIZE? bucket_cnt = BUCK_MAX_SIZE:true;
			slice_sz = get_slice_cnt(slice_sz, sizeof(hash_node_s));

			sz = PAGE_MIN_SIZE + bucket_cnt * 4 + sizeof(hash_node_s) * slice_sz;
			FALSE_RETURN(ftruncate(m_fd, sz) < 0);


			st.bucket_cnt = bucket_cnt;
			st.key_cnt = 0;
			st.inod_cnt = 0;
			st.file_mark = INIT_OK;
			st.key_free_beg = 0;
			st.key_free_end = 0;
			st.inod_free_beg = 0;
			st.inod_free_end = 0;
			st.slice_sz = slice_sz;
			st.slice_cnt = 1;
			st.key_alloc_cnt = 1;
			st.inod_alloc_cnt = 1;
			st.inod_slice_sz = slice_sz << 2;
			st.inod_slice_cnt = 1;
			st.time_stamp = 0;
			st.inc_sequence = 0;

			st.ivt_node_pow = get2pow(get_2pow_size(sizeof(Vty)+sizeof(u32)));
			FALSE_RETURN(-1 == lseek(m_fd, 0, SEEK_SET));
			FALSE_RETURN(write(m_fd, &st, sizeof(st)) != sizeof(st));
#ifndef _WIN32
			fsync(m_fd);
#endif
		}
		else
		{

			if (st.file_mark != SYNC_FAIL && test_need_merge())
			{
				int ret = merge_to_trunk(m_ivt_name, m_ivt_fd, false) ;//first sync
				if(ret >= SYNC_FAIL)
				{
					fprintf(stderr,"file:%s , line: %d, fail num =%d ,error info: %s\n",__FILE__,__LINE__, ret, strerror(errno));
					return false;
				}

				ret = merge_to_trunk(m_idx_name, m_fd, true) ;//second sync ,inc_sequenc must be the last one
				if(ret >= SYNC_FAIL)
				{
					fprintf(stderr,"file:%s , line: %d, fail num =%d ,error info: %s\n",__FILE__,__LINE__, ret, strerror(errno));
					return false;
				}
			}
			

		}

		u32 bsz=st.bucket_cnt*sizeof(u32);
		FALSE_RETURN(-1 == lseek(m_fd, 0, SEEK_SET));
		m_ptr_header = (table_header_s*)(new char[PAGE_MIN_SIZE]);
		FALSE_RETURN(m_ptr_header == NULL || read(m_fd, m_ptr_header, PAGE_MIN_SIZE) != PAGE_MIN_SIZE);
		m_ptr_bucket = (u32*)(new char[bsz]);
		FALSE_RETURN(m_ptr_bucket == NULL || !batch_read(m_fd, m_ptr_bucket, bsz));

		m_hash_node_sz = get_2pow_size(sizeof(hash_node_s));
		m_hash_node_pow = get2pow(m_hash_node_sz);
		m_slice_buf_len = get_slice_buf_len(st.slice_sz, m_hash_node_pow);

		for (int i = 0; i < m_ptr_header->slice_cnt; ++i)
		{
			m_ptr_slices[i] = new char[m_slice_buf_len];
			FALSE_RETURN(m_ptr_slices[i] == NULL || !batch_read(m_fd, m_ptr_slices[i], m_slice_buf_len));
		}

		
		m_buck_cnt = m_ptr_header->bucket_cnt;
		m_slice_pow = get2pow(m_ptr_header->slice_sz);
		m_slice_sz = m_ptr_header->slice_sz;
		m_inod_slice_pow = get2pow(m_ptr_header->inod_slice_sz);
		m_inod_slice_sz = m_ptr_header->inod_slice_sz;
		m_ivt_node_sz = get_2pow_size(sizeof(Vty)+sizeof(u32));
		m_ivt_node_pow = get2pow(m_ivt_node_sz);
		m_ivt_slice_buf_len = get_slice_buf_len(st.inod_slice_sz, m_ivt_node_pow);

	

		FALSE_RETURN(fstat(m_ivt_fd, &statInfo ) < 0) ;
		
		if (statInfo.st_size < m_ivt_slice_buf_len) //need re initial
		{
			FALSE_RETURN(ftruncate(m_ivt_fd, m_ivt_slice_buf_len) < 0);
		}
		FALSE_RETURN(-1 == lseek(m_ivt_fd, 0, SEEK_SET));
		for (int i = 0; i < m_ptr_header->inod_slice_cnt; ++i)
		{
			m_ptr_ivt_slices[i] = new char[m_ivt_slice_buf_len];
			FALSE_RETURN(m_ptr_ivt_slices[i] == NULL || !batch_read(m_ivt_fd, m_ptr_ivt_slices[i], m_ivt_slice_buf_len));
		}
#ifndef _WIN32
		fsync(m_ivt_fd);
#endif
		m_ci_idx.reserve(DUMP_SIZE);
		m_ci_ivt.reserve(DUMP_SIZE);
	
#ifndef _WIN32
		FALSE_RETURN(pthread_mutex_init(&m_mutexlock, NULL) != 0);
#endif
		m_init_ok = true;
		return true;
}

	inline const Vty* next_ivt_node(const Vty* node)
	{
		u32 off = *(u32*)(++node);
		if (off == 0)
			return NULL;
		return (Vty*)get_node_ptr(off, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
	}

	inline const Vty* get(const Kty& k)
	{
		
		u64 hasher = hash_wrap(k);
		u32 id = mod2pow_num(hasher, m_buck_cnt);
		u32 off = m_ptr_bucket[id];
		if (off != 0)
		{
			hash_node_s* p_hn = (hash_node_s*)get_node_ptr(off, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);
			if (p_hn->hasher == hasher)
			{
				return (Vty *)get_node_ptr(p_hn->node_off, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
			}
			else
			{
				off = p_hn->next;
			}

			while(off)
			{
				p_hn = (hash_node_s*)get_node_ptr(off, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);
				if (p_hn->hasher == hasher)
				{
					return (Vty *)get_node_ptr(p_hn->node_off, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
				}
				else
				{
					off = p_hn->next;
				}
			}
		}
		return NULL;

	}


	bool put(const Kty& k, const Vty* ptr_val)
	{
		u64 hasher = hash_wrap(k);
		u32 id = mod2pow_num(hasher, m_buck_cnt);
		u32 off = m_ptr_bucket[id];
		hash_node_s* p_hn;
		u32 inod_off = 0;
		while(off)
		{
			p_hn = (hash_node_s*)get_node_ptr(off, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);

			if (p_hn->hasher == hasher)
			{
				inod_off = p_hn->node_off;
				break;
			}
			off = p_hn->next;
		}



		inod_off = store_ivt_node(ptr_val, inod_off);
		FALSE_RETURN(inod_off == 0)

		u64 off64;
		if (off == 0) //find none
		{
			if (m_ptr_header->key_free_beg != m_ptr_header->key_free_end ) //  free space left
			{
				p_hn = (hash_node_s*)get_node_ptr(m_ptr_header->key_free_beg, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);
				u32 next_free_off = *(u32*)p_hn;
				off64 = PAGE_MIN_SIZE + m_buck_cnt*sizeof(u32) + m_ptr_header->key_free_beg*m_hash_node_sz;
				cp_with_buf(&(p_hn->hasher), &hasher, sizeof(hasher), off64 + offsetof(hash_node_s,hasher), m_ci_idx);//p_hn->hasher = hasher;
				cp_with_buf(&(p_hn->node_off), &inod_off, sizeof(u32),off64 + offsetof(hash_node_s,node_off), m_ci_idx);//p_hn->node_off = inod_off;
				cp_with_buf(&(p_hn->next), &(m_ptr_bucket[id]), sizeof(p_hn->next), off64 + offsetof(hash_node_s,next), m_ci_idx);//p_hn->next = m_ptr_bucket[id];
				cp_with_buf(&(m_ptr_bucket[id]), &(m_ptr_header->key_free_beg), sizeof(m_ptr_bucket[id]), PAGE_MIN_SIZE+id*sizeof(u32), m_ci_idx);//m_ptr_bucket[id] = m_ptr_header->key_free_beg;
				cp_with_buf(&(m_ptr_header->key_free_beg), &next_free_off, sizeof(next_free_off), offsetof(table_header_s, key_free_beg), m_ci_idx);//m_ptr_header->key_free_beg = next_free_off;
				//used one free block
			}
			else //alloc new block ,0 is empty
			{
				if (m_ptr_header->key_alloc_cnt >= m_slice_sz*m_ptr_header->slice_cnt)
				{
					FALSE_RETURN(m_ptr_header->slice_cnt >= SLICE_MAX_CNT)
					m_ptr_slices[m_ptr_header->slice_cnt] = new char[m_slice_buf_len];
					FALSE_RETURN(m_ptr_slices[m_ptr_header->slice_cnt] == NULL);
					++m_ptr_header->slice_cnt;
					cp_with_buf(&(m_ptr_header->slice_cnt), &(m_ptr_header->slice_cnt), sizeof(u32), offsetof(table_header_s, slice_cnt), m_ci_idx);
					size_t new_sz = PAGE_MIN_SIZE + m_ptr_header->bucket_cnt * 4 + m_slice_buf_len * (m_ptr_header->slice_cnt);
					FALSE_RETURN(0 > ftruncate(m_fd, new_sz));
				}
				p_hn = (hash_node_s*)get_node_ptr(m_ptr_header->key_alloc_cnt, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);
				off64 = PAGE_MIN_SIZE + m_buck_cnt*sizeof(u32) + m_ptr_header->key_alloc_cnt*m_hash_node_sz;
				cp_with_buf(&(p_hn->hasher), &hasher, sizeof(hasher),off64 + offsetof(hash_node_s, hasher), m_ci_idx);//p_hn->hasher = hasher;
				cp_with_buf(&(p_hn->node_off), &inod_off, sizeof(u32), off64 + offsetof(hash_node_s,node_off), m_ci_idx);//p_hn->node_off = inod_off;
				cp_with_buf(&(p_hn->next), &(m_ptr_bucket[id]), sizeof(p_hn->next),off64 + offsetof(hash_node_s,next), m_ci_idx);//p_hn->next = m_ptr_bucket[id];
				cp_with_buf(&(m_ptr_bucket[id]), &(m_ptr_header->key_alloc_cnt), sizeof(m_ptr_bucket[id]),  PAGE_MIN_SIZE+id*sizeof(u32),  m_ci_idx);//m_ptr_bucket[id] = m_ptr_header->key_free_beg;
				++m_ptr_header->key_alloc_cnt;
				cp_with_buf(&(m_ptr_header->key_alloc_cnt), &(m_ptr_header->key_alloc_cnt), sizeof(u32), offsetof(table_header_s, key_alloc_cnt), m_ci_idx);

			}	
			++m_ptr_header->key_cnt;
			cp_with_buf(&(m_ptr_header->key_cnt), &(m_ptr_header->key_cnt),  sizeof(u32), offsetof(table_header_s, key_cnt), m_ci_idx);

		}
		else //find and append
		{

			off64 = PAGE_MIN_SIZE + m_buck_cnt*sizeof(u32) + off*m_hash_node_sz;
			cp_with_buf(&(p_hn->node_off), &inod_off, sizeof(u32), off64 + offsetof(hash_node_s, node_off), m_ci_idx);//p_hn->node_off = inod_off;
		}
		return true;
	}

	bool put_no_save(const Kty& k, const Vty* ptr_val)
	{
		u64 hasher = hash_wrap(k);
		u32 id = mod2pow_num(hasher, m_buck_cnt);
		u32 off = m_ptr_bucket[id];
		hash_node_s* p_hn;
		u32 inod_off = 0;
		while(off)
		{
			p_hn = (hash_node_s*)get_node_ptr(off, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);

			if (p_hn->hasher == hasher)
			{
				inod_off = p_hn->node_off;
				break;
			}
			off = p_hn->next;
		}



		inod_off = store_ivt_node_no_save(ptr_val, inod_off);
		FALSE_RETURN(inod_off == 0)

			u64 off64;
		if (off == 0) //find none
		{
			if (m_ptr_header->key_free_beg != m_ptr_header->key_free_end ) //  free space left
			{
				p_hn = (hash_node_s*)get_node_ptr(m_ptr_header->key_free_beg, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);
				u32 next_free_off = *(u32*)p_hn;
				off64 = PAGE_MIN_SIZE + m_buck_cnt*sizeof(u32) + m_ptr_header->key_free_beg*m_hash_node_sz;
				p_hn->hasher = hasher;
				p_hn->node_off = inod_off;
				p_hn->next = m_ptr_bucket[id];
				m_ptr_bucket[id] = m_ptr_header->key_free_beg;
				m_ptr_header->key_free_beg = next_free_off;
				//used one free block
			}
			else //alloc new block ,0 is empty
			{
				if (m_ptr_header->key_alloc_cnt >= m_slice_sz*m_ptr_header->slice_cnt)
				{
					FALSE_RETURN(m_ptr_header->slice_cnt >= SLICE_MAX_CNT)
					m_ptr_slices[m_ptr_header->slice_cnt] = new char[m_slice_buf_len];
					FALSE_RETURN(m_ptr_slices[m_ptr_header->slice_cnt] == NULL);
					++m_ptr_header->slice_cnt;
					size_t new_sz = PAGE_MIN_SIZE + m_ptr_header->bucket_cnt * 4 + m_slice_buf_len * (m_ptr_header->slice_cnt);
					FALSE_RETURN(0 > ftruncate(m_fd, new_sz));
				}
				p_hn = (hash_node_s*)get_node_ptr(m_ptr_header->key_alloc_cnt, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);
				off64 = PAGE_MIN_SIZE + m_buck_cnt*sizeof(u32) + m_ptr_header->key_alloc_cnt*m_hash_node_sz;
				p_hn->hasher = hasher;
				p_hn->node_off = inod_off;
				p_hn->next = m_ptr_bucket[id];
				m_ptr_bucket[id] = m_ptr_header->key_alloc_cnt;
				++m_ptr_header->key_alloc_cnt;
			}	
			++m_ptr_header->key_cnt;
		}
		else //find and append
		{

			off64 = PAGE_MIN_SIZE + m_buck_cnt*sizeof(u32) + off*m_hash_node_sz;
			p_hn->node_off = inod_off;
		}
		return true;
	}

	bool dump_to_file()
	{
		if(0 > lseek(m_fd, 0, SEEK_SET))
			return false;
		FALSE_RETURN(!batch_write(m_fd, (char*)m_ptr_header, PAGE_MIN_SIZE));
		FALSE_RETURN(!batch_write(m_fd, (char*)m_ptr_bucket, m_ptr_header->bucket_cnt * sizeof(u32)));
		for (int i = 0; i < m_ptr_header->slice_cnt; ++i)
		{
			FALSE_RETURN(!batch_write(m_fd, m_ptr_slices[i], m_slice_buf_len));
		}

		if(0 > lseek(m_ivt_fd, 0, SEEK_SET))
			return false;
		for (int i = 0; i < m_ptr_header->inod_slice_cnt; ++i)
		{
			FALSE_RETURN(!batch_write(m_ivt_fd, m_ptr_ivt_slices[i], m_ivt_slice_buf_len));
		}

		return true;
	}

	bool dump_to_file_optimize()
	{

		u64 hasher;
		u32 off;
		hash_node_s* p_hn;
		const Vty * pval;
		char* out_buf = new char[m_ivt_slice_buf_len*m_ptr_header->inod_slice_cnt];
		FALSE_RETURN(!out_buf);

		char* cur = out_buf;
		u32 inc = 0;
		u32* p_nod_off;
		memset(cur, 0, m_ivt_node_sz);
		cur += m_ivt_node_sz;
		++inc;
		
		for (int i = 0; i < m_buck_cnt; ++i)
		{
			if (m_ptr_bucket[i] == 0)
			{
				continue;
			}

			off = m_ptr_bucket[i];
			while(off)
			{
				p_hn = (hash_node_s*)get_node_ptr(off, m_ptr_slices, m_slice_pow, m_slice_sz, m_hash_node_pow);

				pval = (Vty *)get_node_ptr(p_hn->node_off, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
				p_hn->node_off = inc;
				while(pval)
				{
					memcpy(cur, pval, m_ivt_node_sz);
					p_nod_off = (u32*)(cur + sizeof(Vty));
					++inc;
					cur += m_ivt_node_sz;
					if (*p_nod_off)
					{
						*p_nod_off = inc;
					}
					pval = next_ivt_node(pval);
				}
				off = p_hn->next;
			}

		}

		if(0 > lseek(m_fd, 0, SEEK_SET))
			return false;
		FALSE_RETURN(!batch_write(m_fd, (char*)m_ptr_header, PAGE_MIN_SIZE));
		FALSE_RETURN(!batch_write(m_fd, (char*)m_ptr_bucket, m_ptr_header->bucket_cnt * sizeof(u32)));
		for (int i = 0; i < m_ptr_header->slice_cnt; ++i)
		{
			FALSE_RETURN(!batch_write(m_fd, m_ptr_slices[i], m_slice_buf_len));
		}

		if(0 > lseek(m_ivt_fd, 0, SEEK_SET))
			return false;
		FALSE_RETURN(!batch_write(m_ivt_fd, out_buf, m_ivt_slice_buf_len * m_ptr_header->inod_slice_cnt));
		return true;
	}

	inline int get_inc_num()
	{
		return m_ptr_header->inc_sequence;
	}
 
	
	inline u32 size()
	{
		return m_ptr_header->key_cnt;
	}


	bool sync(int inc_num)
	{
		if (!m_init_ok)
			return false;
	
		if(!io_ajust(m_ci_ivt, m_ivt_name, inc_num))
			return false;
		if(!io_ajust(m_ci_idx, m_idx_name, inc_num))
			return false;

		 int ret = merge_to_trunk(m_ivt_name, m_ivt_fd, false) ;//first sync
		 if(ret >= SYNC_FAIL)
		 {
			 fprintf(stderr,"file:%s , line: %d, fail num =%d ,error info: %s\n",__FILE__,__LINE__, ret, strerror(errno));
			 return false;
		 }

		 ret = merge_to_trunk(m_idx_name, m_fd, true) ;//second sync ,inc_sequenc must be the last one
		 if(ret >= SYNC_FAIL)
		 {
			 fprintf(stderr,"file:%s , line: %d, fail num =%d ,error info: %s\n",__FILE__,__LINE__, ret, strerror(errno));
			 return false;
		 }
		 m_ptr_header->inc_sequence = inc_num;
		 return true;
	}

	bool near_limit()
	{
		return m_ci_idx.size() > DUMP_SIZE/2 || m_ci_ivt.size() > DUMP_SIZE/2;
	}

	void clear_last_file(const char* idx_file, const char* ivt_file)
	{
		remove(idx_file);
		remove(ivt_file);
		char diff_file[MAX_PATH];
		strcpy(diff_file, idx_file);
		strcat(diff_file, ".slice");
		remove(diff_file);
		strcpy(diff_file, ivt_file);
		strcat(diff_file, ".slice");
		remove(diff_file);
		
	}
	
	bool file_frozen(bool bGet)
	{
		if (bGet)
			return m_ptr_header->file_mark == SYNC_FAIL;
		else
		{
			u32 mark = SYNC_FAIL;
			lseek(m_fd, offsetof(table_header_s, file_mark), SEEK_SET);
			write(m_fd, &mark, sizeof(u32));
			m_ptr_header->file_mark = mark;
#ifndef _WIN32
			fsync(m_fd);
#endif
			return true;

		}
	}


	bool set_file_tm(int time_stamp){;}
	void view_header()
	{
		printf("inc_num:%d, bucket_num:%d, slice_size:%d, slice_count:%d, key_cnt:%d, key_alloc_cnt:%d, free_begin:%d, key_free_end:%d\n",
				m_ptr_header->inc_sequence, m_ptr_header->bucket_cnt,  m_ptr_header->slice_sz, m_ptr_header->slice_cnt, m_ptr_header->key_cnt,
				m_ptr_header->key_alloc_cnt,  m_ptr_header->key_free_beg,  m_ptr_header->key_free_end);
	}
	/*
   void view_stat_info()
   {
   printf("bucket_num:%d, value_size:%d, slice_size:%d, slice_count:%d, key_cnt:%d, key_alloc_cnt:%d, free_begin:%d, key_free_end:%d\n",
   m_ptr_header->bucket_cnt, m_ptr_header->value_sz, m_ptr_header->slice_sz, m_ptr_header->slice_cnt, m_ptr_header->key_cnt,
   m_ptr_header->key_alloc_cnt,  m_ptr_header->key_free_beg,  m_ptr_header->key_free_end);

   int stat[16] = {0};
			int off;
			int cnt;
			u64* p_hasher;
			for(int i = 0; i < m_ptr_header->bucket_cnt; ++i)
			{
				cnt = 0;
				off = m_ptr_bucket[i];
				while(off)
	                        {
			
					if (NULL == m_ptr_slices[off>>m_slice_pow])
					{
						if (!load_new_alloc_data(off))
							return ;
					}
	
					++cnt;
	                                p_hasher = get_hasher_ptr(off, m_ptr_slices, m_slice_pow, m_slice_sz, m_row_len);
	                                off = *get_off_ptr(p_hasher, m_next_off);
	                        }
				if (cnt == 0) continue;
				(cnt < 15) ? ++stat[cnt]:++stat[15];
			}
			for(int i = 0; i < 15; ++i)
				printf("%-10d bucket has  %-10d node\n", stat[i], i);
			printf("%-10d bucket has  >=15 node\n", stat[15]);
		}
	*/
	
private:



	inline u32 get_slice_cnt(u32 expect_sz, u32 val_sz)
	{
		u32 space = expect_sz * val_sz;
		if (space > PAGE_MAX_SIZE)
		{
			space = PAGE_MAX_SIZE;
		}
		else if (space < PAGE_MIN_SIZE)
		{
			space = PAGE_MIN_SIZE;
		}

		u32 s = 1;
		while(val_sz * (s<<=1) < space)
			;
		return s;
	}

	inline int get2pow(u32 m)//m must from get_2pow_size()
	{
		int pow = 0;
		int n = 1;
		while((n<<=1)<=m)
		{
			++pow;
		}
		return pow;
	}


	void cp_with_buf(void* dest, void* src, u32 len, u64 off, vector<change_mark_s>& vmark)
	{
		memcpy(dest, src, len);
		change_mark_s cm;
		cm.ptr = (char*)dest;
		cm.len = len;
		cm.id = vmark.size();
		cm.off = off;
		vmark.push_back(cm);
	}

	void add_last_block_len(vector<change_mark_s>& vmark, int len)
	{
		vmark.back().len += len;
	}

	bool io_ajust(vector<change_mark_s>& vec, const char* src_file, int inc_num)
	{
		vector<change_mark_s>::iterator it;
		sort(vec.begin(), vec.end());//io sort
		it = unique(vec.begin(), vec.end());//io distinct
		vec.erase(it, vec.end());
		it = GroupByMember(vec.begin(), vec.end(), continus_io, merge_io);
		vec.erase(it, vec.end());

		char name[MAX_PATH];
		strcpy(name, src_file);
		strcat(name, ".slice");
		int fd = open(name, O_RDWR|O_CREAT|_O_BINARY, 0644);

		int count = vec.size();
		FALSE_RETURN(fd==-1);
		FALSE_RETURN(write(fd, &m_mgnum, sizeof(int))!=sizeof(int));
		FALSE_RETURN(write(fd, &count, sizeof(int))!=sizeof(int));
		FALSE_RETURN(write(fd, &inc_num, sizeof(int))!=sizeof(int));

		u64 off;
		for (int i = 0; i < vec.size(); ++i) //io merge
		{
			//lseek(fd, vec[i].off, SEEK_SET);
			FALSE_RETURN(write(fd, &(vec[i].off), sizeof(u64)) != sizeof(u64));
			FALSE_RETURN(write(fd, &(vec[i].len), sizeof(u32)) != sizeof(u32));
			FALSE_RETURN(!batch_write(fd, vec[i].ptr, vec[i].len));
		}
		FALSE_RETURN(write(fd, &m_mgnum, sizeof(int))!=sizeof(int));
		close(fd);
		vec.clear();
		return true;
	}

	bool test_need_merge()
	{
		char diff_file[MAX_PATH];
		strcpy(diff_file, m_idx_name);
		strcat(diff_file, ".slice");
		vector<char> vec;
		u32* pb;
		u32* pe;
		int fd = ::open(diff_file, O_RDONLY|_O_BINARY, 0644);
		if (fd < 0)
		{
			goto NO_NEED;
		}

		struct stat statInfo;
		if (fstat(fd, &statInfo) < 0 || statInfo.st_size < 4*sizeof(u32)) 
		{
			close(fd);
			remove(diff_file);
			goto NO_NEED;
		}

		vec.resize(statInfo.st_size);
		if(!batch_read(fd, &vec[0], statInfo.st_size))
		{
			close(fd);
			remove(diff_file);
			goto NO_NEED;
		}
		pb = (u32*)&vec[0];
		pe = (u32*)((char*)pb + vec.size() - 4);
		if (*pb!=*pe || *pb != m_mgnum)
		{
			close(fd);
			remove(diff_file);
			goto NO_NEED;
		}
		return true;
NO_NEED:
		strcpy(diff_file, m_ivt_name);
		strcat(diff_file, ".slice");
		remove(diff_file);

		return false;

	}

	int merge_to_trunk(const char* trunk_file, int fd_tr, bool is_idx)
	{

		char diff_file[MAX_PATH];
		strcpy(diff_file, trunk_file);
		strcat(diff_file, ".slice");
		int fd = ::open(diff_file, O_RDONLY|_O_BINARY, 0644);
		if (fd < 0)
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return FILE_INVALID;
		}
		
		struct stat statInfo;
		if (fstat(fd, &statInfo) < 0 || statInfo.st_size < 4*sizeof(u32)) 
		{
			close(fd);
			remove(diff_file);
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return FILE_INVALID;
		}

		vector<char> vec(statInfo.st_size);
		if(!batch_read(fd, &vec[0], statInfo.st_size))
		{
			return SYNC_FAIL;
		}
		u32* pb = (u32*)&vec[0];
		u32* pe = (u32*)((char*)pb + vec.size() - 4);
		if (*pb!=*pe || *pb != m_mgnum)
		{
			return SYNC_FAIL + 1;
		}

		u32 count = *(++pb);
		u32 inc_num = *(++pb);

		u64 off64;
		u32 len;
		char* ps = (char*)(++pb);
		while(ps < (char*)pe)
		{
			off64 = *(u64*)ps;
			ps += sizeof(u64);
			len = *(u32*)ps;
			ps += sizeof(u32);
			if(0 > lseek(fd_tr, off64, SEEK_SET))
				return SYNC_FAIL + 3;
			if(!batch_write(fd_tr, ps, len))
				return SYNC_FAIL + 4;
			ps += len;
			--count;
			if(ps > (char*)pe || count < 0)
				return SYNC_FAIL + 5;			
		}

#ifndef _WIN32
		fsync(fd_tr);
#endif
		if (is_idx)
		{
			if(0 > lseek(fd_tr, offsetof(table_header_s, inc_sequence), SEEK_SET))
				return SYNC_FAIL + 6;
			if (write(fd_tr, &inc_num, sizeof(u32)) != sizeof(u32))
				return SYNC_FAIL + 7;
#ifndef _WIN32
			fsync(fd_tr);
#endif
		}

		close(fd);
		remove(diff_file);
		return SYNC_OK;
	}

	
	

	//return node off;
	u32 store_ivt_node(const Vty* ptr_val, u32 node_off)
	{
		u64 off64;
		if (m_ptr_header->inod_free_beg != m_ptr_header->inod_free_end)
		{
			Vty* p = (Vty*)get_node_ptr(m_ptr_header->inod_free_beg, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
			u32 next_free_off = *(u32*)p;
			
			off64 = m_ptr_header->inod_free_beg;
			off64 *= m_ivt_node_sz;
			cp_with_buf(p, (void*)ptr_val, sizeof(Vty), off64, m_ci_ivt);//*p = *ptr_val;
			++p;
			cp_with_buf((u32*)p, &node_off, sizeof(u32),off64+sizeof(Vty), m_ci_ivt);//*(u32*p) = node_off;
			add_last_block_len(m_ci_ivt, m_ivt_node_sz - (sizeof(Vty)+sizeof(u32)));
			node_off = m_ptr_header->inod_free_beg;
			cp_with_buf(&(m_ptr_header->inod_free_beg), &next_free_off, sizeof(next_free_off), offsetof(table_header_s, inod_free_beg),  m_ci_idx);//m_ptr_header->key_free_beg = next_free_off;
		}
		else
		{
			if (m_ptr_header->inod_alloc_cnt >= m_inod_slice_sz*m_ptr_header->inod_slice_cnt)
			{
				ZERO_RETURN(m_ptr_header->inod_slice_cnt >= SLICE_MAX_CNT);
				m_ptr_ivt_slices[m_ptr_header->inod_slice_cnt] = new char[m_ivt_slice_buf_len];
				ZERO_RETURN(m_ptr_ivt_slices[m_ptr_header->inod_slice_cnt] == NULL);
				++m_ptr_header->inod_slice_cnt;
				cp_with_buf(&(m_ptr_header->inod_slice_cnt), &(m_ptr_header->inod_slice_cnt), sizeof(u32), offsetof(table_header_s, inod_slice_cnt), m_ci_idx);
				u64  new_sz = (u64)m_ivt_slice_buf_len * (u64)(m_ptr_header->inod_slice_cnt);
				ZERO_RETURN(0 > ftruncate(m_ivt_fd, new_sz));

			}
			Vty* p = (Vty*)get_node_ptr(m_ptr_header->inod_alloc_cnt, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
			off64 = m_ptr_header->inod_alloc_cnt;
			off64 *= m_ivt_node_sz;
			cp_with_buf(p, (void*)ptr_val, sizeof(Vty), off64, m_ci_ivt);//*p = *ptr_val;
			++p;
			cp_with_buf((u32*)p, &node_off, sizeof(u32), off64+sizeof(Vty), m_ci_ivt);//*(u32*p) = node_off;
			add_last_block_len(m_ci_ivt, m_ivt_node_sz - (sizeof(Vty)+sizeof(u32)));
			node_off = m_ptr_header->inod_alloc_cnt;
			++m_ptr_header->inod_alloc_cnt;
			cp_with_buf(&(m_ptr_header->inod_alloc_cnt), &(m_ptr_header->inod_alloc_cnt),  sizeof(u32),offsetof(table_header_s, inod_alloc_cnt), m_ci_idx);
		}
		++m_ptr_header->inod_cnt;
		cp_with_buf(&(m_ptr_header->inod_cnt), &(m_ptr_header->inod_cnt),  sizeof(u32), offsetof(table_header_s, inod_cnt), m_ci_idx);
		return node_off;
	}
	u32 store_ivt_node_no_save(const Vty* ptr_val, u32 node_off)
	{
		u64 off64;
		if (m_ptr_header->inod_free_beg != m_ptr_header->inod_free_end)
		{
			Vty* p = (Vty*)get_node_ptr(m_ptr_header->inod_free_beg, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
			u32 next_free_off = *(u32*)p;
			
			off64 = m_ptr_header->inod_free_beg;
			off64 *= m_ivt_node_sz;
			*p = *ptr_val;
			++p;
			*(u32*)p = node_off;
			node_off = m_ptr_header->inod_free_beg;
			m_ptr_header->inod_free_beg = next_free_off;
		}
		else
		{
			if (m_ptr_header->inod_alloc_cnt >= m_inod_slice_sz*m_ptr_header->inod_slice_cnt)
			{
				ZERO_RETURN(m_ptr_header->inod_slice_cnt >= SLICE_MAX_CNT);
				m_ptr_ivt_slices[m_ptr_header->inod_slice_cnt] = new char[m_ivt_slice_buf_len];
				ZERO_RETURN(m_ptr_ivt_slices[m_ptr_header->inod_slice_cnt] == NULL);
				++m_ptr_header->inod_slice_cnt;
				u64  new_sz = (u64)m_ivt_slice_buf_len * (u64)(m_ptr_header->inod_slice_cnt);
				ZERO_RETURN(0 > ftruncate(m_ivt_fd, new_sz));

			}
			Vty* p = (Vty*)get_node_ptr(m_ptr_header->inod_alloc_cnt, m_ptr_ivt_slices, m_inod_slice_pow, m_inod_slice_sz, m_ivt_node_pow);
			off64 = m_ptr_header->inod_alloc_cnt;
			off64 *= m_ivt_node_sz;
			*p = *ptr_val;
			++p;
			*(u32*)p = node_off;
			node_off = m_ptr_header->inod_alloc_cnt;
			++m_ptr_header->inod_alloc_cnt;
		}
		++m_ptr_header->inod_cnt;
		return node_off;
	}
private:

	u32*  m_ptr_bucket;
	char* m_ptr_slices[SLICE_MAX_CNT];
	char* m_ptr_ivt_slices[SLICE_MAX_CNT];
	int   m_idx_mark_len;
	int   m_ivt_mark_len;
	vector<change_mark_s> m_ci_idx;
	vector<change_mark_s> m_ci_ivt;
	table_header_s* m_ptr_header;
	int   m_fd;
	int   m_ivt_fd;
	int   m_buck_cnt;
	int   m_slice_pow;
	int   m_slice_sz;
	int   m_inod_slice_pow;
	int   m_inod_slice_sz;
	int   m_ivt_node_sz;
	int   m_ivt_node_pow;
	int   m_hash_node_sz;
	int   m_hash_node_pow;
	int   m_checksum_off;
	int   m_next_off;
	int   m_slice_buf_len;
	int   m_ivt_slice_buf_len;

	bool  m_init_ok;
	int   m_mgnum;
	

	const char* m_idx_name;
	const char* m_ivt_name;

#ifndef _WIN32
	pthread_mutex_t m_mutexlock;
#endif
	//pthread_spin_init(&m_spinLock, PTHREAD_PROCESS_PRIVATE);

};
#endif
