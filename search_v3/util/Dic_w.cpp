//#include "stdafx.h"
#include "Dic_w.h"


CDic_w::CDic_w(void)
{
}

CDic_w::~CDic_w(void)
{
	ClearData();
}

inline bool KeyWordCmp(const KEY_WORD_W& l,const KEY_WORD_W& r)
{
	return lexicographical_compare(l.word,l.word+MAX_LEN,r.word,r.word+MAX_LEN);

}


inline bool TestKeyCmp(const TEST_KEY& l,const TEST_KEY& r)
{
  return strcmp(l.chBuf,r.chBuf)<0;
}

bool CDic_w::LoadDicFromMem(VEC_PR_SD &vpr)
{	
	char chBuf[1024];
	char *pchBuf;

	vector<KEY_WORD_W> vecStr;
	KEY_WORD_W keyWord;
	WORD *pWord;

	//[[����Ӣ����ȫ����˫�ֽڴ��**************************
	int i=0;
	for(i=0;i<vpr.size();++i)
	{  
		
		strcpy(chBuf,vpr[i].first.c_str());		
		memset(keyWord.word,0,MAX_LEN*sizeof(WORD));
		pWord=keyWord.word;
		pchBuf=chBuf;
		while(*pchBuf)
		{
			if (*pchBuf=='\r'||*pchBuf=='\n')
			{
				*pchBuf=0;
			}
			++pchBuf;
		}
		pchBuf=chBuf;
		if (strlen(chBuf)==0)
		{
			continue;
		}

		while (*pchBuf!=0)//����Ӣ����ȫ����˫�ֽڴ��
		{
			if (*pchBuf<0&&pchBuf[1]!=0&&IsGBKWord(*pchBuf,pchBuf[1])<HALFGBK)//������
			{
				*pWord=*(WORD*)pchBuf;
				++pWord;
				pchBuf+=2;
			}
			else//Ӣ�Ļ���������
			{
				*pWord=*(BYTE*)pchBuf;
				++pWord;
				++pchBuf;
			}				
		}	
		//]]����Ӣ����ȫ����˫�ֽڴ��**************************
		keyWord.dwWeight=vpr[i].second;
		vecStr.push_back(keyWord);
		

	}

	sort(vecStr.begin(),vecStr.end(),KeyWordCmp);

	//cout<<"input complete\n";


	//[[�����ֵ�******************************
	vector<KEY_WORD_W>::iterator itTest;//ͬһ���ֵĿ�ʼ
	vector<KEY_WORD_W>::iterator itF;//ͬһ���ֵĿ�ʼ
	vector<KEY_WORD_W>::iterator itE;//ͬһ���ֵĽ���
	vector<KEY_WORD_W>::iterator it;
	vector<vector<KEY_WORD_W>::iterator> vecIter; 
	vector<vector<KEY_WORD_W>::iterator>::iterator itVecIter;
	WORD wordFrst;
	WORD wordCur;
	WORD wordIndex;

	WORD preLevelWordFrst;
	int nPreLevelPosFrst;
	WORD preLevelWordCur;
	int nPreLevelPosCur;


	int nLevel;
	int nlevelCount;//ÿ�����
	int nSubCnt;//ÿ���ڵ��ӽڵ���
	TREE_NODE_W treeNode;

	m_vvpWordIdx.resize(65536);
	hash_map<int,int> hmSeg;//һ���������ֵ�ӳ�䣻wordCode and level->һ�� INT
	int nKey;
	for (it=vecStr.begin();it!=vecStr.end();/*++it*/)
	{					
		itF=it;
		wordIndex=itF->word[0];

		if (m_vvpWordIdx[wordIndex]!=NULL)
			continue;
		m_vvpWordIdx[wordIndex]=new vector<TREE_NODE_W>;

		memset(&treeNode,0,sizeof treeNode);
		treeNode.wData=itF->word[0];//�����ݿ�����ȥ��
		//memcpy(treeNode.chBuf+2,itF->word,2);
		//treeNode.chBuf[4]=0;                 //���ýڵ�������	

		m_vvpWordIdx[wordIndex]->push_back(treeNode);

		//ȷ��ͬһ���ֿ�ʼ�Ķ�		
		while (it!=vecStr.end()&&(it)->word[0]==wordIndex)//������ͬ
		{	
			if (it->word[1]!=0)//�����еڶ�����
			{
				vecIter.push_back(it);
			}
			else
			{
				nKey=(it-itF)*MAX_LEN;
				hmSeg.insert(pair<int,int>(nKey,0));//����λ����Ϣ		
				m_vvpWordIdx[wordIndex]->front().bEnd=1;//����һ�����Ѿ��ɴ�
				m_vvpWordIdx[wordIndex]->front().dwWeight=it->dwWeight;
			}			
			++it;
		}
		//itE=it;
		nLevel=1;//�����2���Ժ�
		//preLevelWordPos=-1;
		hmSeg.clear();
		while (!vecIter.empty())
		{   	
			wordFrst=0;
			nlevelCount=0;
			nSubCnt=0;

			preLevelWordFrst=0;
			nPreLevelPosFrst=-1;
			for (itVecIter=vecIter.begin();itVecIter!=vecIter.end();)
			{

				//[[���ڸ��ڵ�*****************************************************************
				preLevelWordCur=(*itVecIter)->word[nLevel-1];
				//nPreLevelPosCur=
				wordCur=(*itVecIter)->word[nLevel];

				nKey=((*itVecIter)-itF)*MAX_LEN+nLevel-1;//���ڵ��ڷֶξ����е�һάλ��	
				if (preLevelWordCur!=preLevelWordFrst||(preLevelWordCur==preLevelWordFrst&&hmSeg[nKey]!=hmSeg[nPreLevelPosFrst]))//���ڵ�ı�
				{			    
					//���ô˸��ڵ�����ӽڵ���		
					(*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].wSubCnt=1;
					//*(WORD*)((*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].chBuf+4)=1;
					//���ø��ڵ��һ���ӽڵ����������λ�á�
					(*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].wSubBeg=m_vvpWordIdx[wordIndex]->size()-hmSeg[nKey];
					//*(WORD*)((*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].chBuf+6)=m_vvpWordIdx[wordIndex]->size()-hmSeg[nKey];
					preLevelWordFrst=preLevelWordCur;	
					wordFrst=0;//���ڵ�ı�ʱ�κ��ֶ����´�
				}
				else
				{						
					if (wordCur!=wordFrst)//ͬһ�����ڵ��³������ֳ�������
						++(*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].wSubCnt;//���ô˸��ڵ�����ӽڵ���			
					//++(*(WORD*)((*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].chBuf+4));		
				}
				//]]���ڸ��ڵ�*****************************************************************


				nKey=((*itVecIter)-itF)*MAX_LEN+nLevel;//��ǰ�ڵ��ڷֶξ����е�һάλ��	
				if (wordCur!=wordFrst)//��������
				{
					++nlevelCount;
					wordFrst=wordCur;

					memset(&treeNode,0,sizeof treeNode);
					treeNode.wData=wordCur;//�����ݿ�����ȥ��
					//memcpy(treeNode.chBuf+2,&wordCur,2);


					if ((*itVecIter)->word[nLevel+1]==0)//�����ֵ�·������Ϊ��
					{
						//treeNode.chBuf[0]=1;
						treeNode.bEnd=1;
						treeNode.dwWeight=(*itVecIter)->dwWeight;
						hmSeg.insert(pair<int,int>(nKey,m_vvpWordIdx[wordIndex]->size()));//����λ����Ϣ
						nPreLevelPosFrst=((*itVecIter)-itF)*MAX_LEN+nLevel-1;//���ڵ��ڷֶξ����е�һάλ��
						itVecIter=vecIter.erase(itVecIter);//���ٱ�������

					}
					else
					{	
						hmSeg.insert(pair<int,int>(nKey,m_vvpWordIdx[wordIndex]->size()));//����λ����Ϣ
						nPreLevelPosFrst=((*itVecIter)-itF)*MAX_LEN+nLevel-1;//���ڵ��ڷֶξ����е�һάλ��
						++itVecIter;
					}								
					m_vvpWordIdx[wordIndex]->push_back(treeNode);						
				}
				else
				{
					++itVecIter;
					hmSeg.insert(pair<int,int>(nKey,m_vvpWordIdx[wordIndex]->size()-1));//����λ����Ϣ	
				}							
			}
			++nLevel;
		}
		vecIter.clear();
		if (it==vecStr.end())
		{
			break;
		}
	}

	//in.close();
	return true;

}

