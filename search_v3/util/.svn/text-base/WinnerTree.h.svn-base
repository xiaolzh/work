#ifndef WINNERTREE_H
#define WINNERTREE_H
#include <vector>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#ifndef _WIN32
#include <unistd.h>
#define O_BINARY 0
#else
#include <io.h>
#define O_LARGEFILE 0
#endif


#define  INFINITE_DISTANCE 65536

using namespace std;

template<class INVERT>
inline int DEFULT_MINUS_INVERT(INVERT&l,INVERT&r)
{
	if (l.nDocId!=r.nDocId)
	{
		return l.nDocId-r.nDocId>0?INFINITE_DISTANCE:-INFINITE_DISTANCE;
	}	

	return l.nOffset-r.nOffset;
}

template<class INVERT>
inline INVERT& DEFAULT_INVERT_MINUS_INT(INVERT& l,int nMinus)
{
	l.nOffset-=nMinus;
	return l;
}

template<class TYPE>
inline	bool CMP_LESS_DEFAULT(const TYPE& l,const TYPE& r)
{
	return l<r;
}




//利用败者树实现K路文件归并排序算法
//************************************
// Method:    K_MergeFile
// FullName:  <NODE>::K_MergeFile
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: const char *pchFileIn 输入文件名字
// Parameter: const char *pchFileOut输出文件名字

// Parameter: *CMP_NODE             比较函数
// Parameter: int nFreeMemSize      设定归并使用连续内存空间的大小
//************************************
template <class NODE>
int  K_MergeFile(const char *pchFileIn,            const char *pchFileOut,
				 bool(*CMP_NODE)(const NODE& L, const NODE& R), int nFreeMemSize);



//利用败者树实现K路文件归并排序算法,对K个有序文件进行归并
//************************************
// Method:    K_MergeFile
// FullName:  <NODE>::K_MergeFile
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: vector<char*> &vecFileNames 要归并的输入文件名字列表
// Parameter: const char *pchFileOut输出文件名字

// Parameter: *CMP_NODE             比较函数
// Parameter: int nFreeMemSize      设定归并使用连续内存空间的大小
//************************************
template <class NODE>
int  K_MergeFile(vector<string> &vecFileNames,   const char *pchFileOut,
				 bool(*CMP_NODE)(const NODE& L, const NODE& R), int nCacheNodeCnt);




//高效K路序列求交集
//************************************
// Method:    K_Intersect
// FullName:  <NODE>::K_Intersect
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: vector<NODE*>&vecNodePtr    各列首地址数组
// Parameter: vector<NODE*>&vecNodeEndPtr 各列结束地址数组。
// Parameter: vector<NODE> *pVecDest      指向归并后存放的向量，将调用vector的push_back方法。

// Parameter: *CMP_NODE                   比较函数返回小于

//************************************
template <class NODE>
NODE*  K_Intersect(vector<NODE*>&vecNodePtr,
				  vector<NODE*>&vecNodeEndPtr,
				  NODE *pNodeDest,
				  bool(*CMP_NODE)(const NODE& L, const NODE& R),
				  void(*ADD_NODE)(NODE& L, NODE& R));




//高效K路倒排表序列求交集 返回倒排表序列，每个节点的偏移是首字在文档中的偏移位置

//************************************
// Method:    K_InvertIntersect
// FullName:  <NODE>::K_InvertIntersect
// Access:    public 
// Returns:   void
// Qualifier:
// Parameter: vector<NODE*>&vecNodePtr    各列首地址数组
// Parameter: vector<NODE*>&vecNodeEndPtr 各列结束地址数组。
// Parameter: vector<NODE> *pVecDest      指向归并后存放的向量，全部返回首列的元素。故每个元素的偏移地址都是所查串的第一个字。

// Parameter: *MINUS_INVERT               两个倒排做减法的比较函数，

// Parameter: *INVERT_MINUS_INT           倒排元素减去整数，目的是为了找到文档中前面元素的倒排节点。

//************************************
template <class NODE>
NODE*  K_InvertIntersect(vector<NODE*>&vecNodePtr,
				  vector<NODE*>&vecNodeEndPtr,
				  NODE*        pNodeDest,
				  int (*MINUS_INVERT)(NODE&l,NODE&r),
				  NODE&(*INVERT_MINUS_INT)(NODE&l,int nMinus));







//败者树－可以用它来实现内存内K路归并
template <class NODE>
class CWinnerTree
{
public:

	//CMP_ELEMENT指向比较函数指针 若为空调用OPERATOR <
	CWinnerTree(bool(*CMP_ELEMENT)(const NODE& L, const NODE& R))
	{	
		M_CMP_ELEMENT=CMP_ELEMENT;	
		m_pNodeDest=NULL;
		m_pNodePrDest=NULL;
	}


