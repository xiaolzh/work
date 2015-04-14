#ifndef DICCOMMON_NEW_H
#define DICCOMMON_NEW_H

//#include "my_hash_map.h"
#include "common_libs.h"
#include <map>

namespace segment
{
	typedef std::vector<std::string>    VEC_STR;
	typedef std::vector<int>            VEC_INT;
	typedef std::map<std::string, int>	MAP_STR_INT;
	typedef std::pair<std::string, int> PAIR_STR_INT;
	typedef std::vector<PAIR_STR_INT>		VEC_PAIR_STR_INT;

	//字符编码
	struct TERM_CODE
	{
		std::vector<short> code;
		int weight;
	};
	enum lex_type{ambiguity, common, special, english}; //词典类型，汉歧/汉普/汉专/英专
  enum seg_type{omniSeg, maxSeg, minSeg};
	//trie
	struct TRIE_NODE
	{
		int weight;		//权重
		short wSubCnt;	//孩子个数
		short wSubBeg;	//第一子结点起始索引
		short wData;	//字序号
		bool bEnd;		//可成词标志
	};
	
	struct PosCnt 
	{	//结点在TRIE中的位置信息
		int nPosInBase;		//base下标
		short nPosInTrie;	//Trie树中位置
		short nCnt;				//子结点数
		bool operator<(const PosCnt &pc)const	//优先级
		{
			return this->nCnt < pc.nCnt;
		}
	};

	//************内部结构*************//
	struct DaTrie
	{	//double-array-trie
		DaTrie(void){};
		DaTrie(int b, int c, int w) :
					base(b), check(c), weight(w){}
		int base;
		int check;
		int weight;
	};

	struct KeyMap
	{ //chinese-word-mapping
		KeyMap(void){};
		KeyMap(unsigned short c, short i) :
					code(c), index(i){}
		unsigned short code;
		short index;
	};
	
	//*******分词结果结构*********//
	struct Term
	{
		Term(void){};
		Term(int p, short l, short le) : 
					pos(p), len(l), level(le){}
		
		bool operator==(const Term& t) const 
		{
			return this->pos == t.pos && this->len == t.len;
		}

		int pos;				//起始位置
		short len;			//词长（字节数）
		short level;		//词等级 [0-7] .实占3 bit 空间
	};
}
#endif