void CDic_w::Segment(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr)

{

	// WORD word;
	char* pCur=pchSrc;
	// hash_map<int,vector<TREE_NODE>* >::iterator it;
	WORD recordWord[MAX_LEN];

	int nValidPos=-1;//ƥ������Чλ��
	int nRecordPos=-1;//ƥ���е�ǰλ��
	int nCurPos=0;//��ǰ�����������ڵ�
	DWORD dwWei=INT_MAX;//Ȩ��
	char* pMinAdvancePos;//��С��ǰ��λ��
	char* pBackCur;//����ÿһ���´ʵ���ʼ�ڵ㡣
	//  int nNextPos=0;//��һ�����ڵ�����
	// int nNextCnt=0;//�����ӽڵ��ܸ�����
	//   set<string> setStr;
	vecStr.clear();
	vector<TREE_NODE_W>*pV;
	TREE_NODE_W*pTN;

	while(pCur!=pchEnd)
	{	
		dwWei=INT_MAX;
		pBackCur=pCur;
		memset(recordWord,0,MAX_LEN*2);
		GetOneWord(pCur,pchEnd,*recordWord);
		if(*recordWord==0)
			break;	   

		/* it=m_vvpWordIdx[*recordWord];*/

		pV =m_vvpWordIdx[*recordWord];
		if (pV!=NULL)//�������
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur�Ѿ�ǰ������С��λ
			if (pTN[0].bEnd==1)//�����Ǵʲ�����
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//��һ�㿪ʼλ��
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//��һ��Ҫ�õ�������

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//��ֹ��
				{
					if (nValidPos>-1)//����Ч��
					{
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);	
// 						if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//ǰ������������Ӣ��
// 							||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//�������������Ӣ��
// 						{
// 							break;
// 						}
						PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

					}
					else
					{

						vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
						pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
					}
					break;
				}
				else//ȡ����һ����
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//�����ֿ���Ϊ��
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//����Ч��
						{
							PointBack(pCur,recordWord,nRecordPos,nValidPos);	
// 							if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//ǰ������������Ӣ��
// 								||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//�������������Ӣ��
// 							{
// 								break;
// 							}
							PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

						}
						else
						{
							vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
							pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//�������
		else
		{
			vecStr.push_back(make_pair(string(pBackCur,pCur),INT_MAX));
		}
	}// while(pCur!=pchEnd)

}


