#ifndef BIT_MAP_FILE_H
#define BIT_MAP_FILE_H

class CBitMapFile
{
public:
	CBitMapFile();
	~CBitMapFile();
	void* OpenBitMap(const char* pchFileName, int bitCount);

	inline int GetBitCount(){return m_nLen<<3;}
	inline void SetBit(unsigned int n){m_pBeg[n>>3] |= (1<<(n&7));}
	inline bool TestBit(unsigned int n){return (m_pBeg[n>>3] & (1<<(n&7)))!=0;}
	inline void ClearBit(unsigned int n){m_pBeg[n>>3] &= (~(1<<(n&7)));}

	bool  Sync();
private:
	char    *m_pBeg;
	size_t   m_nLen;

#ifdef _WIN32
	void* m_hFile;
	void* m_hMapFile;
#else
	int   m_fd; 
#endif
};





#endif
