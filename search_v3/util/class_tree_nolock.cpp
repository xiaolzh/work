#include "class_tree_nolock.h"
#include <algorithm>

using namespace std;

class_tree_nolock::class_tree_nolock()
{
	class_tree_node_t t;
	memset(&t, 0, sizeof(t));
	m_tree.push_back(t);
}

// you can insert when create ,once 
//lexical sort by 8 unsigned char , distinct, each path has weight 
bool class_tree_nolock::batch_insert(class_cnt_t* cls_cnt, int len)
{
	if (size()!=1)
		return false;
	class_tree_node_t t;
	class_tree_node_t* pt;
	class_tree_node_t* root = m_tree[0];
	unsigned char mark[8]={0};
	unsigned int  off[8]={0};
	int flag = 0;
	int mark_lev, cur_lev;
	int i, j;
	unsigned char c;
	unsigned long long cls;

	for (i = 0; i < len; ++i)
	{
		cls = cls_cnt[i].cls;
		if (cls == 0)
			continue;
		mark_lev = 0;
		cur_lev = 0;
		while((c = cls & 0xff) )
		{
			if (c == mark[mark_lev])
				++ mark_lev;
			cls >>= 8;
			++cur_lev;
		}

		if (mark_lev == 0)
			pt = root;
		else
			pt = m_tree[off[mark_lev - 1]];

		for(j = mark_lev; j < cur_lev-1; ++j)
		{
			c = cls_cnt[i].cls >> ((cur_lev-1)*8);
			t.bro = pt->son;
			t.cnt = 0;
			t.val =  c;
			t.son = 0;
			pt->son = m_tree.size();
			m_tree.push_back(t);
			pt = m_tree[m_tree.size()-1];
			fprintf(stderr, "file:%s,line:%d, warning class_tree format error\n", __FILE__, __LINE__);
			return false;
		}

		if (mark_lev+1 != cur_lev)
		{
			fprintf(stderr, "file:%s,line:%d,some thing wrong class_tree format error\n", __FILE__, __LINE__);
			return false;
		}

		c = cls_cnt[i].cls >> ((cur_lev-1)*8);
		t.bro = pt->son;
		t.cnt = cls_cnt[i].cnt;
		t.val =  c;
		t.son = 0;
		pt->son = m_tree.size();
		m_tree.push_back(t);
		mark[mark_lev] = c;
		off[mark_lev] = pt->son;
		if (mark_lev<7)
		{
			mark[mark_lev+1] = 0;
		}
	}
	return true;
}

//direct +1/-1
bool class_tree_nolock::batch_modify_class(unsigned long long* arr_cls, int len, int direct)
{
	class_tree_node_t t;
	class_tree_node_t* pt, *ptb;
	class_tree_node_t* root = m_tree[0];
	unsigned char mark[8]={0};
	unsigned int  off[8]={0};
	int off_node = 0;
	int mark_lev, cur_lev;
	int i, j;
	unsigned char c;
	unsigned long long cls;

	for (i = 0; i < len; ++i)
	{
		cls = arr_cls[i];
		if (cls == 0)
			continue;
		mark_lev = 0;
		cur_lev = 0;
		while((c = cls & 0xff) )
		{
			if (c == mark[mark_lev])
				++ mark_lev;
			cls >>= 8;
			++cur_lev;
		}

		if (mark_lev == 0)
			pt = root;
		else
			pt = m_tree[off[mark_lev - 1]];
		if (mark_lev == cur_lev)
		{
			pt->cnt += direct;
		}
		else if (mark_lev == cur_lev - 1)
		{
			
				c = arr_cls[i] >> ((cur_lev-1)*8);
				ptb = pt;
				pt = find_next_level_son(c, pt, off_node);
				if (pt->val)
				{
					pt->cnt += direct;
					off[mark_lev] = off_node;
				}
				else if (direct > 0)
				{
					t.bro = ptb->son;
					t.cnt = direct;
					t.val =  c;
					t.son = 0;
					m_tree.push_back(t);
					ptb->son = m_tree.size()-1;
					off[mark_lev] = ptb->son;
				}
				else
				{
					fprintf(stderr, "file:%s,line:%d,some thing wrong delete class_tree \n", __FILE__, __LINE__);
					return false;
				}
				
				mark[mark_lev] = c;
				if (mark_lev<7)
				{
					mark[mark_lev+1] = 0;
				}
		}
		else
		{
			fprintf(stderr, "file:%s,line:%d,some thing wrong delete class_tree \n", __FILE__, __LINE__);
			return false;
		}
	}
	return true;
}

class_tree_node_t* class_tree_nolock::get_class(unsigned long long cls)
{
	unsigned char c;
	class_tree_node_t* pt = m_tree[0];
	int off;
	while ((c = cls & 0xff))
	{
		pt = find_next_level_son(c, pt, off);
		if (!pt->val)
		{
			return m_tree[0];
		}
		cls >>=8;
	}
	return pt;
}

class_tree_node_t* class_tree_nolock::find_next_level_son(unsigned char c, class_tree_node_t* father, int& off)
{
	if (!father->son)
		return m_tree[0];
	off = father->son;
	class_tree_node_t * t;
	do 
	{
		t = m_tree[off];
	} while ((t->val != c) && (off = t->bro));

	if (t->val == c)
		return t;
	else
		return m_tree[0];
}
