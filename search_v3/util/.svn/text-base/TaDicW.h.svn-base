/*
usage 
1.字典文件必须排好序
2.从内存载入字典LoadDicFromMem必须 再调用Build2Array
3.反向载入使用反向匹配，否则使用正向
*/
#pragma once
#include "DicCommon.h"
#include <queue>
#include <functional>
typedef pair<string,DWORD>           PSD;

typedef vector<pair<string,DWORD> >  VEC_PR_SD;
typedef vector<pair<string,string> > VEC_PR_SS;
typedef vector<pair<WORD,WORD> >     VEC_PR_WW;
typedef vector<pair<int,int> >       VEC_PR_II;
typedef vector<WORD>                 VW;
typedef vector<int>                  VI;
typedef vector<string>               VS;


struct PosCnt //节点在TRIE中的位置，包含的子节点数量
{
	int nPosInTrie;
    int nPosInBase;
	int nCnt;
	bool operator<(const PosCnt &pc)const
	{
		return this->nCnt<pc.nCnt;
	}
};

class CDic_w
{
public:
	enum {GBKWORD,GBKSYMBOL,GBKUNCOMMON,HALFGBK,NONGBK};
public:
	CDic_w(void);

	bool InitFromFile(const char *pchFile,bool bReverse);
/*	bool LoadDic(const char*pchDicFile);*/
	bool LoadDicFromMem(VEC_PR_SD &vpr,bool bReverse);//词权重
	//bool SplitSentence("");
	//搜索一个字符串里面是否有字典里面的词 有重复的
	//************************************
	// Method:    SearchKeyWordInDic
	// FullName:  CDic_w::SearchKeyWordInDic
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: char* pchSrc             //源字符串首指针
	// Parameter: const char*pchEnd        //源字符串末指针
	// Parameter: vector<string> &vecStr   //查找到的词数组
	//************************************ 
	void SearchKeyWordInDic( char* pchSrc,  char*pchEnd,VEC_PR_SD &vecStr);

	//加入了数字英文截断验证
	void SearchKeyWordInDicEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);
	//加入了数字英文截断验证
	void SearchKeyWordInDicEx(char* pchSrc, char*pchEnd,VEC_PR_II &vecPII);

	//如果词典里面没有对应的分词，则分词权重为INT_MAX
	void Segment(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);


	//加入了数字英文截断验证， 如果词典里面没有对应的分词，则分词权重为INT_MAX
	void SegmentEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);

	//搜索以什么某前缀开始的字符串
	void SearchBeginWith( char * pchSrc, char* pchEnd,VEC_PR_SD&vecStr);
	
	void ClearData();

public:
	void Build2Array();
	void SegmentExBy2ArrayFMM(char* pchSrc, char*pchEnd,VS &vecStr);
	void SegmentExBy2ArrayBMM(char* pchSrc, char*pchEnd,VS &vecStr);
	

	inline int MapOneWordToSequence(WORD w);
private:
	inline void  GetOneWord(char* &pchCur,const char*pchEnd,WORD& word);
	inline void PushOneKeyWord(WORD *pWord,int nValPos, DWORD dwWeight, VEC_PR_SD&vecStr);
	inline bool FindWord(WORD word,int &nCurPos,TREE_NODE_W* pTN);
	inline void PointBack(char*&pCur,WORD *pWord,int nRecordPos,int nValidPos);	

	inline int IsGBKWord(unsigned char ch1,unsigned char ch2);



public:
	~CDic_w(void);
private:
	vector<vector<TREE_NODE_W>* > m_vvpWordIdx;
	vector<string> m_vecTrieKeys;//词典关键词集合按照TRie树生成顺序。

	//hash_map<int, vector<TREE_NODE_W>*> m_hm1WordIdx;//首字哈希索引
	//TREE_NODE** m_pTree;
	VW m_vwMapWord;//中文字的映射

	VI m_viBase; //每个状态和基本数组
	VI m_viCheck;//
	

	int m_nMaxBegWordNum;//最大的可以作起始字的号码
	int m_nMaxNum;//最大的单字序列号。



};
