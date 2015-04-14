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
#include "UtilDef.h"
#include "MMapFile.h"



CMMapFile::CMMapFile()
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

CMMapFile::~CMMapFile()
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

int CMMapFile::ReleaseOldMap(void)
{
#ifdef _WIN32
#else

	time_t   curStamp;
	time(&curStamp);
	while(!m_dropList.empty())
	{
		list<OLD_MAP_ELEMENT>::iterator i = m_dropList.begin();
		if(curStamp - i->m_dropTime > DROP_SECOND)
		{
			munmap(i->m_ptr, i->m_nLen);
			m_dropList.erase(i);
		}
		else
			break;
	}
#endif
	return 0;
}


int CMMapFile::WriteData(char* src, size_t off, size_t sz)
{
	if (sz >= ROUND_SIZE)
	{
		return -1;
	}

#ifdef _WIN32
	if (m_pBeg + off + sz > m_pBeg + m_nLen)//remap
	{
		m_nLen += ROUND_SIZE;
		UnmapViewOfFile(m_pBeg);
		CloseHandle(m_hMapFile);


		if(SetFilePointer(m_hFile,m_nLen,0,FILE_BEGIN)==INVALID_SET_FILE_POINTER)
			return -1;
		if(!SetEndOfFile(m_hFile))
			return -1;

		m_hMapFile = CreateFileMapping(m_hFile, NULL, PAGE_READWRITE|SEC_COMMIT, 0, 0, NULL);
		if (m_hMapFile == NULL) 
		{
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			CloseHandle(m_hFile);
			return -1;
		}

		m_pBeg = (char*)MapViewOfFile(m_hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (m_pBeg == NULL) {
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			CloseHandle(m_hMapFile);
			CloseHandle(m_hFile);
			return -1;
		}
	}
#else

	if (m_pBeg + off + sz > m_pBeg + m_nLen)//remap
	{
		time_t   curStamp;
		time(&curStamp);
		OLD_MAP_ELEMENT o;
		o.m_dropTime = curStamp;
		o.m_nLen = m_nLen;
		o.m_ptr = m_pBeg;
		m_dropList.push_back(o);

		m_nLen += ROUND_SIZE;
		ftruncate(m_fd,m_nLen);
		m_pBeg = (char*)mmap(0, m_nLen, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);
		if (m_pBeg == MAP_FAILED || m_pBeg == NULL) {
			m_pBeg = o.m_ptr ;
			m_dropList.clear();
			fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
			return -1;
		}
	}
#endif
	memcpy(m_pBeg + off,src,sz);
	return 0;
}



char* CMMapFile::GetPtr(){return (char*)m_pBeg;}
void* CMMapFile::OpenMapPtr(const char* pchFileName, bool bReadOnly, int expectSz, int& retSize, bool bRound)
{
	retSize=0;
#ifdef _WIN32

	m_hFile = ::CreateFile(pchFileName,GENERIC_WRITE|GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(m_hFile == INVALID_HANDLE_VALUE)
		return NULL;
	
	m_nLen = GetFileSize(m_hFile,NULL); //文件长度


	if (expectSz == 0)//follow org size->round
	{
		//m_nLen = statInfo.st_size;	
	}
	else //by designate
	{
		m_nLen = expectSz;
	}

	if (!bReadOnly && bRound)
	{
		if (m_nLen==0||m_nLen%ROUND_SIZE!=0)
		{
			m_nLen += (ROUND_SIZE - m_nLen%ROUND_SIZE);
			if(SetFilePointer(m_hFile,m_nLen,0,FILE_BEGIN)==INVALID_SET_FILE_POINTER)
				return NULL;
			if(!SetEndOfFile(m_hFile))
				return NULL;
		}
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

	retSize=m_nLen;
	return m_pBeg;
#else

	if (bReadOnly) 
		m_fd = ::open(pchFileName, O_RDONLY, 0644);
	else 
	{
		bRound==true?m_fd = ::open(pchFileName, O_RDWR|O_CREAT, 0644):
			m_fd = ::open(pchFileName, O_RDWR, 0644);
	}

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

	if (expectSz == 0)//follow org size->round
	{
		m_nLen = statInfo.st_size;	
	}
	else //by designate
	{
		m_nLen = expectSz;
	}
	if (!bReadOnly && bRound)
	{
		if (m_nLen==0||m_nLen%ROUND_SIZE!=0)
		{
			m_nLen += (ROUND_SIZE - m_nLen%ROUND_SIZE);
			if(ftruncate(m_fd,m_nLen)<0)
				return NULL;
		}
	}

	if (bReadOnly) 
		m_pBeg = (char*)mmap(0, m_nLen, PROT_READ, MAP_SHARED, m_fd, 0);
	else 
		m_pBeg = (char*)mmap(0, m_nLen, PROT_READ|PROT_WRITE, MAP_SHARED, m_fd, 0);

	if (m_pBeg == MAP_FAILED || m_pBeg==NULL) {
		::close(m_fd);
		m_pBeg = NULL;
		fprintf(stderr,"file:%s , line: %d, error info: %s\n",__FILE__,__LINE__,strerror(errno));
		return NULL;
	}
	retSize = m_nLen;
	return m_pBeg;
	#endif
}

bool  CMMapFile::Sync()
{

	if (m_pBeg!=NULL) 
#ifdef _WIN32
		return FlushViewOfFile(m_pBeg,m_nLen);
#else
		return msync(m_pBeg, m_nLen, MS_SYNC)!=-1;		//MS_SYNC表示同步立即执行
#endif
		
	return true;
}