//����������Ӣ�Ľض���֤ ����ʵ�����û�ж��ڵķִʣ���ִ�Ȩ��ΪINT_MAX
void CDic_w::SegmentEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr)
{

	// WORD word;
	char* pCur=pchSrc;
	// hash_map<int,vector<TREE_NODE>* >::iterator it;
	WORD recordWord[MAX_LEN];

	int nValidPos=-1;//ƥ������Чλ��
	int nRecordPos=-1;//ƥ���е�ǰλ��
	int nCurPos=0;//��ǰ�����������ڵ�
	DWORD dwWei=INT_MAX;//Ȩ��
	char* pMinAdvancePos;//��С��ǰ��λ��
	char* pBackCur;//����ÿһ���´ʵ���ʼ�ڵ㡣
	//  int nNextPos=0;//��һ�����ڵ�����
	// int nNextCnt=0;//�����ӽڵ��ܸ�����
	//   set<string> setStr;
	vecStr.clear();
	vector<TREE_NODE_W>*pV;
	TREE_NODE_W*pTN;

	while(pCur!=pchEnd)
	{	
		dwWei=INT_MAX;
		pBackCur=pCur;
		memset(recordWord,0,MAX_LEN*2);
		GetOneWord(pCur,pchEnd,*recordWord);
		if(*recordWord==0)
			break;	   

		/* it=m_vvpWordIdx[*recordWord];*/

		pV =m_vvpWordIdx[*recordWord];
		if (pV!=NULL)//�������
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur�Ѿ�ǰ������С��λ
			if (pTN[0].bEnd==1)//�����Ǵʲ�����
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//��һ�㿪ʼλ��
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//��һ��Ҫ�õ�������

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//��ֹ��
				{
					if (nValidPos>-1)//����Ч��
					{
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);	
						if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//ǰ������������Ӣ��
							||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//�������������Ӣ��
						{
							vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
							pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
							break;
						}
						PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

					}
					else
					{

						vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
						pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
					}
					break;
				}
				else//ȡ����һ����
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//�����ֿ���Ϊ��
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//����Ч��
						{
							PointBack(pCur,recordWord,nRecordPos,nValidPos);	
							if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//ǰ������������Ӣ��
								||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//�������������Ӣ��
							{
								vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
								pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��

								break;
							}
							PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

						}
						else
						{
							vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
							pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//�������
		else
		{
			vecStr.push_back(make_pair(string(pBackCur,pCur),INT_MAX));
		}
	}// while(pCur!=pchEnd)

}

