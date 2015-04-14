#include <typeinfo>
#include "UtilTemplateFunc.h"
#include "TaDicW.h"

using namespace std;

template <class K,class V>
bool Load2PairFile_V(const char* pchFileName,vector<pair<K,V> > &vec)
{
    FILE *fpIn=fopen(pchFileName,"rb");

    if(!fpIn)
    {
        //ErrorLog("打开文件失败! 出错位置: %s,%d",__FILE__,__LINE__);
        fclose(fpIn);
        return false;
    }
    char chBuf[255];
    char chBuf1[96];
    char chBuf2[96];
    int n;
    int nNum;
    while(!feof(fpIn))
    {
        fgets(chBuf,255,fpIn);
        n=strlen(chBuf);
        while(n>0 && (chBuf[n-1]=='\r' || chBuf[n-1]=='\n')) 
            chBuf[(n--)-1]='\0';
        sscanf(chBuf,"%s%s",chBuf1,chBuf2);
        K k;
        if (typeid(K)==typeid(string))
        {
            k=(K)chBuf1;
        }
        else if (typeid(K)==typeid(int))
        {
            nNum=atoi(chBuf1);
            k=*(K*)&(nNum);
        }   
        V v;
        if (typeid(V)==typeid(string))
        {
            v=(V)chBuf2;
        }
        else if (typeid(V)==typeid(int))
        {
            nNum=atoi(chBuf2);
            v=*(V*)&(nNum);
        }   
        vec.push_back(make_pair(k,v));

    }
    fclose(fpIn);   
    return true;
}

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

