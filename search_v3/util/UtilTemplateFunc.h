#ifndef MY_T_FUNC_H
#define MY_T_FUNC_H
#include "UtilDef.h"
#include <algorithm>
using namespace std;

//比较函数，按照任意结构体的任一个成员进行=val
template<class T,class E>
struct	FOBJ_EqualBy1MemVal
{
	FOBJ_EqualBy1MemVal(int nOffset,E ele):m_nOffset(nOffset),m_ele(ele){}
	inline bool operator()(const T&r)const 
	{
		return *(E*)((char*)&r+m_nOffset)==m_ele;
	}

	int m_nOffset;
	int m_ele;

};


//比较函数，按照任意结构体的任一个成员进行=比较
template<class T,class E>
struct	FOBJ_EqualBy1Mem
{
	FOBJ_EqualBy1Mem(int nOffset):m_nOffset(nOffset){}
	inline bool operator()(const T&l,const T&r)const 
	{
		return *(E*)((char*)&l+m_nOffset)==*(E*)((char*)&r+m_nOffset);
	}

	int m_nOffset;

};


//比较函数，按照任意结构体的任一个成员进行<比较
template<class T,class E>
struct	FOBJ_LessBy1Mem
{
	FOBJ_LessBy1Mem(int nOffset):m_nOffset(nOffset){}
	inline bool operator()(const T&l,const T&r)const 
	{
		return *(E*)((char*)&l+m_nOffset)<*(E*)((char*)&r+m_nOffset);
	}

	int m_nOffset;

};


//比较函数，按照任意结构体的任一个成员进行>比较
template<class T,class E>
struct	FOBJ_GreatBy1Mem
{
	FOBJ_GreatBy1Mem(int nOffset):m_nOffset(nOffset){}
	inline bool operator()(const T&l,const T&r)const 
	{
		return *(E*)((char*)&l+m_nOffset)>*(E*)((char*)&r+m_nOffset);
	}

	int m_nOffset;
};
//将一个结构加到另一个结构 按照某个成员变量相加
template<class T,class E>
struct	FOBJ_AddBy1Mem
{
	FOBJ_AddBy1Mem(int nOffset):m_nOffset(nOffset){}
	inline E& operator()(const T&l,const T&r)const 
	{
		return *(E*)((char*)&l+m_nOffset)+=*(E*)((char*)&r+m_nOffset);
	}

	int m_nOffset;
};


//求交集，相同元素返回第一列的值
template<class I1,class I2>
I1 My_Set_InterSect(I1 f1,I1 l1,I2 f2,I2 l2,I1 r)
{
	while (f1!=l1&&f2!=l2)
	{
		if (*f1<*f2) ++f1;
		else if (*f2<*f1) ++f2;
		else
		{
			*r=*f1;
			++f1;
			++f2;
			++r;
		}
	}
	return r;
}



//求交集，相同元素返回第一列的值，第一列相邻的相同值，全部返还
template<class I1,class I2>
I1 My_Set_InterSect_col1_all(I1 f1,I1 l1,I2 f2,I2 l2,I1 r)
{
	while (f1!=l1&&f2!=l2)
	{
		if (*f1<*f2) ++f1;
		else if (*f2<*f1) ++f2;
		else
		{
			*r=*f1;
			++f1;
			//++f2;
			++r;
		}
	}
	return r;
}

//按照结构体的某个成员分组汇总函数，以结构体中某个元素为基准把该元素相同的节点的某个其他元素全部累加。
//{1,1: 2,1: 2,2: 2,3: 3,5}->{1,1: 2,6: 3,5}
//************************************
// Method:    GroupBySomeMember
// FullName:  <_FwdIt, _Pr, _PrAdd>::GroupBySomeMember
// Access:    public 
// Returns:   _FwdIt              该迭代器指向有效序列的下一个节点
// Qualifier:
// Parameter: _FwdIt _First    
// Parameter: _FwdIt _Last
// Parameter: _Pr    _PredEqual  以某个成员为标准相等
// Parameter: _PrAdd _PredAdd    以某个成员为标准相加
//************************************
template<class _FwdIt,
class _PrEqual,class _PrAdd> inline
	_FwdIt GroupBySomeMember(_FwdIt _First, _FwdIt _Last, _PrEqual _PredEqual,_PrAdd _PredAdd)
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


//************************************

// Parameter: int nStart      从第几个元素开始要求排序
// Parameter: int nEnd        从第几个元素中止要求排序
//************************************
template<class TYPE>
void SortRange(vector<TYPE>& vec, int nStart,int nEnd)
{
	bool bReverseIt=nStart+nEnd>(int)vec.size()?true:false;
	if (bReverseIt)//反向迭代器
	{
		partial_sort(vec.rbegin(),vec.rbegin()+vec.size()-nStart,vec.rend());
	}
	else
	{
		partial_sort(vec.begin(),vec.begin()+nEnd,vec.end(),greater<TYPE>());
	}

}

