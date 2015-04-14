#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <assert.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "circle_queue.h"


//we store file in this format
//1. header three fields of int |bufsize|write off|read off
//2. and continue with each request
//   time_stamp|len|string|......
//3. when we meet the end we continue from the begin;
//4. we read one url when timestamp is comming

char* InitializeCircleQueue(const char* pName, int bufLen)
{

	int fd = open(pName, O_CREAT|O_RDWR, 0644);
	if (fd < 0)
	{
		return NULL;
	}

	ftruncate(fd, bufLen);  
	char *p = (char*)mmap(0, bufLen,  PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	close(fd);
	if(p == MAP_FAILED)
	{                       
		return NULL;
	}

	// use last;	
	if (bufLen == *(int*)p && *((int*)p + 1) && *((int*)p + 2))
		return p;

	*(int*)p = bufLen;
	*((int*)p + 1) = BUFFER_HEADER_LEN;
	*((int*)p + 2) = BUFFER_HEADER_LEN;
	return p;

}


int StoreToCircleQueue(char* p, const char* buf, int len)
{
	int bufLen = *(int*)p;
	int readOff = *((int*)p + 1);
	int* pWriteOff = (int*)p + 2;
	int nextWriteOff = *pWriteOff + STORE_HEADER_LEN + len;
	time_t curStamp;
	time(&curStamp);

	if (readOff > *pWriteOff)
	{
		if (nextWriteOff >= readOff)
		{
			return BUFFER_FULL;
		}	
		else
		{
			p += *pWriteOff;
		}
	}
	else
	{
		if (nextWriteOff >= bufLen - (int)sizeof(time_t))// reserve time_t for turning flag
		{
			nextWriteOff = BUFFER_HEADER_LEN + STORE_HEADER_LEN + len;
			if (nextWriteOff >= readOff)// 
			{
				return BUFFER_FULL;
			}
			else
			{
				//*pWriteOff = BUFFER_HEADER_LEN;
				*(time_t*)(p + *pWriteOff) = 0;//set flag for reading to return to begining
				p += BUFFER_HEADER_LEN;				
			}
		}
		else
		{
			p += *pWriteOff;
		}
	}

	*(time_t*)p = curStamp;
	p += sizeof(time_t);
	*(int*)p = len;
	p += sizeof(int);
	memcpy(p, buf, len);	
	*pWriteOff = nextWriteOff;
	return 0;
	
}


int ReadFromCircleQueue(char* p, char** retPtr, time_t expectInterval, time_t* pPassedInterVal)
{
	char* orgPtr = p;

	int readOff = *((int*)p + 1);
	int writeOff = *((int*)p + 2);

	//case 1 empty
	if (readOff == writeOff) // 
	{
		return BUFFER_EPMTY;
	}

	if (*(time_t*)(p+readOff) == 0)//turning  flag meet
	{
		*((int*)p + 1) = BUFFER_HEADER_LEN;
		readOff = *((int*)p + 1);
		if (readOff == writeOff) // 
		{
			return BUFFER_EPMTY;
		}
	}
	
	//time stamp not suitable
	time_t curStamp;
	time(&curStamp);
	*pPassedInterVal = curStamp - *(time_t*)(p+readOff);
	if (*pPassedInterVal < expectInterval)
	{
		return TIME_AGAIN;		
	}
	
	p += readOff + sizeof(time_t);
	int retLen = *(int*)p;
	p += sizeof(int);
	*retPtr = p;
	p += retLen;
	readOff = p - orgPtr ;
	*((int*)orgPtr + 1) = readOff;
	return retLen;
}

int GetBufferLeftRate(char* p)
{
	int bufLen = *(int*)p;
	int readOff = *((int*)p + 1);
	int writeOff = *((int*)p + 2);	
	if (readOff == writeOff)
		return 100;
	if (readOff > writeOff)
		return (int)((long long)(readOff - writeOff)*100/bufLen);
	else
		return (int)((long long)(bufLen - writeOff + readOff)*100/bufLen);
}

int IsCircleQueueEmpty(char* p)
{
	return *((int*)p + 1) == *((int*)p + 2);
}

int FindLastZeroLenOff(char* p)
{
	char* orgPtr = p;

	int readOff = *((int*)p + 1);
	int writeOff = *((int*)p + 2);
	int lastReadOff = -1;

	p = orgPtr + readOff;
	while (true)
	{
		if (readOff == writeOff) // 
			break;
		if (*(time_t*)p == 0)//turning  flag meet
		{
			readOff = BUFFER_HEADER_LEN;
			if (readOff == writeOff) // 
				break;
		}
		p = orgPtr + readOff + sizeof(time_t);
		if (*(int*)p == 0) //one zero len mark
			lastReadOff = readOff;
		readOff += sizeof(time_t) + sizeof(int) + *(int*)p;
		p = orgPtr + readOff;
	}

	return lastReadOff;
}

void SetReadOff(char* p, int off)
{
	*((int*)p + 1) = off;
}


int GetReadOff(char* p)
{
	return *((int*)p + 1);
}
int GetWriteOff(char* p)
{
	return *((int*)p + 2);

}

int ReleaseCircleQuery(char* p, int len)
{
	munmap(p, len);
}

int PeekAllFromQueue(char* p, string& strOut, int tid, int& cnt)
{
	int tmp = cnt;
	char buf_tid[64];
	char* orgPtr = p;

	int readOff = *((int*)p + 1);
	int writeOff = *((int*)p + 2);

	p = orgPtr + readOff;
	while (true)
	{
		if (readOff == writeOff) // 
			break;
		if (*(time_t*)p == 0)//turning  flag meet
		{
			readOff = BUFFER_HEADER_LEN;
			if (readOff == writeOff) // 
				break;
		}
		p = orgPtr + readOff + sizeof(time_t);
		if (*(int*)p != 0) 
		{
			sprintf(buf_tid, "log_id = %d-%d ", tid, cnt);
			strOut += buf_tid;
			strOut += (p + sizeof(int));
		}
		else
			++cnt;
		readOff += sizeof(time_t) + sizeof(int) + *(int*)p;
		p = orgPtr + readOff;
	}
	if (!strOut.empty())
		++cnt;
	return readOff;
}

int PeekSessionsFromQueue(char* p, string& strOut, int tid, int& cnt)
{
	int tmp = cnt;
	char buf_tid[64];
	int off = FindLastZeroLenOff(p);
	if (off != -1)
	{
		char* orgPtr = p;

		int readOff = *((int*)p + 1);
		int writeOff = *((int*)p + 2);

		p = orgPtr + readOff;
		while (true)
		{
			if (readOff == off) // 
				break;
			if (*(time_t*)p == 0)//turning  flag meet
			{
				readOff = BUFFER_HEADER_LEN;
				if (readOff == off) // 
					break;
			}
			p = orgPtr + readOff + sizeof(time_t);
			if (*(int*)p != 0) 
			{
				sprintf(buf_tid, "log_id = %d-%d ", tid, cnt);
				strOut += buf_tid;
				strOut += (p + sizeof(int));
			}
			else
				++cnt;
			readOff += sizeof(time_t) + sizeof(int) + *(int*)p;
			p = orgPtr + readOff;
		}

		if (!strOut.empty())
			++cnt;

		return off+sizeof(time_t) + sizeof(int);//next newstart
		

	}
	else
		return GetReadOff(p);
}