bool CDic_w::LoadDicFromMem(VEC_PR_SD &vpr,bool bReverse)
{	
	char chBuf[1024];
	char *pchBuf;

	vector<KEY_WORD_W> vecStr;
	KEY_WORD_W keyWord;
	WORD *pWord;

	//[[将中英文字全部以双字节存放**************************
	int i=0;
	int nLen;
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
		nLen=strlen(chBuf);
		if (nLen==0||nLen>=MAX_LEN)
		{
			continue;
		}

		while (*pchBuf!=0)//将中英文字全部以双字节存放
		{
			if (*pchBuf<0&&pchBuf[1]!=0&&IsGBKWord(*pchBuf,pchBuf[1])<HALFGBK)//中文字
			{
				*pWord=*(WORD*)pchBuf;
				++pWord;
				pchBuf+=2;
			}
			else//英文或半个中文字
			{
				*pWord=*(BYTE*)pchBuf;
				++pWord;
				++pchBuf;
			}				
		}	
		//]]将中英文字全部以双字节存放**************************
		keyWord.dwWeight=vpr[i].second;
		vecStr.push_back(keyWord);


	}
	if (bReverse)
	{
		for (int i=0;i<vecStr.size();++i)
		{			
			reverse(vecStr[i].word,find(vecStr[i].word,vecStr[i].word+MAX_LEN,0));
		}
	}

	sort(vecStr.begin(),vecStr.end(),KeyWordCmp);

	//cout<<"input complete\n";


	//[[构造字典******************************
	vector<KEY_WORD_W>::iterator itTest;//同一个字的开始
	vector<KEY_WORD_W>::iterator itF;//同一个字的开始
	vector<KEY_WORD_W>::iterator itE;//同一个字的结束
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
	int nlevelCount;//每层个数
	int nSubCnt;//每个节点子节点数
	TREE_NODE_W treeNode;

	m_vvpWordIdx.resize(65536);
	hash_map<int,int> hmSeg;//一棵数各个字的映射；wordCode and level->一个 INT
	int nKey;
	for (it=vecStr.begin();it!=vecStr.end();/*++it*/)
	{					
		itF=it;
		wordIndex=itF->word[0];

		if (m_vvpWordIdx[wordIndex]!=NULL)
			continue;
		m_vvpWordIdx[wordIndex]=new vector<TREE_NODE_W>;

		memset(&treeNode,0,sizeof treeNode);
		treeNode.wData=itF->word[0];//将数据拷贝过去；
		//memcpy(treeNode.chBuf+2,itF->word,2);
		//treeNode.chBuf[4]=0;                 //设置节点所处层	

		m_vvpWordIdx[wordIndex]->push_back(treeNode);

		//确定同一个字开始的段		
		while (it!=vecStr.end()&&(it)->word[0]==wordIndex)//首字相同
		{	
			if (it->word[1]!=0)//后面有第二个字
			{
				vecIter.push_back(it);
			}
			else
			{
				nKey=(it-itF)*MAX_LEN;
				hmSeg.insert(pair<int,int>(nKey,0));//加入位置信息		
				m_vvpWordIdx[wordIndex]->front().bEnd=1;//到第一个字已经成词
				m_vvpWordIdx[wordIndex]->front().dwWeight=it->dwWeight;
			}			
			++it;
		}
		//itE=it;
		nLevel=1;//考察第2层以后
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

				//[[关于父节点*****************************************************************
				preLevelWordCur=(*itVecIter)->word[nLevel-1];
				//nPreLevelPosCur=
				wordCur=(*itVecIter)->word[nLevel];

				nKey=((*itVecIter)-itF)*MAX_LEN+nLevel-1;//父节点在分段矩阵中的一维位置	
				if (preLevelWordCur!=preLevelWordFrst||(preLevelWordCur==preLevelWordFrst&&hmSeg[nKey]!=hmSeg[nPreLevelPosFrst]))//父节点改变
				{			    
					//设置此父节点包含子节点数		
					(*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].wSubCnt=1;
					//*(WORD*)((*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].chBuf+4)=1;
					//设置父节点第一个子节点索引，相对位置。
					(*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].wSubBeg=m_vvpWordIdx[wordIndex]->size()-hmSeg[nKey];
					//*(WORD*)((*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].chBuf+6)=m_vvpWordIdx[wordIndex]->size()-hmSeg[nKey];
					preLevelWordFrst=preLevelWordCur;	
					wordFrst=0;//父节点改变时任何字都当新词
				}
				else
				{						
					if (wordCur!=wordFrst)//同一个父节点下出现新字出现新字
						++(*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].wSubCnt;//设置此父节点包含子节点数			
					//++(*(WORD*)((*m_vvpWordIdx[wordIndex])[hmSeg[nKey]].chBuf+4));		
				}
				//]]关于父节点*****************************************************************


				nKey=((*itVecIter)-itF)*MAX_LEN+nLevel;//当前节点在分段矩阵中的一维位置	
				if (wordCur!=wordFrst)//出现新字
				{
					++nlevelCount;
					wordFrst=wordCur;

					memset(&treeNode,0,sizeof treeNode);
					treeNode.wData=wordCur;//将数据拷贝过去；
					//memcpy(treeNode.chBuf+2,&wordCur,2);


					if ((*itVecIter)->word[nLevel+1]==0)//到本字的路径可以为词
					{
						//treeNode.chBuf[0]=1;
						treeNode.bEnd=1;
						treeNode.dwWeight=(*itVecIter)->dwWeight;
						hmSeg.insert(pair<int,int>(nKey,m_vvpWordIdx[wordIndex]->size()));//加入位置信息
						nPreLevelPosFrst=((*itVecIter)-itF)*MAX_LEN+nLevel-1;//父节点在分段矩阵中的一维位置
						itVecIter=vecIter.erase(itVecIter);//减少遍历长度

					}
					else
					{	
						hmSeg.insert(pair<int,int>(nKey,m_vvpWordIdx[wordIndex]->size()));//加入位置信息
						nPreLevelPosFrst=((*itVecIter)-itF)*MAX_LEN+nLevel-1;//父节点在分段矩阵中的一维位置
						++itVecIter;
					}								
					m_vvpWordIdx[wordIndex]->push_back(treeNode);						
				}
				else
				{
					++itVecIter;
					hmSeg.insert(pair<int,int>(nKey,m_vvpWordIdx[wordIndex]->size()-1));//加入位置信息	
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

	int nValidPos=-1;//匹配列有效位置
	int nRecordPos=-1;//匹配列当前位置
	int nCurPos=0;//当前遍历到的树节点
	DWORD dwWei=INT_MAX;//权重
	char* pMinAdvancePos;//最小的前到位置
	char* pBackCur;//备份每一个新词的起始节点。
	//  int nNextPos=0;//下一个树节点索引
	// int nNextCnt=0;//下面子节点总个数。
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
		if (pV!=NULL)//有这个字
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur已经前进了最小单位
			if (pTN[0].bEnd==1)//首字是词不返回
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//下一层开始位置
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//下一层要用的数量。

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//中止了
				{
					if (nValidPos>-1)//有有效的
					{
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);	
						// 						if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
						// 							||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
						// 						{
						// 							break;
						// 						}
						PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

					}
					else
					{

						vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
						pCur=pMinAdvancePos;//推进到最小推进位置
					}
					break;
				}
				else//取得了一个词
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//到此字可以为词
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//有有效的
						{
							PointBack(pCur,recordWord,nRecordPos,nValidPos);	
							// 							if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
							// 								||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
							// 							{
							// 								break;
							// 							}
							PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

						}
						else
						{
							vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
							pCur=pMinAdvancePos;//推进到最小推进位置
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//有这个字
		else
		{
			vecStr.push_back(make_pair(string(pBackCur,pCur),INT_MAX));
		}
	}// while(pCur!=pchEnd)

}


