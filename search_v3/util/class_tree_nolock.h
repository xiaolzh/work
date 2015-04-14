#ifndef CLASS_TREE_H
#define CLASS_TREE_H
/*
*by liugang 2011.12.26
*class tree  
*one modify  multi get lockfree
*/
#include <algorithm>
#include "array_rp_nolock.h"
using namespace std;
struct  class_tree_node_t
{
	unsigned int val;//just use 1byte
	unsigned int cnt;
	unsigned int bro;
	unsigned int son;
};
struct class_cnt_t
{
	unsigned long long cls;
	unsigned int cnt;
};

inline bool cmp_class_lexical(unsigned long long l, unsigned long long r)
{
	return lexicographical_compare((unsigned char*)&l, (unsigned char*)&l+8, (unsigned char*)&r, (unsigned char*)&r+8);
} 
inline bool cmp_class_cnt(const class_cnt_t& l, const class_cnt_t& r)
{
	return cmp_class_lexical(l.cls, r.cls);
}

class class_tree_nolock
{
public:

	class_tree_nolock();

	inline int size(){return m_tree.size();}

	// you can insert when create ,once 
	//lexical sort by 8 unsigned char , distinct, each path has weight 
	bool batch_insert(class_cnt_t* cls_arr,int len);
	//direct +1/-1
	bool batch_modify_class(unsigned long long* cls, int len, int direct);
	class_tree_node_t* get_class(unsigned long long cls);

	inline class_tree_node_t* get_first_son(class_tree_node_t* org_cls)
	{
			return m_tree[org_cls->son];
	}

	inline class_tree_node_t* get_next_bro(class_tree_node_t* org_cls)
	{
			return m_tree[org_cls->bro];
	}

private:
	class_tree_node_t* find_next_level_son(unsigned char c, class_tree_node_t* father, int& off);
private:
	array_rp_nolock<class_tree_node_t> m_tree;
};

#endif
