#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define O_LARGEFILE 0
#define  ftruncate _chsize
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#define _O_BINARY 0
#include <unistd.h>
#endif
#include "BitMapFile.h"


#define ROUND 4*1024*32


CBitMapFile::CBitMapFile()
{
	m_pBeg=NULL;
	m_nLen=0;


#ifdef _WIN32
	m_hFile=NULL;
	m_hMapFile=NULL;
#else
	m_fd=0;
#endif
}

CBitMapFile::~CBitMapFile()
{
	if (m_pBeg!=NULL) 
	{
#ifdef _WIN32
		UnmapViewOfFile(m_pBeg);
		CloseHandle(m_hMapFile);
		CloseHandle(m_hFile);
#else
		munmap(m_pBeg, m_nLen);
		::close(m_fd);
#endif
	}
}

void* CBitMapFile::OpenBitMap(const char* pchFileName,int bitCount)
{
	int nLeft = bitCount%ROUND;
	int nExpectCount = bitCount ;
	if (nLeft)
	{
		nExpectCount = bitCount + ROUND - nLeft;
	}

	m_nLen = nExpectCount / 8;

#ifdef _WIN32

	m_hFile = ::CreateFile(pchFileName,GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return NULL;

	
	int fz = GetFileSize(m_hFile,NULL); //文件长度

	if (fz != m_nLen)
	{
		if(SetFilePointer(m_hFile,m_nLen,0,FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			return NULL;
		if(!SetEndOfFile(m_hFile))
			return NULL;
	}

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

	return m_pBeg;
#else

	m_fd = ::open(pchFileName, O_RDWR|O_CREAT, 0644);
	if (m_fd == -1)
	{
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return NULL;
	}

	struct stat statInfo;
	if (fstat(m_fd, &statInfo ) < 0) 
	{
		::close(m_fd);
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return NULL;
	}

	if (statInfo.st_size != m_nLen)
	{
		if(ftruncate(m_fd,m_nLen)<0)
			return NULL;
	}

	m_pBeg = (char*)mmap(0, m_nLen, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);

	if (m_pBeg == MAP_FAILED || m_pBeg==NULL) {
		::close(m_fd);
		m_pBeg = NULL;
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return NULL;
	}

	return m_pBeg;
#endif
}

bool  CBitMapFile::Sync()
{

	if (m_pBeg!=NULL) 
#ifdef _WIN32
		return FlushViewOfFile(m_pBeg,m_nLen);
#else
		return msync(m_pBeg, m_nLen, MS_SYNC)!=-1;		//MS_SYNC表示同步立即执行
#endif
		
	return true;
}