	//构造一颗完全二叉树，并初始化第一排节点。
	//************************************
	// Method:    MakeTree
	// FullName:  CWinnerTree<NODE>::MakeTree
	// Access:    public 
	// Returns:   void
	// Qualifier:
	// Parameter: vector<NODE*>&vecNodePtr    各列首地址数组
	// Parameter: vector<NODE*>&vecNodeEndPtr 各列结束地址数组。
	// Parameter: vector<NODE> *pVecDest      指向归并后存放的向量，将调用vector的push_back方法。
	//************************************
	//归并结果中不带列序号 ，和 K_Merge(vector<NODE>* )成对使用
	bool CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,NODE *pNodeDest=NULL);

	//内存中的K路求并集输出到m_pVecDest中
	inline NODE*  K_Merge(NODE *);




   //归并列中带列序号，和K_Merge(vector<pair<NODE,int> >* )成对使用
	bool CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,pair<NODE,int>  *pNodePrDest);

	//内存中的K路求并集输出到m_pVecDest中
	inline pair<NODE,int> *   K_Merge(pair<NODE,int> * );

     //友元函数
	friend	int  K_MergeFile<NODE>(const char *pchFileIn,            const char *pchFileOut,
		bool(*CMP_NODE)(const NODE& L, const NODE& R), int nFreeMemSize);
	//友元函数
	friend int  K_MergeFile<NODE>(vector<string> &vecFileNames,   const char *pchFileOut,
		bool(*CMP_NODE)(const NODE& L, const NODE& R), int nCacheNodeCnt);
public:
	~CWinnerTree(void){};
	
private:
	inline bool CMP_NODE(const pair<NODE*,int> &nodeL, const pair<NODE*,int>& nodeR)
	{
// 		if (nodeL.first==NULL)//到一列的末尾插入NULL表示无限大
// 		{
// 			return 1;
// 		}
// 		else
// 		{
// 			if (nodeR.first==NULL)
// 			{
// 				return -1;
// 			}		
// 		}
// 		int minus=*(nodeL.first)-*(nodeR.first);
// 		if (minus==0)
// 		{
// 			return nodeL.second-nodeR.second;
// 		}
// 		return minus;
		return  M_CMP_ELEMENT(*(nodeL.first),*(nodeR.first))||
                !M_CMP_ELEMENT(*(nodeR.first),*(nodeL.first))&&nodeL.second<nodeR.second;
		
	}

	void MakeTree();	

	//内部使用函数
	bool K_MergeFile(vector<int> &vecTempFileHandle,int hWriteFileHandle,int nReadCnt,NODE*pOutPtr);	

private:
	vector<pair<NODE*,int> > m_vecTree;//完全二叉树结构

	vector<NODE*> m_vecRunBeginPtr;      //保存每一列起始地址
	vector<NODE*> m_vecRunTracks;        //当前跟踪到的各个Node
	vector<NODE*> m_vecRunEnds;         //Run的中止指针
	vector<int>   m_vecStopMark;        //每个Rund的中止标准
	vector<int>   m_vecRunInTreePos;    //每个Run号对应的TREE中的位置。

	NODE *m_pNodeDest;          //存放归并结果的地方；
	pair<NODE,int>  *m_pNodePrDest;          //存放归并结果的地方 pair 为结果和归并队号；

	//	NODE *m_pMaxNodePtr//;
	int m_nLevel;                      //层数
	int m_nLastLevelBegin;             //最后一层节点起始号
	size_t m_nStoppedRun;                 //归并完停止的列数

	bool (*M_CMP_ELEMENT)(const NODE&L,const NODE& R);//元素比较函数
	
private:
	//临时计算变量，速度起见
	//pair<NODE*,int> resPair;
	int nRun;
	int nTrack;//跟踪计算结点
	
};



//文件归并排序
//************************************
// Method:    K_MergeFile
// FullName:  <NODE>::K_MergeFile
// Access:    public 
// Returns:   int
// Qualifier:
// Parameter: const char *pchFileIn 输入文件名字
// Parameter: const char *pchFileOut输出文件名字