void CDic_w::SearchKeyWordInDicEx( char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr)
{

	// WORD word;
	char* pCur=pchSrc;
	// hash_map<int,vector<TREE_NODE>* >::iterator it;
	WORD recordWord[MAX_LEN];

	int nValidPos=-1;//ƥ������Чλ��
	int nRecordPos=-1;//ƥ���е�ǰλ��
	int nCurPos=0;//��ǰ�����������ڵ�
	DWORD dwWei=0;//Ȩ��
	char* pMinAdvancePos;//��С��ǰ��λ��
	char* pBackCur;//����ÿһ���´ʵ���ʼ�ڵ㡣
	//  int nNextPos=0;//��һ�����ڵ�����
	// int nNextCnt=0;//�����ӽڵ��ܸ�����
	//   set<string> setStr;
	vecStr.clear();
	vector<TREE_NODE_W>*pV;
	TREE_NODE_W*pTN;

	while(pCur!=pchEnd)
	{	  
		pBackCur=pCur;
		memset(recordWord,0,MAX_LEN*2);
		GetOneWord(pCur,pchEnd,*recordWord);
		if(*recordWord==0)
			break;	   

		/* it=m_vvpWordIdx[*recordWord];*/

		pV =m_vvpWordIdx[*recordWord];
		if (pV!=NULL)//�������
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur�Ѿ�ǰ������С��λ
			if (pTN[0].bEnd==1)//�����Ǵʲ�����
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//��һ�㿪ʼλ��
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//��һ��Ҫ�õ�������

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//��ֹ��
				{
					if (nValidPos>-1)//����Ч��
					{
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);	
						if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//ǰ������������Ӣ��
							||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//�������������Ӣ��
						{
							break;
						}
						PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

					}
					else
					{
						pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
					}
					break;
				}
				else//ȡ����һ����
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//�����ֿ���Ϊ��
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//����Ч��
						{
							PointBack(pCur,recordWord,nRecordPos,nValidPos);	
							if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//ǰ������������Ӣ��
								||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//�������������Ӣ��
							{
								break;
							}
							PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);
													 
						}
						else
						{
							pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//�������
		else
		{

		}
	}// while(pCur!=pchEnd)

}


void CDic_w::SearchKeyWordInDic( char* pchSrc,  char*pchEnd,VEC_PR_SD &vecStr)

