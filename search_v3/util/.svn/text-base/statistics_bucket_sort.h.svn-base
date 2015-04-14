#include <assert.h>
#include <stdlib.h>
#include "../searcher/ShowTime.h" 

template<class T>
void statistics_bucket_partial_sort(int bucket_max, T* beg, T* end)
{


}

struct off_cnt_s
{
	int off;
	int cnt;
};

struct id_off_s
{
	int id;
	int off;
};


template<class T>
void bucket_partial_sort(T* beg,  int cnt, int low, int high, int from, int to)
{
	if (from < 0 || from >= cnt || to <= from || to > cnt || cnt <= 0)
		return;
	if (high < low)
		return;

	int i,bid;
	int buck_num = high - low + 1;
	int ret_cnt = to - from ;

	off_cnt_s* p_buck = (off_cnt_s*)calloc(buck_num, sizeof(off_cnt_s));
	id_off_s* p_chain = (id_off_s*)malloc((cnt + 1) * sizeof(id_off_s));
	T* p_swap = (T*)malloc((ret_cnt)*sizeof(T));

	
	int diff_cnt = 0;
	int* p_src_seq = (int*)malloc((ret_cnt)*sizeof(int)*2);
	int* p_dest_seq = p_src_seq + ret_cnt;
	for (i = 0; i < ret_cnt; ++i)
	{
		p_dest_seq[i] = from + i;
	}
	//put
	for (i = 0; i < cnt; )
	{
		bid = beg[i]() - low;
		++i;
		p_chain[i].id = i - 1;
		p_chain[i].off = p_buck[bid].off;
		p_buck[bid].off = i;
		++p_buck[bid].cnt;
	}
	//get 
	id_off_s* p_tc;
	T* p_tmp = p_swap;
	// find start
	int acc = 0;
	for (i = 0; acc < from; ++i)
		acc += p_buck[i].cnt;

	if (acc - from == 0)
	{
		while (p_buck[i].cnt == 0)
			++i;
		p_tc =  p_chain + p_buck[i].off; 
	}
	else
	{
		--i;
		p_tc = p_chain + p_buck[i].off;
		acc -= p_buck[i].cnt;
		while(acc != from)
		{
			p_tc = p_chain + p_tc->off;
			++acc;
		}
	}

	// move to tmp and find diff set
	int id,j=0;
	assert(acc < to);
	while(true)
	{
		id = p_tc->id;
		*p_tmp = beg[id];

		if (from <= id && id < to)
			p_dest_seq[id - from] = -1;
		else
			p_src_seq[diff_cnt++] = id;

		++acc;
		++p_tmp;
		if (acc == to)
			break;

		if (p_tc->off == 0)
		{
			++i;
			while(p_buck[i].cnt == 0)
				++i;
			p_tc = p_chain + p_buck[i].off;
		}
		else
			p_tc = p_chain + p_tc->off;
	}
	//swap diff set
	for (i = 0,j = 0; i < ret_cnt; ++i)
	{
		if (p_dest_seq[i] != -1)
			p_dest_seq[j++] = p_dest_seq[i];
	}
	assert(j == diff_cnt);
	for(i = 0; i < diff_cnt; ++i)
		beg[p_src_seq[i]] = beg[p_dest_seq[i]];

	// sort range to dest
	memcpy(beg+from, p_swap, ret_cnt*sizeof(T));
	free(p_buck);
	free(p_chain);
	free(p_swap);
	free(p_src_seq);

	return ;
}