// Parameter: *CMP_NODE             比较函数
// Parameter: int nFreeMemSize      设定归并使用连续内存空间的大小
//************************************
template <class NODE>
int  K_MergeFile(const char *pchFileIn,const char *pchFileOut,bool(*CMP_NODE)(const NODE& L,const NODE& R),int nFreeMemSize)
{

	int fdSrc;
	
    fdSrc = ::open(pchFileIn, O_RDWR|O_BINARY|O_LARGEFILE, 0644);

	if (fdSrc == -1)
	{	
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}

//获得大小
	struct stat64 statInfo;
	if (fstat64(fdSrc, &statInfo ) < 0) 
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		//::close(fdSrc);
		return -1;
	}

	long long  llFileSize;

	llFileSize = statInfo.st_size;		//文件长度

	int fdDest;
	fdDest = ::open(pchFileOut, O_CREAT|O_RDWR|O_TRUNC|O_BINARY|O_LARGEFILE, 0644);

	if (fdDest == -1)
	{	
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}


	//计算临时文件目录路径
	string strDirectPrefix;
	const char*p=pchFileOut+strlen(pchFileOut);
	while (p!=pchFileOut&&*(p-1)!='\\'&&*(p-1)!='/')
	{
		--p;
	}
	if (p!=pchFileOut)
	{
		strDirectPrefix=string(pchFileOut,p);
	}




	if (llFileSize%sizeof(NODE)!=0)//文件大小应该是结构体的整数倍
	{
		fprintf(stderr,"FileSize error .\n");
		return -1;
	}


	long long llTotalEleCnt;
	//int nTotalEleCnt;//所有元素个数
	int nEachColCnt;//每个归并列元素个数
	int nColCnt;   //列个数
	unsigned int nReadCnt;//实际读取个数




	llTotalEleCnt=llFileSize/sizeof(NODE);
	nEachColCnt=nFreeMemSize/sizeof(NODE);
	if (llTotalEleCnt%nEachColCnt==0)
	{
		nColCnt=llTotalEleCnt/nEachColCnt;
	}
	else
	{
		nColCnt=llTotalEleCnt/nEachColCnt+1;
	}


	if (nColCnt<=0)
	{
		printf("文件大小为0.\n"); 
		return -1;
	}
	if (nEachColCnt<=0)
	{
		printf("归并空间不够0.\n"); 
		return -1;
	}
	int i;
	if (nColCnt==1)//内存能放下该文件
	{		
		vector<NODE> vecBuf;//向量缓冲	
		vecBuf.resize(nEachColCnt);
		
		if((nReadCnt=read(fdSrc,(void*)&vecBuf[0],llTotalEleCnt*sizeof(NODE)))==-1
			||nReadCnt!=llTotalEleCnt*sizeof(NODE))
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return -1;
		}
		vecBuf.erase(vecBuf.begin()+nReadCnt/sizeof(NODE),vecBuf.end());

		sort(vecBuf.begin(),vecBuf.end(),CMP_NODE);	
		
		if ((nReadCnt=write(fdDest,(void*)&vecBuf[0],llTotalEleCnt*sizeof(NODE)))==-1
			||nReadCnt!=llTotalEleCnt*sizeof(NODE))
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return -1;
		}

	}
	else//采用外排序法
	{
		//0.创建各个分段排序文件
		vector<NODE> vecBuf;//向量缓冲	
		vecBuf.resize(nEachColCnt);
		vector<string> vecTempFileName;
		vector<int> vecTempFileHandle;
		vecTempFileName.resize(nColCnt,"0");
		vecTempFileHandle.resize(nColCnt);
		unsigned int nWriteCnt;
		char chBuf[1024];

		for (i=0;i<nColCnt;++i )
		{
			sprintf(chBuf,"%d",i);
			vecTempFileName[i]=strDirectPrefix;//产生文件名字		
			vecTempFileName[i]+=chBuf;
			vecTempFileHandle[i] = ::open(vecTempFileName[i].c_str(), O_CREAT|O_RDWR|O_TRUNC|O_BINARY|O_LARGEFILE, 0644);
			if (vecTempFileHandle[i] ==  -1)
			{		
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
			}
		}

		//1读取缓冲区 写临时分段文件
		for ( i=0;i<nColCnt;++i)
		{

			
			if((nReadCnt=::read(fdSrc,(void*)&vecBuf[0],nEachColCnt*sizeof(NODE)))==-1)
			{
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
			}

// 			for (int k=0;k<128;++k)
// 			{
// 			  printf("%-10d%-10d",vecBuf[k].a,vecBuf[k].b);
// 			}
// 			getchar();
			if (i!=nColCnt-1&&nReadCnt!=nEachColCnt*sizeof(NODE))//读取个数有问题
			{
				fprintf(stderr,"Read element cnt  error %d.\n");
				return -1;
			}
			if (i==nColCnt-1)//最后一列可能读取的比较前面的列少
			{
				vecBuf.erase(vecBuf.begin()+nReadCnt/sizeof(NODE),vecBuf.end());
			}

			sort(vecBuf.begin(),vecBuf.end(),CMP_NODE);

			
			if ((nWriteCnt=write(vecTempFileHandle[i],(void*)&vecBuf[0],nReadCnt))==-1
				||nReadCnt!=nWriteCnt)
			{
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
			}
		}

		//准备归并
		int nEachMergeCnt=nEachColCnt/(nColCnt+1);//归并所用的每个缓冲区元素个数
		vecBuf.resize(nEachColCnt);

		NODE* outPtrBegin;//输出缓冲区起始位置

		vector<NODE*> vecNodePtr;
		vector<NODE*> vecNodeEndPtr;
		vecNodePtr.resize(nColCnt);
		vecNodeEndPtr.resize(nColCnt);



		for (i=0;i<nColCnt;++i)
		{
			vecNodePtr[i]=&vecBuf[0]+i*nEachMergeCnt;	
			lseek64(vecTempFileHandle[i],0,SEEK_SET);
			//SetFilePointer(vecTempFileHandle[i],0,NULL,FILE_BEGIN);
			if((nReadCnt=read(vecTempFileHandle[i],(void*)vecNodePtr[i],nEachMergeCnt*sizeof(NODE)))==-1)
			//if(!ReadFile(vecTempFileHandle[i],vecNodePtr[i],nEachMergeCnt*sizeof(NODE),&nReadCnt,NULL))
			{
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
				//return pair<NODE*,int>(NULL,-1);//表示读取文件失败
			}
			vecNodeEndPtr[i]=vecNodePtr[i]+nReadCnt/sizeof(NODE);
		}
		outPtrBegin=vecNodeEndPtr[nColCnt-1];

		CWinnerTree<NODE> winnerTree(CMP_NODE);
		winnerTree.CreateTree(vecNodePtr,vecNodeEndPtr);
		winnerTree.K_MergeFile(vecTempFileHandle,fdDest,nEachMergeCnt,outPtrBegin);

		for (i=0;i<nColCnt;++i)
		{
			close(vecTempFileHandle[i]);
			unlink(vecTempFileName[i].c_str());
		}

	}
	close(fdSrc);
	close(fdDest);
	return 0;
}

