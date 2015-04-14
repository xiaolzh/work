#pragma once
#include "DicCommon.h"

class CDic
{
public:
	enum {GBKWORD,GBKSYMBOL,GBKUNCOMMON,HALFGBK,NONGBK};
public:
	CDic(void);
	bool LoadDic(const char*pchDicFile);
	bool LoadDicFromMem(vector<string> &vs);
	//bool SplitSentence("");
	//搜索一个字符串里面是否有字典里面的词 有重复的
	//************************************
	// Method:    SearchKeyWordInDic
	// FullName:  CDic::SearchKeyWordInDic
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: char* pchSrc             //源字符串首指针
	// Parameter: const char*pchEnd        //源字符串末指针
	// Parameter: vector<string> &vecStr   //查找到的词数组
	//************************************ 
	void SearchKeyWordInDic( char* pchSrc,  char*pchEnd,vector<string> &vecStr);

	//加入了数字英文截断验证
	void SearchKeyWordInDicEx(char* pchSrc, char*pchEnd,vector<string> &vecStr);

	//搜索以什么某前缀开始的字符串
	void SearchBeginWith( char * pchSrc, char* pchEnd,vector<string> &vecStr);

	void SegmentEx(char* pchSrc, char*pchEnd,vector<string> &vecStr);
	void ClearData();
private:
	inline void  GetOneWord(char* &pchCur,const char*pchEnd,WORD& word);
	inline void PushOneKeyWord(WORD *pWord,int nValPos, vector<string>&vecStr);
	inline bool FindWord(WORD word,int &nCurPos,TREE_NODE* pTN);
	inline void PointBack(char*&pCur,WORD *pWord,int nRecordPos,int nValidPos);	

	inline int IsGBKWord(unsigned char ch1,unsigned char ch2);

public:
	~CDic(void);
private:
	vector<vector<TREE_NODE>* > m_vvpWordIdx;

	hash_map<int, vector<TREE_NODE>*> m_hm1WordIdx;//首字哈希索引
	//TREE_NODE** m_pTree;


};
