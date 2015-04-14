#ifndef EFTTREE_H
#define EFTTREE_H

template <class NOD, class COL>
struct	_TREE_NODE
{
	NOD n;
	COL c;
};


template <class NOD, class COL>
void winner_tree_merge(_TREE_NODE<NOD, COL>* buf, NOD** start, NOD** end,
					   _TREE_NODE<NOD, COL>* dest, int col, NOD min_ele)
{
	int i ;
	const int leaf_beg = col;

	_TREE_NODE<NOD, COL>* p_nod = buf + col;
	for (i = 0; i < col; ++i)
	{
		if ( start[i] < end[i] ) {
			p_nod[i].n = *(start[i]);
			p_nod[i].c = i;
			++start[i];
		} else {
			p_nod[i].n = min_ele;
		}
	}
	//total = col*2-1;
	int r = col*2 - 1 ;
	int l = r - 1;
	while (l > 1) 
	{
		buf[l >> 1] = buf[l].n > buf[r].n ? buf[l] : buf[r];
		r-=2;
		l-=2;
	}
	
	p_nod = buf + 1;
	while(min_ele < p_nod->n)
	{
		*dest = *p_nod;
		++dest;
		i = p_nod->c;
		if (start[i] == end[i])
			buf[i + leaf_beg].n = min_ele;
		else
		{
			buf[i + leaf_beg].n = *(start[i]);
			++(start[i]);
		}
		i += leaf_beg;

		while (i > 1)
		{
			i = i  >> 1;
			l = i << 1;
			r = l + 1;
			buf[i] = buf[l].n > buf[r].n ? buf[l]: buf[r];
		};
	}
}

template <class NOD, class COL>
void loser_tree_merge(_TREE_NODE<NOD, COL>* buf, NOD** start, NOD** end,
					   _TREE_NODE<NOD, COL>* dest, int col, NOD max_ele)
{
	int i ;
	const int leaf_beg = col;

	_TREE_NODE<NOD, COL>* p_nod = buf + col;
	for (i = 0; i < col; ++i)
	{
		if ( start[i] < end[i] ) {
			p_nod[i].n = *(start[i]);
			p_nod[i].c = i;
			++start[i];
		} else {
			p_nod[i].n = max_ele;
		}
	}
	//total = col*2-1;
	int r = col*2 - 1 ;
	int l = r - 1;
	while (l > 1) 
	{
		buf[l >> 1] = buf[l].n < buf[r].n ? buf[l] : buf[r];
		r-=2;
		l-=2;
	}
	
	p_nod = buf + 1;
	while(max_ele > p_nod->n)
	{
		*dest = *p_nod;
		++dest;
		i = p_nod->c;
		if (start[i] == end[i])
			buf[i + leaf_beg].n = max_ele;
		else
		{
			buf[i + leaf_beg].n = *(start[i]);
			++(start[i]);
		}
		i += leaf_beg;

		while (i > 1)
		{
			i = i  >> 1;
			l = i << 1;
			r = l + 1;
			buf[i] = buf[l].n < buf[r].n ? buf[l]: buf[r];
		};
	}
}
#endif