{
  
  // WORD word;
   char* pCur=pchSrc;
  // hash_map<int,vector<TREE_NODE>* >::iterator it;
   WORD recordWord[MAX_LEN];

   int nValidPos=-1;//ƥ������Чλ��
   int nRecordPos=-1;//ƥ���е�ǰλ��
   int nCurPos=0;//��ǰ�����������ڵ�
   	DWORD dwWei=0;//Ȩ��
   char* pMinAdvancePos;//��С��ǰ��λ��
 //  int nNextPos=0;//��һ�����ڵ�����
  // int nNextCnt=0;//�����ӽڵ��ܸ�����
//   set<string> setStr;
   vecStr.clear();
   vector<TREE_NODE_W>*pV;
   TREE_NODE_W*pTN;
   
   while(pCur!=pchEnd)
   {	  
	   memset(recordWord,0,MAX_LEN*2);
	   GetOneWord(pCur,pchEnd,*recordWord);
	   if(*recordWord==0)
		   break;	   
	 
	   
	  /* it=m_vvpWordIdx[*recordWord];*/
	  
	   pV =m_vvpWordIdx[*recordWord];
	   if (pV!=NULL)//�������
	   {
		   pTN=&pV->front();
		   nRecordPos=0;//the first word		  
		   nCurPos=0;
		   nValidPos=-1;
		   pMinAdvancePos=pCur;//pCur�Ѿ�ǰ������С��λ
		   if (pTN[0].bEnd==1)//�����Ǵ�
		   {
			   dwWei=pTN[0].dwWeight;
			   nValidPos=nRecordPos;
		   }
		   //nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//��һ�㿪ʼλ��
		   //nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//��һ��Ҫ�õ�������

		   while (true)
		   {

			   GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
			   if(recordWord[nRecordPos]==0)//��ֹ��
			   {
				   if (nValidPos>-1)//����Ч��
				   {
					   PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);
					   //PointBack(pCur,recordWord,nRecordPos,nValidPos);						 
				   }
				   else
				   {
					   pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
				   }
				   break;
				   
			   }
			   else//ȡ����һ����
			   {
				   if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
				   {					   
					   if (pTN[nCurPos].bEnd==1)//�����ֿ���Ϊ��
					   {
						   nValidPos=nRecordPos;
						   dwWei=pTN[nCurPos].dwWeight;
					   }

				   }
				   else
				   {

					   if (nValidPos>-1)//����Ч��
					   {
						   PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);
						   PointBack(pCur,recordWord,nRecordPos,nValidPos);						 
					   }
					   else
					   {
						   pCur=pMinAdvancePos;//�ƽ�����С�ƽ�λ��
					   }
					   break;

				   }
			   }
		   }// while (true)

	   }//if (it!=m_hm1WordIdx.end())//�������
	   else
	   {

	   }
   }// while(pCur!=pchEnd)

}

void CDic_w::GetOneWord(char* &pCur,const char*pchEnd,WORD&word)
{
	if (pCur==pchEnd)
	{
		word=0;
		return;
	}
	if (*pCur<0&&pCur+1!=pchEnd/*&&IsGBKWord(*pCur,pCur[1])<HALFGBK*/)//һ��������
	{
		word=*(WORD*)pCur;
		pCur+=2;

	}
	else 
	{	
		word=*(BYTE*)pCur;
		++pCur;
	}

}


void CDic_w::PushOneKeyWord(WORD *pWord,int nValPos,DWORD dwWeight, VEC_PR_SD&vecStr)
{	
	char chBuf[MAX_LEN]={0};
	char *p1=chBuf;
	char *p2=(char*)pWord;
	for (int i=0;i<=nValPos;++i)
	{
		*p1=*p2;
		++p1;++p2;
		if (*p2!='\0')		
		{
			*p1=*p2;
			++p1;
		}
		++p2;		
	}

	vecStr.push_back(make_pair(chBuf,dwWeight));

}


struct FOBJ_CMP_NODE 
{
	inline bool operator()(const TREE_NODE_W&l,const TREE_NODE_W &r)
	{
		return  l.wData<r.wData;

	}
};


bool CDic_w::FindWord(WORD word,int &nCurPos,TREE_NODE_W* pTN)
{
	TREE_NODE_W*    pIt;
	TREE_NODE_W*    pItStart=pTN[nCurPos].wSubBeg+nCurPos+pTN;
	TREE_NODE_W*    pItEnd=pItStart+pTN[nCurPos].wSubCnt;
	
	if(pItEnd-pItStart>20)//����2������
	{
		TREE_NODE_W treeNode;
		treeNode.wData=word;

		pIt=lower_bound(pItStart,pItEnd,treeNode,FOBJ_CMP_NODE());
		if (pIt!=pItEnd&&word==pIt->wData)//�ҵ���
		{
			nCurPos=pIt-pTN;//�ҵ��˸ı� nCurPos
			return true;
		}
	}
	else
	{
		for (pIt=pItStart;pIt!=pItEnd;++pIt)
		{
			if (pIt->wData==word)
			{
				break;
			}
		}
		if (pIt!=pItEnd)//�ҵ���
		{
			nCurPos=pIt-pTN;//�ҵ��˸ı� nCurPos
			return true;
		}
		
	}
	return false;

}

