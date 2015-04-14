#ifndef _BITMAP_H
#define _BITMAP_H
#include <vector>
using namespace std;

class CBitMap
{

public:
	CBitMap(unsigned int n)
	{
		unsigned int resCnt = n/8;
		if(n%8>0)
			++resCnt;
		m_v.resize(resCnt,0);
	}

	inline int GetBitCount(){return m_v.size()<<3;}
	inline void SetBit(unsigned int n){m_v[n>>3] |= (1<<(n&7));}
	inline bool TestBit(unsigned int n){return (m_v[n>>3] & (1<<(n&7)))!=0;}
	inline void ClearBit(unsigned int n){m_v[n>>3] &= (~(1<<(n&7)));}
	inline void ClearAll(){memset(&m_v[0],0,m_v.size());}

private:

	vector<unsigned char> m_v;
};
#endif
