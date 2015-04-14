#ifndef MMAP_FILE_H
#define MMAP_FILE_H
#define ROUND_SIZE (1024*1024)
#define DROP_SECOND 160
#include <list>
using namespace std;

struct OLD_MAP_ELEMENT
{
	time_t m_dropTime;
	char*  m_ptr;
	size_t m_nLen;
};

class CMMapFile
{
public:
	CMMapFile();
	~CMMapFile();
	void* OpenMapPtr(const char* pchFileName, bool bReadOnly, int expectSz, int& retSz,bool bRound);
	int WriteData(char* src, size_t off, size_t sz);
	int ReleaseOldMap(void);
	char* GetPtr();
	bool  Sync();
private:
	char    *m_pBeg;
	size_t   m_nLen;
	list<OLD_MAP_ELEMENT>	m_dropList;

#ifdef _WIN32
	void* m_hFile;
	void* m_hMapFile;
#else
	int   m_fd; 
#endif
};





#endif
