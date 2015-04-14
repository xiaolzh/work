#ifndef DETAIL_H
#define DETAIL_H

#include "MyDef.h"
#include <sys/types.h>
#include <unistd.h>
#include "InvertReader.h"
#include "FileOperation.h"
#include "MMapFile.h"
class CDetail
{
public:
	CDetail(void)
	{
	}

	~CDetail(void)
	{
	}
	void Dispose()
	{
		m_IvtRdr.Dispose();
	}

	bool WriteDetail(const char* src,uint nLen,int nDocId)
	{
#ifndef _WIN32
#ifndef NOUPDATE
		//struct stat64 statInfo;
		//FALSE_RETURN(fstat64(m_IvtRdr.GetFd(), &statInfo));	
		m_idxMF.ReleaseOldMap();
		int nRoundTime = nLen/sizeof(uint);//倍数
		int nLeft=0;
		if (nLen%sizeof(uint))
		{
			nLeft=sizeof(uint)-nLen%sizeof(uint);
			nRoundTime+=1;
		}

		uint* pIdx=(uint*)m_idxMF.GetPtr();
		uint64_t off=(uint64_t)pIdx[nDocId]*sizeof(uint);
		lseek64(m_IvtRdr.GetFd(),off,SEEK_SET);
		uint nWriteCnt;
		nWriteCnt=write(m_IvtRdr.GetFd(),src,nLen);
		FALSE_RETURN(((int)nWriteCnt==-1||nWriteCnt!=nLen));
		if (nLeft)
		{
			nWriteCnt=write(m_IvtRdr.GetFd(),"\0\0\0\0",nLeft);
			FALSE_RETURN(((int)nWriteCnt==-1||(int)nWriteCnt!=nLeft));
		}
		uint sz=*((uint*)m_idxMF.GetPtr()+nDocId)+nRoundTime;
		FALSE_RETURN(m_idxMF.WriteData((char*)&sz,(nDocId+1)*sizeof(uint),sizeof(uint)));
#endif
#endif
		return true;
	}

    bool Sync() {
        return m_idxMF.Sync();
    }

	bool LoadDetail(const char* pchIdx, const char* pchIvt,size_t nPageSize)
	{
		//FALSE_RETURN_STRERROR(!LoadStruct(pchIdx,m_vIdx,false));
		int sz;
		FALSE_RETURN(!m_idxMF.OpenMapPtr(pchIdx,false,0,sz,true));
		FALSE_RETURN(!m_IvtRdr.Init(pchIvt,nPageSize));
		return true;
	}

	const char* GetData(int nDocId,SFetchInfo &fi)
	{
		void* pRetPtr=NULL;
		uint* pIdx=(uint*)m_idxMF.GetPtr();
		
		if(!m_IvtRdr.GetInvert((uint64_t)pIdx[nDocId]*sizeof(uint),(uint64_t)pIdx[nDocId+1]*sizeof(uint),pRetPtr,fi))
			return NULL;
		return	(const char*)pRetPtr;
	}

	inline bool PutData(SFetchInfo& fi)
	{
		return m_IvtRdr.put(fi);
	}

	inline size_t GetDocLen(int nDocId)
	{
		uint* pIdx=(uint*)m_idxMF.GetPtr();
		return sizeof(uint)*(pIdx[nDocId+1] - pIdx[nDocId]);
	}
private:
	CMMapFile m_idxMF;
//	vector<uint>      m_vIdx; //下标代表KEY标号 from 0 to maxid+1，val表示本key在倒排中的按照节点偏移
	CInvertReader       m_IvtRdr;
};

#endif
