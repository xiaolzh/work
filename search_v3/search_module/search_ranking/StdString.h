#ifndef STD_STRING_H
#define STD_STRING_H

#include <string>
#include <inttypes.h>

using namespace std;

#define CHAR_INVALID 0
#define CHAR_DIGIT 1
#define CHAR_LOWERCASE 2
#define CHAR_UPPERCASE 3
#define CHAR_CHINESE 4
#define CHAR_SEGSEP 5
#define CHAR_CH_DIGIT 6
#define CHAR_CH_LOWERCASE 7
#define CHAR_CH_UPPERCASE 8

	extern uint8_t g_aCharType[0x100];
	extern uint8_t g_aCharTypeChinese[0x100];

	bool EraseUnprintable(string &strIn);
	bool HasInvalidChar(const string &strSrc);
	bool StdString(const string &strSrc, string &strDst);
	string StdString(const string &strSrc);
	bool LowerCaseString(const string &strSrc, string &strDst);
	string LowerCaseString(const string &strSrc);
	bool LowerCaseStringNew(const string &strSrc,string &strDst);
	string LowerCaseStringNew(const string &strSrc);
	
	inline uint8_t CharType(const char *p, size_t nPos, uint8_t &nLen)
	{
		if (p[nPos])
		{
			uint8_t nCharType = g_aCharType[(unsigned char)p[nPos]];
			if (nCharType == CHAR_CHINESE)
			{
				nLen = 2;
				if (!p[nPos+1])
				{
					nLen = 1;
					return CHAR_INVALID;
				}
				if ((unsigned char)p[nPos] == 0xa3)
				{
					uint8_t nCharTypeChinese = g_aCharTypeChinese[(unsigned char)p[nPos+1]];
					return nCharTypeChinese;
				}
				if (0xa1 <= (unsigned char)p[nPos] && (unsigned char)p[nPos] <= 0xa9)
				{
					return CHAR_INVALID;
				}
				return CHAR_CHINESE;
			}
			else
			{
				nLen = 1;
				return nCharType;
			}
		}
		else
		{
			nLen = 0;
		}
		return CHAR_INVALID;
	}

#endif
