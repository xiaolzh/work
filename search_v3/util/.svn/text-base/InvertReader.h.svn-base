
#ifndef _INVERT_READER
#define _INVERT_READER
#include <fcntl.h>
#include <string>
#ifndef _WIN32
#include <inttypes.h>
#define O_BINARY 0
#else
typedef unsigned long long uint64_t;
#endif
using std::string;
//**大文件问题
struct SFetchInfo	
{
	SFetchInfo()
	{
		pPage=0;
		nRealLen=0;
		pNewPtr=0;
	}
	void*         pPage;
	size_t        nRealLen;
	void*         pNewPtr;//MERGE RESULT
};


class CInvertReader
{
public:
	CInvertReader(void){m_fd = -1;}

	static bool GlobleInit(const char *pchReaderHome);

	inline int GetFd(){return m_fd;}
	inline const string& GetFileName(){return m_strName;}
	bool Init(const char* pchIvtFile,int nPageSize);
	bool GetInvert(uint64_t nOffStart, uint64_t nOffEnd, void*& pIvt, SFetchInfo& fi);
	bool put(SFetchInfo& fi);
	bool Dispose();
	~CInvertReader(void);
	static bool GlobleDispose();
public://idx all load to memory

private://db relevance

	int			  m_fd;
	int           m_nPageSize;
	int           m_nPowOf2;
	string        m_strName;

};
#endif
