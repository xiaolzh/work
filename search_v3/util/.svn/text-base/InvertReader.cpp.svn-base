#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "UtilDef.h"
#include "InvertReader.h"
#ifdef _WIN32
#include <windows.h>
#define O_LARGEFILE 0
#define  ftruncate64 _chsize
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#define O_BINARY 0
#endif


#define MMAP_MAXSIZE 1024u*1024u*512u
#ifndef _WIN32
#include <unistd.h>
#else
#include <io.h>
#endif


bool CInvertReader::GlobleInit(const char *pchReaderHome)
{
	return true;
}

bool CInvertReader::Init(const char* pchIvtFile,int nPageSize)
{
	nPageSize = 4096;
	m_nPageSize=nPageSize;
	int fd;

	FALSE_RETURN_STRERROR((fd=open(pchIvtFile, O_RDWR | O_LARGEFILE |O_BINARY, 0644)) == -1) 
	struct stat statInfo;
	if (fstat(fd, &statInfo ) < 0) 
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		::close(fd);
		return false;
	}
	
	long long nFillCnt=nPageSize-statInfo.st_size%nPageSize;
	if((statInfo.st_size==0 || nFillCnt!=nPageSize)
		&&ftruncate64(fd, statInfo.st_size+nFillCnt))
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		::close(fd);
		return false;
	}
	m_fd = fd;	

	int nTmp=nPageSize;
	m_nPowOf2=0;
	while (nTmp>>=1)
		++m_nPowOf2;
	m_strName=pchIvtFile;
	return true;

}
//>=noffStart,<nOffEnd;
bool CInvertReader::GetInvert(uint64_t nOffStart, uint64_t nOffEnd, void*& pIvt, SFetchInfo& fi)
{

	uint64_t nPageNum=nOffStart>>m_nPowOf2;
	size_t nOffPage=nOffStart-(nPageNum<<m_nPowOf2);
	size_t nGetCnt=nOffEnd-nOffStart;
	size_t nRealMapCnt = m_nPageSize;
	if (m_nPageSize-nOffPage<nGetCnt)//first page enough
	{
		nGetCnt-=(m_nPageSize-nOffPage);
		while(nGetCnt>=m_nPageSize)
		{
			nGetCnt-=m_nPageSize;
			nRealMapCnt+=m_nPageSize;
		}
		if(nGetCnt)
			nRealMapCnt+=m_nPageSize;
			
	}
#ifdef	_WIN32
	int nCnt;
	int nNeedLen=nOffEnd-nOffStart;
	lseek(m_fd, nOffStart,SEEK_SET);
	fi.pPage = new char[nNeedLen];
	nCnt=read(m_fd,fi.pPage,nNeedLen);
	pIvt=(char*)fi.pPage;
	if (nCnt!=nNeedLen)
	{
		return false;
	}
	return true;
#else
	pIvt = mmap(0, nRealMapCnt, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, nPageNum<<m_nPowOf2);
//	fprintf(stderr, "start =%u, end=%u, pno =%u ,getcnt= %u, 1stget= %u,offpage=%u,  nRealMapCnt =%u, pow = %u \n",nOffStart, nOffEnd, nPageNum, nOffEnd-nOffStart, m_nPageSize-nOffPage,  nOffPage, nRealMapCnt, m_nPowOf2);
	FALSE_RETURN(pIvt==MAP_FAILED);	
	fi.pPage=pIvt;
	fi.nRealLen=nRealMapCnt;
	pIvt=(char*)pIvt+nOffPage;
	return true;
#endif
}

bool CInvertReader::put(SFetchInfo& fi)
{
#ifdef	_WIN32
	delete fi.pPage;
	return true;
#else
	if (fi.pPage)
		munmap(fi.pPage, fi.nRealLen);
	return true;
#endif
}

bool CInvertReader::Dispose()
{	
	return true;
}

CInvertReader::~CInvertReader(void)
{
	if (m_fd != -1)
		close(m_fd);
}

bool CInvertReader::GlobleDispose()
{
	return true;
}