//要求各个文件已经排好序 
template <class NODE>
int  K_MergeFile(vector<string> &vecFileNames,   const char *pchFileOut,
				        bool(*CMP_NODE)(const NODE& L, const NODE& R), int nCacheNodeCnt)
{


	vector<int> vecTempFileHandle;
	vecTempFileHandle.resize(vecFileNames.size());
	for (int i=0;i<vecFileNames.size();++i )
	{		
// 		vecTempFileHandle[i] = 
// 			CreateFile(
// 			vecFileNames[i].c_str(),
// 			GENERIC_READ,
// 			FILE_SHARE_READ|FILE_SHARE_DELETE,
// 			0,
// 			OPEN_EXISTING,
// 			0,
// 			0);
// 		if (vecTempFileHandle[i] == INVALID_HANDLE_VALUE)
// 		{		
// 			printf("Could not create file handle object !(%d).\n",GetLastError()); 
// 			return -1;
// 		}
		vecTempFileHandle[i] = open(vecFileNames[i].c_str(), O_RDWR|O_BINARY|O_LARGEFILE, 0644);
		if (vecTempFileHandle[i] ==  -1)
		{		
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return -1;
		}
	}

// 	HANDLE hDestFile;
// 	hDestFile = 
// 		CreateFile(
// 		pchFileOut,
// 		GENERIC_WRITE,
// 		0,
// 		0,
// 		CREATE_ALWAYS,
// 		0,
// 		0);
// 	if (hDestFile == INVALID_HANDLE_VALUE)
// 	{		
// 		printf("Could not create file handle object !(%d).\n",GetLastError()); 
// 		return -1;
// 	}
	int fdDest;
	fdDest = ::open(pchFileOut, O_CREAT|O_RDWR|O_TRUNC|O_BINARY|O_LARGEFILE, 0644);

	if (fdDest == -1)
	{	
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return -1;
	}


	//准备归并
	vector<NODE> vecBuf;//向量缓冲	
	
	int nEachMergeCnt=nCacheNodeCnt/(vecFileNames.size()+1);//归并所用的每个缓冲区元素个数
	vecBuf.resize(nCacheNodeCnt);

	NODE* outPtrBegin;//输出缓冲区起始位置

	vector<NODE*> vecNodePtr;
	vector<NODE*> vecNodeEndPtr;
	vecNodePtr.resize(vecFileNames.size());
	vecNodeEndPtr.resize(vecFileNames.size());


    unsigned int nReadCnt;
	for (int i=0;i<vecFileNames.size();++i)
	{
		vecNodePtr[i]=&vecBuf[0]+i*nEachMergeCnt;	
		//SetFilePointer(vecTempFileHandle[i],0,NULL,FILE_BEGIN);
		//if(!ReadFile(vecTempFileHandle[i],vecNodePtr[i],nEachMergeCnt*sizeof(NODE),&nReadCnt,NULL))
		if((nReadCnt=read(vecTempFileHandle[i],vecNodePtr[i],nEachMergeCnt*sizeof(NODE)))==-1)
		{
			//printf("ReadFile error %d.\n",GetLastError());
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return -1;
			//return pair<NODE*,int>(NULL,-1);//表示读取文件失败
		}
		vecNodeEndPtr[i]=vecNodePtr[i]+nReadCnt/sizeof(NODE);
	}
	outPtrBegin=vecNodeEndPtr[vecFileNames.size()-1];

	CWinnerTree<NODE> winnerTree(CMP_NODE);
	winnerTree.CreateTree(vecNodePtr,vecNodeEndPtr);
	winnerTree.K_MergeFile(vecTempFileHandle,fdDest,nEachMergeCnt,outPtrBegin);

	for (int i=0;i<vecFileNames.size();++i)
	{
		//CloseHandle(vecTempFileHandle[i]);
		close(vecTempFileHandle[i]);
		//DeleteFile(vecTempFileName[i].c_str());
	}
	//CloseHandle(hDestFile);
	close(fdDest);
	return 0;
}

