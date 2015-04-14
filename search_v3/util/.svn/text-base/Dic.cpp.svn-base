//#include "stdafx.h"
#include "Dic.h"

CDic::CDic(void)
{
}

CDic::~CDic(void)
{
	ClearData();
}

inline bool KeyWordCmp(const KEY_WORD& l,const KEY_WORD& r)
{
	return lexicographical_compare(l.word,l.word+MAX_LEN,r.word,r.word+MAX_LEN);

}


inline bool TestKeyCmp(const TEST_KEY& l,const TEST_KEY& r)
{
  return strcmp(l.chBuf,r.chBuf)<0;
}

bool CDic::LoadDicFromMem(vector<string> &vs)
{	
	char chBuf[1024];
	char *pchBuf;

	vector<KEY_WORD> vecStr;
	KEY_WORD keyWord;
	WORD *pWord;

	//[[将中英文字全部以双字节存放**************************
	
	int i=0;
for(i=0;i<vs.size();++i)
	{  
		
		strcpy(chBuf,vs[i].c_str());		
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
		vecStr.push_back(keyWord);
	

	}

	sort(vecStr.begin(),vecStr.end(),KeyWordCmp);
	//cout<<"input complete\n";


	//[[构造字典******************************
	vector<KEY_WORD>::iterator itTest;//同一个字的开始
	vector<KEY_WORD>::iterator itF;//同一个字的开始
	vector<KEY_WORD>::iterator itE;//同一个字的结束
	vector<KEY_WORD>::iterator it;
	vector<vector<KEY_WORD>::iterator> vecIter; 
	vector<vector<KEY_WORD>::iterator>::iterator itVecIter;
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
	TREE_NODE treeNode;

	m_vvpWordIdx.resize(65536);
	hash_map<int,int> hmSeg;//一棵数各个字的映射；wordCode and level->一个 INT
	int nKey;
	for (it=vecStr.begin();it!=vecStr.end();/*++it*/)
	{					
		itF=it;
		wordIndex=itF->word[0];

		if (m_vvpWordIdx[wordIndex]!=NULL)
			continue;
		m_vvpWordIdx[wordIndex]=new vector<TREE_NODE>;

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


bool CDic::LoadDic(const char*pchDicFile)
{
	ifstream in;
	in.open( pchDicFile, ios_base::in | ios_base::binary );
	if(!in)
	{			
		return false;
	}
	char chBuf[1024];
	char *pchBuf;

	vector<KEY_WORD> vecStr;
	KEY_WORD keyWord;
	WORD *pWord;

	//[[将中英文字全部以双字节存放**************************
	while (in.getline(chBuf,1024))
	{  
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
		vecStr.push_back(keyWord);
		
	}

	sort(vecStr.begin(),vecStr.end(),KeyWordCmp);
	//cout<<"input complete\n";


	//[[构造字典******************************
	vector<KEY_WORD>::iterator itTest;//同一个字的开始
	vector<KEY_WORD>::iterator itF;//同一个字的开始
	vector<KEY_WORD>::iterator itE;//同一个字的结束
	vector<KEY_WORD>::iterator it;
	vector<vector<KEY_WORD>::iterator> vecIter; 
	vector<vector<KEY_WORD>::iterator>::iterator itVecIter;
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
	TREE_NODE treeNode;
	
	m_vvpWordIdx.resize(65536);
	hash_map<int,int> hmSeg;//一棵数各个字的映射；wordCode and level->一个 INT
	int nKey;
	for (it=vecStr.begin();it!=vecStr.end();/*++it*/)
	{					
		itF=it;
		wordIndex=itF->word[0];

		if (m_vvpWordIdx[wordIndex]!=NULL)
			continue;
		m_vvpWordIdx[wordIndex]=new vector<TREE_NODE>;

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

	in.close();
	return true;
	
}


void CDic::SearchKeyWordInDicEx( char* pchSrc, char*pchEnd,vector<string> &vecStr)
{

	// WORD word;
	char* pCur=pchSrc;
	// hash_map<int,vector<TREE_NODE>* >::iterator it;
	WORD recordWord[MAX_LEN];

	int nValidPos=-1;//匹配列有效位置
	int nRecordPos=-1;//匹配列当前位置
	int nCurPos=0;//当前遍历到的树节点
	char* pMinAdvancePos;//最小的前到位置
	char* pBackCur;//备份每一个新词的起始节点。
	//  int nNextPos=0;//下一个树节点索引
	// int nNextCnt=0;//下面子节点总个数。
	//   set<string> setStr;
	vecStr.clear();
	vector<TREE_NODE>*pV;
	TREE_NODE*pTN;

	while(pCur!=pchEnd)
	{	  
		pBackCur=pCur;
		memset(recordWord,0,MAX_LEN*2);
		GetOneWord(pCur,pchEnd,*recordWord);
// 		if(*recordWord==0)
// 			break;	   

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
						PushOneKeyWord(recordWord,nValidPos,vecStr);

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
							PushOneKeyWord(recordWord,nValidPos,vecStr);
													 
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


void CDic::SearchKeyWordInDic( char* pchSrc, char*pchEnd,vector<string> &vecStr)
{
  
  // WORD word;
   char* pCur=pchSrc;
  // hash_map<int,vector<TREE_NODE>* >::iterator it;
   WORD recordWord[MAX_LEN];

   int nValidPos=-1;//匹配列有效位置
   int nRecordPos=-1;//匹配列当前位置
   int nCurPos=0;//当前遍历到的树节点
   char* pMinAdvancePos;//最小的前到位置
 //  int nNextPos=0;//下一个树节点索引
  // int nNextCnt=0;//下面子节点总个数。
//   set<string> setStr;
   vecStr.clear();
   vector<TREE_NODE>*pV;
   TREE_NODE*pTN;
   
   while(pCur!=pchEnd)
   {	  
	   memset(recordWord,0,MAX_LEN*2);
	   GetOneWord(pCur,pchEnd,*recordWord);
// 	   if(*recordWord==0)
// 	   {
// 		   cout<<"\n0 end;";
// 		   break;	   
// 	   }
// 	 
	   
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
					   PushOneKeyWord(recordWord,nValidPos,vecStr);
					  // PointBack(pCur,recordWord,nRecordPos,nValidPos);						 
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
					   }

				   }
				   else
				   {

					   if (nValidPos>-1)//有有效的
					   {
						   PushOneKeyWord(recordWord,nValidPos,vecStr);
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
 //  cout<<"pend-pcur "<<pchEnd-pCur;

}

void CDic::GetOneWord(char* &pCur,const char*pchEnd,WORD&word)
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



void CDic::PushOneKeyWord(WORD *pWord,int nValPos, vector<string>&vecStr)
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

// 	static int i=0;
// 	static FILE *FP;
// 	if (i==0)
// 		FP=fopen("Res.txt","w");
//  	++i;
//  	if (i%1000==0)
//  	{
// 		printf("\n read line %d",i);
//  	}
	vecStr.push_back(chBuf);
// 	if (vecStr.size()>10)
// 	{
// 		for (int i=0;i<vecStr.size();++i)
// 		{
// 			fprintf(FP,"%s\n",vecStr[i].c_str());
// 		}
// 		vecStr.clear();
// 		
// 	}

 	
	

}


struct FOBJ_CMP_NODE 
{
	inline bool operator()(const TREE_NODE&l,const TREE_NODE &r)
	{
		return  l.wData<r.wData;

	}
};


bool CDic::FindWord(WORD word,int &nCurPos,TREE_NODE* pTN)
{
	TREE_NODE*    pIt;
	TREE_NODE*    pItStart=pTN[nCurPos].wSubBeg+nCurPos+pTN;
	TREE_NODE*    pItEnd=pItStart+pTN[nCurPos].wSubCnt;
	
	if(pItEnd-pItStart>20)//采用2分搜索
	{
		TREE_NODE treeNode;
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

void CDic::PointBack(char*&pCur,WORD *pWord,int nRecordPos,int nValidPos)
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
int CDic::IsGBKWord(unsigned char ch1,unsigned char ch2)
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

void CDic::ClearData()
{
	vector<vector<TREE_NODE>* > ::iterator it;
	for (it=m_vvpWordIdx.begin();it!=m_vvpWordIdx.end();++it)
	{
		delete *it;
	}
	m_vvpWordIdx.clear();
}

void CDic::SearchBeginWith( char * pchSrc, char* pchEnd,vector<string> &vecStr)
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

	vector<TREE_NODE>*pV;
	TREE_NODE*pTN;
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
				PushOneKeyWord(recordWord,nPreFixCnt-1,vecStr);
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

void CDic::SegmentEx(char* pchSrc, char*pchEnd,vector<string> &vecStr)
{

	
	// WORD word;
	char* pCur=pchSrc;
	// hash_map<int,vector<TREE_NODE>* >::iterator it;
	WORD recordWord[MAX_LEN];

	int nValidPos=-1;//匹配列有效位置
	int nRecordPos=-1;//匹配列当前位置
	int nCurPos=0;//当前遍历到的树节点
	//DWORD dwWei=INT_MAX;//权重
	char* pMinAdvancePos;//最小的前到位置
	char* pBackCur;//备份每一个新词的起始节点。
	//  int nNextPos=0;//下一个树节点索引
	// int nNextCnt=0;//下面子节点总个数。
	//   set<string> setStr;
	vecStr.clear();
	vector<TREE_NODE>*pV;
	TREE_NODE*pTN;

	while(pCur!=pchEnd)
	{	
		//dwWei=INT_MAX;
		pBackCur=pCur;
		memset(recordWord,0,MAX_LEN*2);
		GetOneWord(pCur,pchEnd,*recordWord);
// 		if(*recordWord==0)
// 			break;	   

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
				//dwWei=pTN[0].dwWeight;
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
							vecStr.push_back(string(pBackCur,pMinAdvancePos));
							pCur=pMinAdvancePos;//推进到最小推进位置
							break;
						}
						PushOneKeyWord(recordWord,nValidPos,vecStr);

					}
					else
					{

						vecStr.push_back(string(pBackCur,pMinAdvancePos));
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
							//dwWei=pTN[nCurPos].dwWeight;
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
								vecStr.push_back(string(pBackCur,pMinAdvancePos));
								pCur=pMinAdvancePos;//推进到最小推进位置

								break;
							}
							PushOneKeyWord(recordWord,nValidPos,vecStr);

						}
						else
						{
							vecStr.push_back(string(pBackCur,pMinAdvancePos));
							pCur=pMinAdvancePos;//推进到最小推进位置
						}
						break;

					}
				}
			}// while (true)

		}//if (it!=m_hm1WordIdx.end())//有这个字
		else
		{
			vecStr.push_back(string(pBackCur,pCur));
		}
	}// while(pCur!=pchEnd)
	cout<<"pend-pcur"<<pchEnd-pCur;
	cout<<"vecStr.size()"<<vecStr.size()<<endl;


	
		static FILE *FP;

	
		FP=fopen("Res.txt","w");
 	
 	

		
	
		for (int i=0;i<vecStr.size();++i)
		{
			fprintf(FP,"%s\n",vecStr[i].c_str());
		}
	
		
	

}