//加入了数字英文截断验证 如果词典里面没有对于的分词，则分词权重为INT_MAX
void CDic_w::SegmentEx(char* pchSrc, char*pchEnd,VEC_PR_SD &vecStr)
{

	// WORD word;
	char* pCur=pchSrc;
	// hash_map<int,vector<TREE_NODE>* >::iterator it;
	WORD recordWord[MAX_LEN];

	int nValidPos=-1;//匹配列有效位置
	int nRecordPos=-1;//匹配列当前位置
	int nCurPos=0;//当前遍历到的树节点
	DWORD dwWei=INT_MAX;//权重
	char* pMinAdvancePos;//最小的前到位置
	char* pBackCur;//备份每一个新词的起始节点。
	//  int nNextPos=0;//下一个树节点索引
	// int nNextCnt=0;//下面子节点总个数。
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
		if (pV!=NULL)//有这个字
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur已经前进了最小单位
			if (pTN[0].bEnd==1)//首字是词不返回
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//下一层开始位置
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//下一层要用的数量。

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//中止了
				{
					if (nValidPos>-1)//有有效的
					{
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);	
						if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
							||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
						{
							vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
							pCur=pMinAdvancePos;//推进到最小推进位置
							break;
						}
						PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

					}
					else
					{

						vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
						pCur=pMinAdvancePos;//推进到最小推进位置
					}
					break;
				}
				else//取得了一个词
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//到此字可以为词
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//有有效的
						{
							PointBack(pCur,recordWord,nRecordPos,nValidPos);	
							if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
								||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
							{
								vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
								pCur=pMinAdvancePos;//推进到最小推进位置

								break;
							}
							PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

						}
						else
						{
							vecStr.push_back(make_pair(string(pBackCur,pMinAdvancePos),INT_MAX));
							pCur=pMinAdvancePos;//推进到最小推进位置
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//有这个字
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

	int nValidPos=-1;//匹配列有效位置
	int nRecordPos=-1;//匹配列当前位置
	int nCurPos=0;//当前遍历到的树节点
	DWORD dwWei=0;//权重
	char* pMinAdvancePos;//最小的前到位置
	char* pBackCur;//备份每一个新词的起始节点。
	//  int nNextPos=0;//下一个树节点索引
	// int nNextCnt=0;//下面子节点总个数。
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
		if (pV!=NULL)//有这个字
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur已经前进了最小单位
			if (pTN[0].bEnd==1)//首字是词不返回
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//下一层开始位置
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//下一层要用的数量。

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//中止了
				{
					if (nValidPos>-1)//有有效的
					{
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);	
						if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
							||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
						{
							break;
						}
						PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

					}
					else
					{
						pCur=pMinAdvancePos;//推进到最小推进位置
					}
					break;
				}
				else//取得了一个词
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//到此字可以为词
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//有有效的
						{
							PointBack(pCur,recordWord,nRecordPos,nValidPos);	
							if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
								||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
							{
								break;
							}
							PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);

						}
						else
						{
							pCur=pMinAdvancePos;//推进到最小推进位置
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//有这个字
		else
		{

		}
	}// while(pCur!=pchEnd)

}