template<class NODE>
bool CWinnerTree<NODE>::CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,NODE *pNodeDest)
{
	m_nStoppedRun=0;	
	if (vecNodePtr.size()<1)
	{ return false;	}
	m_vecRunTracks=vecNodePtr;
	m_vecRunEnds=vecNodeEndPtr;
	m_vecRunBeginPtr=vecNodePtr;
	m_pNodeDest=pNodeDest;

	m_vecStopMark.resize(vecNodePtr.size(),0);		

	MakeTree();
	return true;

}
template<class NODE>
bool CWinnerTree<NODE>:: CreateTree(vector<NODE*>&vecNodePtr,vector<NODE*>&vecNodeEndPtr,pair<NODE,int>  *pNodePrDest)
{
	m_nStoppedRun=0;	
	if (vecNodePtr.size()<1)
	{ return false;	}
	m_vecRunTracks=vecNodePtr;
	m_vecRunEnds=vecNodeEndPtr;
	m_vecRunBeginPtr=vecNodePtr;
	m_pNodePrDest=pNodePrDest;

	m_vecStopMark.resize(vecNodePtr.size(),0);		

	MakeTree();
	return true;

}


template<class NODE>
inline NODE* CWinnerTree<NODE>::K_Merge(NODE *)
{
	//resPair=m_vecTree[0];
	
	while (m_vecRunTracks.size()!=m_nStoppedRun)
	{
		//m_pVecDest->push_back(*(m_vecTree[0].first));
		*m_pNodeDest++=*(m_vecTree[0].first);
		nRun=m_vecTree[0].second;
		nTrack=m_vecRunInTreePos[m_vecTree[0].second];//跟踪计算结点

		//插入一个数到最底层
		if (++(m_vecRunTracks[nRun])==m_vecRunEnds[nRun])//一列中止了
		{

			MakeTree();//一列中止重新构造树
			//++m_nStoppedRun;
		}
		else
		{
			m_vecTree[nTrack].first=m_vecRunTracks[nRun];
			m_vecTree[nTrack].second=nRun;
			//向根方向进行比较
			while (nTrack!=0)
			{
				if (nTrack%2==0)//是偶数为右子树
				{
					m_vecTree[(nTrack>>1)-1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack-1])?m_vecTree[nTrack]:m_vecTree[nTrack-1];				
					nTrack>>=1;
					nTrack-=1;
				}
				else
				{
					m_vecTree[nTrack>>1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack+1])?m_vecTree[nTrack]:m_vecTree[nTrack+1];				
					//nTrack-=1;
					nTrack>>=1;
				}	
			}
		}

	}	
	return m_pNodeDest;
}
template<class NODE>
inline pair<NODE,int> * CWinnerTree<NODE>::K_Merge(pair<NODE,int> *)
{
	pair<NODE,int> resPair;
	int second;
	while (m_nStoppedRun!=m_vecRunTracks.size())
	{


		second=m_vecTree[0].second;
		resPair.first=*(m_vecTree[0].first);
		resPair.second=second;
		//m_pVecDestPair->push_back(resPair);
		*m_pNodePrDest++=resPair;

		nRun=second;
		nTrack=m_vecRunInTreePos[second];//跟踪计算结点

		//插入一个数到最底层
		if (++(m_vecRunTracks[nRun])==m_vecRunEnds[nRun])//一列中止了
		{
			// 			m_vecTree[nTrack].first=NULL;
			// 			m_vecTree[nTrack].second=nRun;
			MakeTree();//一列中止重新构造树
			//++m_nStoppedRun;
		}
		else
		{
			m_vecTree[nTrack].first=m_vecRunTracks[nRun];
			m_vecTree[nTrack].second=nRun;
			//向根方向进行比较
			while (nTrack!=0)
			{
				if (nTrack%2==0)//是偶数为右子树
				{
					m_vecTree[(nTrack>>1)-1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack-1])?m_vecTree[nTrack]:m_vecTree[nTrack-1];				
					nTrack>>=1;
					nTrack-=1;
				}
				else
				{
					m_vecTree[nTrack>>1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack+1])?m_vecTree[nTrack]:m_vecTree[nTrack+1];				
					//nTrack-=1;
					nTrack>>=1;
				}	
			}
		}
	}

	return m_pNodePrDest;
}



