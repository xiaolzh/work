#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "MyDef.h"
#include "InvertReader.h"
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif


#define MMAP_MAXSIZE (1024u*1024u*512u)
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
#ifdef _WIN32

	m_hFile = ::CreateFile(pchIvtFile,GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return NULL;

	m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE|SEC_COMMIT, 0, 0, NULL);
	if (m_hMapFile == NULL) 
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		CloseHandle(m_hFile);
		m_hFile = NULL;
		return NULL;
	}

	m_pBeg = (char*)MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (m_pBeg == NULL) {
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		CloseHandle(m_hMapFile);
		CloseHandle(m_hFile);
		m_hFile = m_hMapFile = NULL;
		return NULL;
	}
	m_nLen = GetFileSize(m_hFile,NULL); //文件长度
#else
	int fd;
	
	//fill to multiple pagesize
	FALSE_RETURN_STRERROR((fd=open(pchIvtFile, O_RDWR | O_LARGEFILE | O_CREAT, 0644)) == -1) 
                //                                long long  llFileSize;
	struct stat64 statInfo;
	if (fstat64(fd, &statInfo ) < 0) 
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		::close(fd);
		return false;
	}
	long long nFillCnt=nPageSize-statInfo.st_size%nPageSize;
	if(ftruncate64(fd, statInfo.st_size+nFillCnt))
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		::close(fd);
		return false;
	}
	m_fd = fd;	//::close(fd);
#endif
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
	pIvt=m_pBeg+(nPageNum<<m_nPowOf2)+nOffPage;
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
#ifdef	_WIN32
	return ;
#endif
	close(m_fd);
}

bool CInvertReader::GlobleDispose()
{
	return true;
}
