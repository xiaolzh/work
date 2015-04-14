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
	bool LoadDicFromMem(VEC_PR_SD &vpr);//��Ȩ��
	//bool SplitSentence("");
	//����һ���ַ��������Ƿ����ֵ�����Ĵ� ���ظ���
	//************************************
	// Method:    SearchKeyWordInDic
	// FullName:  CDic_w::SearchKeyWordInDic
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: char* pchSrc             //Դ�ַ�����ָ��
	// Parameter: const char*pchEnd        //Դ�ַ���ĩָ��
	// Parameter: vector<string> &vecStr   //���ҵ��Ĵ�����
	//************************************ 
	void SearchKeyWordInDic( char* pchSrc,  char*pchEnd,VEC_PR_SD &vecStr);

	//����������Ӣ�Ľض���֤
	void SearchKeyWordInDicEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);

	//����ʵ�����û�ж�Ӧ�ķִʣ���ִ�Ȩ��ΪINT_MAX
	void Segment(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);


	//����������Ӣ�Ľض���֤�� ����ʵ�����û�ж�Ӧ�ķִʣ���ִ�Ȩ��ΪINT_MAX
	void SegmentEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);

	//������ʲôĳǰ׺��ʼ���ַ���
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
	vector<string> m_vecTrieKeys;//�ʵ�ؼ��ʼ��ϰ���TRie������˳��

	//hash_map<int, vector<TREE_NODE_W>*> m_hm1WordIdx;//���ֹ�ϣ����
	//TREE_NODE** m_pTree;


};