template<class NODE>
bool CWinnerTree<NODE>::K_MergeFile(vector<int> &vecTempFileHandle,int hWriteFileHandle,int nReadCnt,NODE*pOutPtr)
{
	int nOutCnt=0;

	while (m_vecRunTracks.size()!=m_nStoppedRun)//所以文件没有归并完全
	{
		pOutPtr[nOutCnt++]=*(m_vecTree[0].first);

		if (nOutCnt==nReadCnt)//写缓冲区满
		{
			unsigned int nWriteCnt;
			if ((nWriteCnt=write(hWriteFileHandle,pOutPtr,nOutCnt*sizeof(NODE)))==-1
			//if (!WriteFile(hWriteFileHandle,pOutPtr,nOutCnt*sizeof(NODE),&nWriteCnt,NULL)
				||nOutCnt*sizeof(NODE)!=nWriteCnt)
			{
				//printf("WriteFile error %d.\n",GetLastError());
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return false;
			}
			nOutCnt=0;
		}

		nRun=m_vecTree[0].second;
		nTrack=m_vecRunInTreePos[m_vecTree[0].second];//跟踪计算结点

		//插入一个数到最底层
		++(m_vecRunTracks[nRun]);
		if (m_vecRunTracks[nRun]==m_vecRunEnds[nRun])//一列中止了
		{
			// 			m_vecTree[nTrack].first=NULL;
			// 			m_vecTree[nTrack].second=nRun;
			unsigned int nRealReadCnt=0;
			if ((nRealReadCnt=read(vecTempFileHandle[nRun],(void*)m_vecRunBeginPtr[nRun],nReadCnt*sizeof(NODE)))==-1)
			//if(!ReadFile(vecTempFileHandle[nRun],m_vecRunBeginPtr[nRun],nReadCnt*sizeof(NODE),&nRealReadCnt,NULL))
			{
				//printf("ReadFile error %d.\n",GetLastError());
				fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
				return false;
				//return pair<NODE*,int>(NULL,-1);//表示读取文件失败
			}

			if (nRealReadCnt==0)//一列文件完毕重新构造树
			{
				MakeTree();//一列中止重新构造树
				continue;
			}
			else
			{
				m_vecRunTracks[nRun]=m_vecRunBeginPtr[nRun];
				m_vecRunEnds[nRun]=m_vecRunTracks[nRun]+nRealReadCnt/sizeof(NODE);

			}			
			//++m_nStoppedRun;
		}

		m_vecTree[nTrack].first=m_vecRunTracks[nRun];
		m_vecTree[nTrack].second=nRun;
		//向根方向进行比较
		while (nTrack!=0)
		{
			if (nTrack%2==0)//是偶数为右子树
			{
				m_vecTree[(nTrack>>1)-1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack-1])?m_vecTree[nTrack]:m_vecTree[nTrack-1];				
				nTrack>>=1;
				nTrack-=1;
			}
			else
			{
				m_vecTree[nTrack>>1]=CMP_NODE(m_vecTree[nTrack],m_vecTree[nTrack+1])?m_vecTree[nTrack]:m_vecTree[nTrack+1];				
				//nTrack-=1;
				nTrack>>=1;
			}	
		}

	}

	if (nOutCnt)//缓冲区还有没写入文件的
	{
		unsigned int nWriteCnt;
		if ((nWriteCnt=write(hWriteFileHandle,pOutPtr,nOutCnt*sizeof(NODE)))==-1
		//if (!WriteFile(hWriteFileHandle,pOutPtr,nOutCnt*sizeof(NODE),&nWriteCnt,NULL)
			||nOutCnt*sizeof(NODE)!=nWriteCnt)
		{
			//printf("WriteFile error %d.\n",GetLastError());
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return false;
		}			
	}



	return true;
}