void CDic_w::SearchKeyWordInDicEx( char* pchSrc, char*pchEnd, VEC_PR_II &vecPII)
{

	// WORD word;
	char* pCur=pchSrc;
	// hash_map<int,vector<TREE_NODE>* >::iterator it;
	WORD recordWord[MAX_LEN];

	int nValidPos=-1;//匹配列有效位置
	int nRecordPos=-1;//匹配列当前位置
	int nCurPos=0;//当前遍历到的树节点
	DWORD dwWei=0;//权重
	char* pMinAdvancePos;//最小的前到位置
	char* pBackCur;//备份每一个新词的起始节点。
	//  int nNextPos=0;//下一个树节点索引
	// int nNextCnt=0;//下面子节点总个数。
	//   set<string> setStr;
	vecPII.clear();
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
		if (pV!=NULL)//有这个字
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur已经前进了最小单位
			if (pTN[0].bEnd==1)//首字是词不返回
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//下一层开始位置
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//下一层要用的数量。

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//中止了
				{
					if (nValidPos>-1)//有有效的
					{
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);	
						if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
							||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
						{
							break;
						}
						vecPII.push_back(make_pair(pBackCur-pchSrc,pCur-pchSrc));
					}
					else
					{
						pCur=pMinAdvancePos;//推进到最小推进位置
					}
					break;
				}
				else//取得了一个词
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//到此字可以为词
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//有有效的
						{
							PointBack(pCur,recordWord,nRecordPos,nValidPos);	
							if (*pBackCur>0&&isalnum(*pBackCur)&&(pBackCur>pchSrc)&&*(pBackCur-1)>0&&isalnum(*(pBackCur-1))//前端有相连数字英文
								||*pCur>0&&*(pCur-1)>0&&isalnum(*pCur)&&isalnum(*(pCur-1)))//后端有相连数字英文
							{
								break;
							}
							vecPII.push_back(make_pair(pBackCur-pchSrc,pCur-pchSrc));

						}
						else
						{
							pCur=pMinAdvancePos;//推进到最小推进位置
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//有这个字
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

	int nValidPos=-1;//匹配列有效位置
	int nRecordPos=-1;//匹配列当前位置
	int nCurPos=0;//当前遍历到的树节点
	DWORD dwWei=0;//权重
	char* pMinAdvancePos;//最小的前到位置
	//  int nNextPos=0;//下一个树节点索引
	// int nNextCnt=0;//下面子节点总个数。
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
		if (pV!=NULL)//有这个字
		{
			pTN=&pV->front();
			nRecordPos=0;//the first word		  
			nCurPos=0;
			nValidPos=-1;
			pMinAdvancePos=pCur;//pCur已经前进了最小单位
			if (pTN[0].bEnd==1)//首字是词
			{
				dwWei=pTN[0].dwWeight;
				nValidPos=nRecordPos;
			}
			//nNextPos+=*(WORD*)((*(it->second))[0].chBuf+6);//下一层开始位置
			//nNextCnt=*(WORD*)((*(it->second))[0].chBuf+4);//下一层要用的数量。

			while (true)
			{

				GetOneWord(pCur,pchEnd,recordWord[++nRecordPos]);
				if(recordWord[nRecordPos]==0)//中止了
				{
					if (nValidPos>-1)//有有效的
					{
						PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);
						//PointBack(pCur,recordWord,nRecordPos,nValidPos);						 
					}
					else
					{
						pCur=pMinAdvancePos;//推进到最小推进位置
					}
					break;

				}
				else//取得了一个词
				{
					if(FindWord(recordWord[nRecordPos],nCurPos,pTN))
					{					   
						if (pTN[nCurPos].bEnd==1)//到此字可以为词
						{
							nValidPos=nRecordPos;
							dwWei=pTN[nCurPos].dwWeight;
						}

					}
					else
					{

						if (nValidPos>-1)//有有效的
						{
							PushOneKeyWord(recordWord,nValidPos,dwWei,vecStr);
							PointBack(pCur,recordWord,nRecordPos,nValidPos);						 
						}
						else
						{
							pCur=pMinAdvancePos;//推进到最小推进位置
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//有这个字
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
	if (*pCur<0&&pCur+1!=pchEnd/*&&IsGBKWord(*pCur,pCur[1])<HALFGBK*/)//一个中文字
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

	if(pItEnd-pItStart>20)//采用2分搜索
	{
		TREE_NODE_W treeNode;
		treeNode.wData=word;

		pIt=lower_bound(pItStart,pItEnd,treeNode,FOBJ_CMP_NODE());
		if (pIt!=pItEnd&&word==pIt->wData)//找到了
		{
			nCurPos=pIt-pTN;//找到了改变 nCurPos
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
		if (pIt!=pItEnd)//找到了
		{
			nCurPos=pIt-pTN;//找到了改变 nCurPos
			return true;
		}

	}
	return false;

}

void CDic_w::PointBack(char*&pCur,WORD *pWord,int nRecordPos,int nValidPos)
{

	while (nRecordPos!=nValidPos)
	{
		if ((pWord[nRecordPos]&0xFF00)==0)//倒退单字节
		{
			--pCur;
		}
		else//倒退双字节
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
	vector<pair<int,int> > vecPii;//堆栈记录本层有几个子几点，当前遍历了几个节点。
	pV =m_vvpWordIdx[*recordWord];
	if (pV==NULL) return;

	pTN=&pV->front();
	nCurPos=0;
	for (int i=1;i<nPreFixCnt;++i)
	{
		if(!FindWord(recordWord[i],nCurPos,pTN))
			return;
	}
	vecPii.push_back(make_pair(nCurPos,0));//prefix 最后一个节点的位置，该节点的字节点的访问个数

	while (!vecPii.empty())
	{

		while (pTN[nCurPos].wSubCnt)//还有子节点向更深层遍历
		{
			nCurPos=nCurPos+pTN[nCurPos].wSubBeg+vecPii.back().second;//下一个节点位置；
			++vecPii.back().second;                                   //子节点访问数量加1；
			vecPii.push_back(make_pair(nCurPos,0));                   //最后一个节点的位置,该节点的字节点的访问个数
			recordWord[nPreFixCnt++]=pTN[nCurPos].wData;              //加入一个节点的数据；
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
inline bool CMP_pair(const pair<WORD,WORD>&l,const pair<WORD,WORD>&r)
{
	return l.second>r.second;
}
void CDic_w::Build2Array()
{
	//－－－－映射文字到序号。
	VI viEndTag;
	m_vwMapWord.resize(65536);
	int i;
	VEC_PR_WW vpww(65536);
	for ( i=0;i<65536;++i)
	{
		if(m_vvpWordIdx[i])
		{
			vpww[i].first=i;
			vpww[i].second=m_vvpWordIdx[i]->front().wSubCnt;
		}
	}
	sort(vpww.begin(),vpww.end(),CMP_pair);

	for ( i=0;i<65536&&vpww[i].second;++i)
	{		
		m_vwMapWord[vpww[i].first]=i+1;//形成映射序号 每个字映射成他的排序后的序号。从1开始
	}

	m_nMaxBegWordNum=i;
	m_nMaxNum=i;

	vpww.erase(vpww.begin()+i,vpww.end());//去掉没出现的首字。


	int nBaseOffset=vpww.size()+1;//

	//生成双数组TRIE树。
	m_viBase.resize(1024*1024);//初始化填入 00
	m_viCheck.resize(1024*1024);//初始化填入 00


	priority_queue<PosCnt,vector<PosCnt> >  pq;

	vector<TREE_NODE_W>* pv;
	TREE_NODE_W*pNodeFrst;
	int nMaxState=0;
	int nSquence;
	int nFrtWordCode;
	PosCnt pc;
	PosCnt pcSub;
	int j;
	int nBaseVal;
	bool bCheck;

	WORD wBeg;
	WORD wEnd;

	int t;
	int s;
	cout<<"first word count"<<vpww.size()<<endl;
	for (i=0;i<vpww.size();++i)//对每一个起始的字处理按照从小到大的顺序。
	{
		if (i%100==0)
		{
			cout<<"readline:"<<i<<endl;
		}

		nFrtWordCode=vpww[i].first;
		nSquence=m_vwMapWord[nFrtWordCode];
		pv=m_vvpWordIdx[nFrtWordCode];
		pNodeFrst=&(pv->front());	
		// 		char buf[1024]={0};
		// 		strncpy(buf,(char*)&nFrtWordCode,2);
		// 		if (pNodeFrst[0].wData==55788)
		// 		{
		// 			int a=0;
		// 		}


		if(pNodeFrst->wSubCnt==0)//只能单独成词
		{
			m_viBase[nSquence]=-nSquence;
			m_viCheck[nSquence]=0;
		}
		else
		{
			m_viCheck[nSquence]=0;
			pc.nCnt=pNodeFrst->wSubCnt;
			pc.nPosInTrie=0;
			pc.nPosInBase=nSquence;

			pq.push(pc);


			while (!pq.empty())
			{
				pc=pq.top();
				pq.pop();
				wBeg=pNodeFrst[pc.nPosInTrie].wSubBeg+pc.nPosInTrie;
				wEnd=wBeg+pNodeFrst[pc.nPosInTrie].wSubCnt;

				if (pc.nPosInBase>nMaxState)
				{
					nMaxState=pc.nPosInBase;
				}
				//检查空间 延长数组
				if (pc.nCnt>0)
				{
					if (pNodeFrst[pc.nPosInTrie].bEnd)
					{
						viEndTag.push_back(pc.nPosInBase);
					}
					nBaseVal=pc.nPosInBase;

					while (true)
					{
						bCheck=true;
						for (j=wBeg;j<wEnd;++j)
						{
							t=nBaseVal+MapOneWordToSequence(pNodeFrst[j].wData);

							if(t<nBaseOffset)//首次便宜量过小。
							{
								//t=nBaseOffset;
								//++nBaseOffset;
								nBaseVal=nBaseOffset-MapOneWordToSequence(pNodeFrst[j].wData);								
								bCheck=false;
								break;	
							}
							if (t>=m_viBase.size())
							{
								m_viBase.resize(t*2);
								m_viCheck.resize(t*2);

							}

							if (m_viCheck[t]!=0||m_viBase[t]!=0)
							{
								bCheck=false;
								++nBaseVal;															
								break;								
							}

						}


						if (bCheck)//找到位置了。
						{
							if ((nBaseVal==pc.nPosInBase)&&//便宜值等于数组下标
								(pc.nCnt>0&&pNodeFrst[pc.nPosInTrie].bEnd)) //有子节点但是可以成词
							{
								++nBaseVal;	
								continue;
							}
							m_viBase[pc.nPosInBase]=nBaseVal;//父节点BASE
							for (j=wBeg;j<wEnd;++j)//子节点CHECK；
							{
								t=nBaseVal+MapOneWordToSequence(pNodeFrst[j].wData);
								m_viCheck[t]=pc.nPosInBase;

								pcSub.nCnt=pNodeFrst[j].wSubCnt;
								pcSub.nPosInTrie=j;
								pcSub.nPosInBase=t;
								pq.push(pcSub);
							}	
							break;
						}
					}

				}
				else//结束了，BASE??
				{
					m_viBase[pc.nPosInBase]=-pc.nPosInBase;

				}		

			}

		}

	}


	for (i=0;i<viEndTag.size();++i)
	{
		m_viBase[viEndTag[i]]*=-1;
	}
	m_viBase.resize(nMaxState+65536);
	m_viCheck.resize(nMaxState+65536);
//	m_viBase.swap(VI(m_viBase));
//	m_viCheck.swap(VI(m_viCheck));
#ifdef TEST_
	int nTotalState=0;
	for (i=0;i<65536;++i)
	{
		if (m_vvpWordIdx[i])
		{
			nTotalState+=m_vvpWordIdx[i]->size();
		}

	}
	cout<<"TWO-ARRAY-STATE:"<<nTotalState<<endl;
	cout<<"use space "<<m_viBase.size()*sizeof(int)*2<<endl;
	cout<<"words count "<<viEndTag.size()<<endl;
#endif


}

int CDic_w::MapOneWordToSequence(WORD w)
{
	if (!m_vwMapWord[w])
	{
		m_vwMapWord[w]=++m_nMaxNum;
		return m_nMaxNum;
	}
	else
	{
		return m_vwMapWord[w];
	}

}

//只有BUILD成正向的才可以使用
void CDic_w::SegmentExBy2ArrayFMM(char* pchSrc, char*pchEnd,VS &vecStr)
{

	char *pBeg;
	char *pVal;
	char *pCur;
	pBeg=pchSrc;
	pCur=pchSrc;
	pVal=pCur;
	WORD w;
	int  sbak=0;
	int  sCur;
	int  nBaseV=0;

#define  GET_ONE_WORD if (*pCur<0&&*(pCur+1)) { w=*((WORD*)pCur); pCur+=2; }  else{	w=*(BYTE*)pCur;	++pCur;  } 

	/* #define  IS_VALID_BEG_WORD(wordId) (wordId=m_vwMapWord[w],wordId&&wordId<m_nMaxBegWordNum)*/
	//   #define  IS_VALID_WORD(wordId) (wordId=m_vwMapWord[w],wordId&&wordId<m_nMaxNum)

	while	(true)
	{
		while (pCur!=pchEnd)
		{
			GET_ONE_WORD;
			if (pVal==pBeg)//最小的PVAL
			{
				pVal=pCur;
			}
			// 
			// 	   if (w==0)
			// 	   {
			// 		   vecStr.push_back(string(pBeg,pVal));
			// 		   pCur=pVal;
			// 		   pBeg=pCur;	
			// 		   sbak=0;//备份状态置0	
			// 	   }
			// 	   else
			// 	   {
			if (sCur=m_vwMapWord[w],sCur&&sCur<=m_nMaxNum) //有效的词
			{		
				nBaseV=m_viBase[sbak];
				if (nBaseV<0)
				{
					sCur=-nBaseV+sCur;
				}
				else
				{
					sCur=nBaseV+sCur;
				}

				if (m_viCheck[sCur]==sbak)//转移成功
				{					   
					if (m_viBase[sCur]<0)  //还有可能延续 不一定是最大匹配
					{ 
						pVal=pCur;
						if ( m_viBase[sCur]==-sCur)//中止状态
						{
							vecStr.push_back(string(pBeg,pVal));							  
							pBeg=pCur;
							sbak=0;//备份状态置0		
							continue;
						}
					}
					sbak=sCur;
				}
				else //转移失败
				{
					vecStr.push_back(string(pBeg,pVal));	
					pCur=pVal;
					pBeg=pCur;
					sbak=0;//备份状态置0	
				}
			}
			else//无效的词
			{
				vecStr.push_back(string(pBeg,pVal));	
				pCur=pVal;
				pBeg=pCur;
				sbak=0;//备份状态置0	
			}

			/* }*/
		}

		if (pBeg!=pchEnd)
		{
			vecStr.push_back(string(pBeg,pVal));
			if (pVal!=pchEnd)//剩余字节处理还没处理完毕.
			{
				pCur=pVal;
				pBeg=pCur;
				sbak=0;//备份状态置0	
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
}





//只有BUILD成反向的才可以使用
void CDic_w::SegmentExBy2ArrayBMM(char* pchSrc, char*pchEnd,VS &vecStr)
{

	if (pchEnd==pchSrc)
	{
		return;
	}
	char *pBeg;
	char *pVal;
	char *pCur;
	pBeg=pchEnd;
	pCur=pchEnd;
	pVal=pBeg;
	WORD w;
	int  sbak=0;
	int  sCur;
	int  nBaseV=0;
#define  GET_ONE_WORD_R  {--pCur; if (*pCur<0&&(pCur!=pchSrc)) {--pCur; w=*(WORD*)pCur;  }  else{	w=*(BYTE*)pCur;	 } }

	while (true)
	{

		do 
		{
			GET_ONE_WORD_R;
			if (pVal==pBeg)//最小的PVAL
			{
				pVal=pCur;
			}

			if (sCur=m_vwMapWord[w],sCur&&sCur<=m_nMaxNum) //有效的词
			{		
				nBaseV=m_viBase[sbak];
				if (nBaseV<0)
				{
					sCur=-nBaseV+sCur;
				}
				else
				{
					sCur=nBaseV+sCur;
				}

				if (m_viCheck[sCur]==sbak)//转移成功
				{					   
					if (m_viBase[sCur]<0)  //还有可能延续 不一定是最大匹配
					{ 
						pVal=pCur;
						if ( m_viBase[sCur]==-sCur)//中止状态
						{
							vecStr.push_back(string(pVal,pBeg));							  
							pBeg=pCur;
							sbak=0;//备份状态置0		
							continue;
						}
					}
					sbak=sCur;
				}
				else //转移失败
				{
					vecStr.push_back(string(pVal,pBeg));		
					pCur=pVal;
					pBeg=pCur;
					sbak=0;//备份状态置0	
				}
			}
			else//无效的词
			{
				vecStr.push_back(string(pVal,pBeg));	
				pCur=pVal;
				pBeg=pCur;
				sbak=0;//备份状态置0	
			}


		}while (pCur!=pchSrc);

		if (pBeg!=pchSrc)//还有剩余的字节没有处理
		{
			vecStr.push_back(string(pVal,pBeg));
			if (pVal!=pchSrc)//剩余字节处理还没处理完毕.
			{
				pCur=pVal;
				pBeg=pCur;
				sbak=0;//备份状态置0	
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}

}

bool CDic_w::InitFromFile(const char *pchFile,bool bReverse)
{

	typedef pair<string,string> PSS;
	typedef vector<PSS > VEC_PR_SS;
	VEC_PR_SS vpss;

	if (!Load2PairFile_V(pchFile,vpss))
	{
		return false;
	}	
	sort(vpss.begin(),vpss.end(),FOBJ_LessBy1Mem<PSS,S>(offsetof(PSS,first)));
	vpss.erase(unique(vpss.begin(),vpss.end(),FOBJ_EqualBy1Mem<PSS,S>(offsetof(PSS,first))),vpss.end());
	DWORD i;
	VEC_PR_SD vpsd;
	FOR_EACH_CLASSIC(i,0,vpss.size())
	{
		vpsd.push_back(make_pair(vpss[i].first,i));		
	}

	LoadDicFromMem(vpsd,bReverse);

	Build2Array();
	return true;


}
