#pragma once
#include "DicCommon.h"

typedef vector<pair<string,DWORD> > VEC_PR_SD;
typedef vector<pair<string,string> > VEC_PR_SS;

class CDic_w
{
public:
	enum {GBKWORD,GBKSYMBOL,GBKUNCOMMON,HALFGBK,NONGBK};
public:
	CDic_w(void);
/*	bool LoadDic(const char*pchDicFile);*/
	bool LoadDicFromMem(VEC_PR_SD &vpr);//词权重
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

	//如果词典里面没有对应的分词，则分词权重为INT_MAX
	void Segment(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);


	//加入了数字英文截断验证， 如果词典里面没有对应的分词，则分词权重为INT_MAX
	void SegmentEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);

	//搜索以什么某前缀开始的字符串
	void SearchBeginWith( char * pchSrc, char* pchEnd,VEC_PR_SD&vecStr);
	
	void ClearData();
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


};