template<class NODE>
void CWinnerTree<NODE>::MakeTree()
{	
	for (size_t i=0;i<m_vecRunTracks.size();++i)
	{
		if ((m_vecRunTracks[i]==NULL||m_vecRunTracks[i]==m_vecRunEnds[i])&&m_vecStopMark[i]==0)
		{
			m_vecStopMark[i]=1;
			++m_nStoppedRun;
		}	
	}

	int nLeftRun=m_vecRunTracks.size()-m_nStoppedRun;
	if (nLeftRun==0)
	{
		return ;
	}

	//____________________构造任意节点有两颗子树，或者是叶子节点的 完全二叉树
	int i=1;
	m_nLevel=0;
	while(nLeftRun>i)
	{
		++m_nLevel;
		i<<=1;
	}	

	++m_nLevel;//完全二叉树的层数
	m_vecTree.resize(2*nLeftRun-1);//最后一层不满时 树节点数为 2*run-1
	m_nLastLevelBegin=m_vecTree.size()-nLeftRun;		


	//____________________填写所有叶子节点，即加入所有RUN的第一个元素
	typename vector<pair<NODE*,int> >::iterator itTree=m_vecTree.begin()+m_nLastLevelBegin;
	int nCnt=-1;
	m_vecRunInTreePos.clear();
	for (i=0;i<m_vecRunTracks.size();++i)
	{
		if ((m_vecRunTracks[i]!=NULL&&m_vecRunTracks[i]!=m_vecRunEnds[i]))			
		{
			(*itTree).first=m_vecRunTracks[i];
			(*itTree).second=i;
			++itTree;
			++nCnt;

		}
		m_vecRunInTreePos.push_back(nCnt+m_nLastLevelBegin);
	}

	if (nLeftRun<=1)
	{
		return;
	}

	//____________________填写其他节点
	int nR=m_vecTree.size()-1;
	int nL=nR-1;
	while (nL>0) 
	{
		m_vecTree[nL>>1]=CMP_NODE(m_vecTree[nL],m_vecTree[nR])?m_vecTree[nL]:m_vecTree[nR];	//左节点小
		nR-=2;
		nL-=2;
	}

}

template <class NODE>
NODE*  K_Intersect(vector<NODE*>&vecNodePtr,
				  vector<NODE*>&vecNodeEndPtr,
				  NODE*        pNodeDest,
				  bool(*CMP_NODE)(const NODE& L, const NODE& R),
				  void(*ADD_NODE)(NODE& L, NODE& R))
{

	size_t i,j;
	if (vecNodePtr.empty())
	{
		return pNodeDest;
	}
	for (j=0;j<vecNodePtr.size();++j)
	{
		if (vecNodePtr[j]==vecNodeEndPtr[j])
		{
			return pNodeDest;
		}				
	}
// 	if (vecNodePtr.size()==1)
// 	{
// 		(*pVecDest).assign(vecNodePtr[0],vecNodeEndPtr[0]);
// 		return ;
// 	}

	if (vecNodePtr.size()==1)
	{		
		NODE* pNode=vecNodePtr[0];
		if (vecNodePtr[0]==pNodeDest)
		{
			return vecNodeEndPtr[0];
		}

		for (;pNode!=vecNodeEndPtr[0];++pNode,++pNodeDest)
		{
			*pNodeDest=*pNode;
		}
		return pNodeDest;
	}


	

// 	for (i=1;i<vecNodePtr.size();++i) //{--for--}找各列头部 最大的值。
// 	{
// 		if (CMP_NODE(nMax,*(vecNodePtr[i])))
// 		{
// 			nMax=*(vecNodePtr[i]);
// 			nMaxMark=i;
// 		}
// 	}


	vector<NODE*> vecCurColMark=vecNodePtr;
	NODE *pCurNode;
	NODE *pEndNode;
	bool bSuccess;
	NODE nMax;
	int nMaxMark=0;
	nMax=*(vecCurColMark[nMaxMark]);
	while (true)
	{			
		bSuccess=true;
		for (i=(nMaxMark+1)%vecNodePtr.size();i!=nMaxMark;)
		{
			pCurNode=vecCurColMark[i];
			pEndNode=vecNodeEndPtr[i];

			while (pCurNode!=pEndNode&&CMP_NODE(*pCurNode,nMax))//去找一个相同
			{
				++pCurNode;
			}
			vecCurColMark[i]=pCurNode;

			if (pCurNode==pEndNode)                             //没找到
			{
				return pNodeDest;
			}
			else
			{
				if (CMP_NODE(nMax,*pCurNode))                          //上一个元素不是公共的，以当前列当前元素为基准
				{
					nMaxMark=i;
					nMax=*pCurNode;
					bSuccess=false;
						break;
				}
				else if(ADD_NODE)
					ADD_NODE(nMax, *pCurNode);

			}
			++i;
			i=i%vecNodePtr.size();
		}

		if (bSuccess)                 //找到一个共有因素
		{			
			          
			*pNodeDest++=nMax;
			//pVecDest->push_back(nMax);   
			for (j=0;j<vecCurColMark.size();++j)
			{
				if (++vecCurColMark[j]==vecNodeEndPtr[j])
				{
					return pNodeDest;
				}				
			}			
			nMax=*(vecCurColMark[nMaxMark]);	
// 			nMax=*vecCurColMark[0];			
// 			for (i=1;i<vecCurColMark.size();++i) //{--for--}找各列头部 最大的值。
// 			{			
// 				cout<<"\ni "<<i<<":*(vecCurColMark[i]) "<<"Id "<<vecCurColMark[i]->nDocID<<" off "<<vecCurColMark[i]->nOffset;
// 				if (CMP_NODE(nMax,*(vecCurColMark[i])))
// 				{
// 					nMax=*(vecCurColMark[i]);
// 					nMaxMark=i;
// 				}
// 			}
		}

	}
    return pNodeDest;
}

