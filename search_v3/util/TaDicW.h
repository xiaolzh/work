/*
usage 
1.�ֵ��ļ������ź���
2.���ڴ������ֵ�LoadDicFromMem���� �ٵ���Build2Array
3.��������ʹ�÷���ƥ�䣬����ʹ������
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


struct PosCnt //�ڵ���TRIE�е�λ�ã��������ӽڵ�����
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
	bool LoadDicFromMem(VEC_PR_SD &vpr,bool bReverse);//��Ȩ��
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
	//����������Ӣ�Ľض���֤
	void SearchKeyWordInDicEx(char* pchSrc, char*pchEnd,VEC_PR_II &vecPII);

	//����ʵ�����û�ж�Ӧ�ķִʣ���ִ�Ȩ��ΪINT_MAX
	void Segment(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);


	//����������Ӣ�Ľض���֤�� ����ʵ�����û�ж�Ӧ�ķִʣ���ִ�Ȩ��ΪINT_MAX
	void SegmentEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr);

	//������ʲôĳǰ׺��ʼ���ַ���
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
	vector<string> m_vecTrieKeys;//�ʵ�ؼ��ʼ��ϰ���TRie������˳��

	//hash_map<int, vector<TREE_NODE_W>*> m_hm1WordIdx;//���ֹ�ϣ����
	//TREE_NODE** m_pTree;
	VW m_vwMapWord;//�����ֵ�ӳ��

	VI m_viBase; //ÿ��״̬�ͻ�������
	VI m_viCheck;//
	

	int m_nMaxBegWordNum;//���Ŀ�������ʼ�ֵĺ���
	int m_nMaxNum;//���ĵ������кš�



};