void CDic_w::PointBack(char*&pCur,WORD *pWord,int nRecordPos,int nValidPos)
{

	while (nRecordPos!=nValidPos)
	{
		if ((pWord[nRecordPos]&0xFF00)==0)//���˵��ֽ�
		{
			--pCur;
		}
		else//����˫�ֽ�
		{
			--pCur;
			--pCur;
		}
		--nRecordPos;
	}

}
int CDic_w::IsGBKWord(unsigned char ch1,unsigned char ch2)
{
	if ((ch1 >= 129)&&(ch2 <= 254))
	{
		if (((ch2 >= 64)&&(ch2 < 127)) ||((ch2 > 127)&&(ch2 <= 254)))
		{//**
			int posit = (ch1 - 129) * 190 + (ch2 - 64) - (ch2/128);
			//posit = posit * 2;
			// gbk/3
			if(ch1>=0x81&&ch1<=0xA0&&ch2>=0x40&&ch2<=0xFE)
				return GBKUNCOMMON;
			// gbk/4
			if(ch1>=0xAA&&ch1<=0xFE&&ch2>=0x40&&ch2<=0xA0)
				return GBKUNCOMMON;
			if (posit>=0 && posit<=6079)
				return GBKWORD;
			else if(posit>=7790 && posit<=23845)
			{
				return GBKWORD;
			}
			else
				return GBKSYMBOL;
		}
		else
			return HALFGBK;
	}
	else	
		return NONGBK;

}

void CDic_w::ClearData()
{
	vector<vector<TREE_NODE_W>* > ::iterator it;
	for (it=m_vvpWordIdx.begin();it!=m_vvpWordIdx.end();++it)
	{
		delete *it;
	}
	m_vvpWordIdx.clear();
}

void CDic_w::SearchBeginWith( char * pchSrc, char* pchEnd,VEC_PR_SD&vecStr)
{

	if (!pchSrc||!*pchSrc)
	{
		return ;
	}
	// WORD word;
	char* pCur=pchSrc;	
	WORD recordWord[MAX_LEN*2]={0};

	int nPreFixCnt=0;
	int nCurPos=0;
	WORD word;
	while (pCur!=pchEnd)
	{
		GetOneWord(pCur,pchEnd,recordWord[nPreFixCnt++]);
	}

	vector<TREE_NODE_W>*pV;
	TREE_NODE_W*pTN;
	vector<pair<int,int> > vecPii;//��ջ��¼�����м����Ӽ��㣬��ǰ�����˼����ڵ㡣
	pV =m_vvpWordIdx[*recordWord];
	if (pV==NULL) return;

	pTN=&pV->front();
	nCurPos=0;
	for (int i=1;i<nPreFixCnt;++i)
	{
		if(!FindWord(recordWord[i],nCurPos,pTN))
			return;
	}
	vecPii.push_back(make_pair(nCurPos,0));//prefix ���һ���ڵ��λ�ã��ýڵ���ֽڵ�ķ��ʸ���

	while (!vecPii.empty())
	{

		while (pTN[nCurPos].wSubCnt)//�����ӽڵ����������
		{
			nCurPos=nCurPos+pTN[nCurPos].wSubBeg+vecPii.back().second;//��һ���ڵ�λ�ã�
			++vecPii.back().second;                                   //�ӽڵ����������1��
			vecPii.push_back(make_pair(nCurPos,0));                   //���һ���ڵ��λ��,�ýڵ���ֽڵ�ķ��ʸ���
			recordWord[nPreFixCnt++]=pTN[nCurPos].wData;              //����һ���ڵ�����ݣ�
			if (pTN[nCurPos].bEnd)
			{
				PushOneKeyWord(recordWord,nPreFixCnt-1,pTN[nCurPos].dwWeight,vecStr);
			}
		}
		do 
		{
			vecPii.pop_back();
			if (vecPii.empty())	break;
			--nPreFixCnt;		
			nCurPos=vecPii.back().first;
		} while(vecPii.back().second==pTN[nCurPos].wSubCnt);
	}


}