;

template <class NODE>
NODE*  K_InvertIntersect(vector<NODE*>&vecNodePtr,
						vector<NODE*>&vecNodeEndPtr,
						NODE*        pNodeDest,
					    int (*MINUS_INVERT)(NODE&l,NODE&r),
						NODE&(*INVERT_MINUS_INT)(NODE&l,int nMinus))
{

	int i,j;
	if (vecNodePtr.empty())
	{
		return pNodeDest;
	}
	for (j=0;j<vecNodePtr.size();++j)
	{
		if (vecNodePtr[j]==vecNodeEndPtr[j])
		{
			return pNodeDest;
		}				
	}

	
	if (vecNodePtr.size()==1)
	{		
		NODE* pNode=vecNodePtr[0];
		if (vecNodePtr[0]==pNodeDest)
		{
			return vecNodeEndPtr[0];
		}

		for (;pNode!=vecNodeEndPtr[0];++pNode,++pNodeDest)
		{
			*pNodeDest=*pNode;
		}
		return pNodeDest;
	}


	vector<NODE*> vecCurColMark=vecNodePtr;
	NODE *pCurNode;
	NODE *pEndNode;
	bool bSuccess;
	int nMatchGap;
	int nMinus;
	NODE nMax;
	int nMaxMark=0;
	nMax=*(vecCurColMark[nMaxMark]);
	while (true)
	{
		bSuccess=true;
		for (i=(nMaxMark+1)%vecNodePtr.size();i!=nMaxMark;)
		{
			pCurNode=vecCurColMark[i];
			pEndNode=vecNodeEndPtr[i];
			nMatchGap=i-nMaxMark;			
			while (pCurNode!=pEndNode&&MINUS_INVERT(*pCurNode,nMax)<nMatchGap)//去找一个相同
			{
				++pCurNode;
			}
		
			vecCurColMark[i]=pCurNode;

			if (pCurNode==pEndNode)                             //没找到
			{
				return pNodeDest;
			}
			else
			{
				if (MINUS_INVERT(*pCurNode,nMax)!=nMatchGap)                          //上一个元素不是公共的，以当前列当前元素为基准
				{
					nMaxMark=i;
					nMax=*pCurNode;
					bSuccess=false;
					break;
				}

			}
			++i;
			i=i%vecNodePtr.size();
		}

		if (bSuccess)                     //找到一个共有因素
		{
			 INVERT_MINUS_INT(nMax,i);    //取匹配的第一个字的偏移
			// nMaxMark=0;      
			 *pNodeDest++ = nMax;

			// pVecDest->push_back(nMax);   

			for (j=0;j<vecCurColMark.size();++j)
			{
				if (++vecCurColMark[j]==vecNodeEndPtr[j])
				{
					return pNodeDest;
				}	
			}

			nMax=*vecCurColMark[nMaxMark];			
// 			for (i=1;i<vecCurColMark.size();++i) //{--for--}找各列头部 最大的值。
// 			{
// 				if (MINUS_INVERT(*vecCurColMark[i],nMax)> i-nMaxMark)
// 				{
// 					nMax=*(vecCurColMark[i]);
// 					nMaxMark=i;
// 				}
// 			}

		}

	}

}
#endif