template<class TYPE,  class BinaryPredicate>
void SortRangeWithCFunc(vector<TYPE>& vec, int nStart,int nEnd, BinaryPredicate cmp)
{
//	bool bReverseIt=nStart+nEnd>vec.size()?true:false;
//	if (bReverseIt)//反向迭代器
//	{
//		partial_sort(vec.rbegin(),vec.rbegin()+vec.size()-nStart,vec.rend(),cmp);
//	}
//	else
//	{
		partial_sort(vec.begin(),vec.begin()+nEnd,vec.end(),cmp);
//	}

}

//查找哈希表的键值 如果失败 返回vBadVal
template <class K,class V>
const V& FindHashValue( K k, hash_map<K,V> &hm, V vBadVal)
{
	typename hash_map<K,V>::iterator i=hm.find(k);
	if (i==hm.end())
	{
		return vBadVal;
	}
	return i->second;
};


template<class K,class V>//如果有值返回原来的
const V& AddHMElement(const K &key,const V& value,hash_map<K,V>& hm,bool &bExist)
{
	if (hm.find(key)==hm.end())
	{
		bExist=false;
		hm[key]=value;
		return value;
	}
	else
	{
		bExist=true;
		return hm[key];
	}
}

//
template<class K,class V>//如果有值返回原来的
const V& AddHMKeyAutoVal(const K &key,hash_map<K,V>& hm)
{

	typename hash_map<K,V>::iterator it=hm.find(key);
	if (it==hm.end())
	{
		hm.insert(make_pair(key,hm.size()))		;
		return hm.size()-1;
	}
	else
	{		
		return it->second;
	}
}

struct FROM_TO
{
	int nFrom;
	int nCount;
};


template<class V>
struct	FOBJ_COMP_NODE
{
	typedef bool (*CMP_FUNC)(const V&l,const V &r); 

	FOBJ_COMP_NODE(CMP_FUNC pFunc):m_pFUnc(pFunc){}

	bool operator()(const pair<int,V>&l,const pair<int,V>&r)const
	{
		return l.first < r.first||
			   l.first==r.first&&m_pFUnc(l.second,r.second);
	}

	CMP_FUNC m_pFUnc;


};

	

template<class K,class V>
class	CTemplateIndex
{
	typedef vector<FROM_TO>    VFT;
	typedef vector<V>          VIVT;
	typedef vector<pair<int,V> > VPKV;
	typedef hash_map<K,int>    HKI;
	typedef bool (*CMP_V)(const V&l,const V &r);
	

public:

	CTemplateIndex(){;}
	void AddElement(K& k,const V &v)
	{
		int nKid=AddHMKeyAutoVal(k,m_hmDic);
		m_vTemp.push_back(make_pair(nKid,v));
	}

	void GenerateFinalIndex(int nEachIndexMaxCnt,CMP_V pFunc)
	{
		if (m_vTemp.empty()) return;
		sort(m_vTemp.begin(),m_vTemp.end(),FOBJ_COMP_NODE<V>(pFunc));
		int nTempCnt=0;
		int nTempKey;
		if (nEachIndexMaxCnt)
		{
			int i=0;
			int j=1;
			int k=0;
			while (j<m_vTemp.size())
			{
				while(j<m_vTemp.size()&&m_vTemp[i].first==m_vTemp[j].first)
					++j;			
				if (j-i>nEachIndexMaxCnt)
				{
					i+=nEachIndexMaxCnt;				
					while(j<m_vTemp.size())
					{
						m_vTemp[i]=m_vTemp[j];						
						nTempKey=m_vTemp[j].first;
						nTempCnt=1;
						++i;++j;
						while (j<m_vTemp.size()&&m_vTemp[j].first==nTempKey)
						{
							if (nTempCnt<nEachIndexMaxCnt)
							{
								m_vTemp[i]=m_vTemp[j];		
								++i;							
							}							
							++nTempCnt;
							++j;
						}							

					}	
				}
				else
				{
					i=j;
					j=i+1;
				}
			}
			m_vTemp.erase(m_vTemp.begin()+i,m_vTemp.end());
		}
		m_vFromTo.resize(m_vTemp.back().first+1);
		int nId=-1;
		for (int i=0;i<m_vTemp.size();++i)
		{
			if (nId!=m_vTemp[i].first)			
			{
				nId=m_vTemp[i].first;
				m_vFromTo[nId].nFrom=m_vIvt.size();
				
			}
			++m_vFromTo[nId].nCount;
			m_vIvt.push_back(m_vTemp[i].second);			
		}
		m_vTemp.swap(VPKV());
	}


	pair<V*,V*> GetSomeIdx(K& k)
	{
		typename HKI::iterator i=m_hmDic.find(k);
		if (i==m_hmDic.end())
			return make_pair((V*)NULL,(V*)NULL);
		V* pFrom=&m_vIvt[0]+m_vFromTo[i->second].nFrom;

		return make_pair(pFrom,pFrom+m_vFromTo[i->second].nCount );
		
	}
	
private:
	//临时缓冲
	VPKV m_vTemp;

	VFT m_vFromTo;
	VIVT m_vIvt;

	//映射字典
	HKI m_hmDic;
	
};


#endif
