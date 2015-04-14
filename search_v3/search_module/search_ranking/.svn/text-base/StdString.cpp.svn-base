#include "StdString.h"

	uint8_t g_aCharType[0x100] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,0,0,0,0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,5};
	uint8_t g_aCharTypeChinese[0x100] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6,6,6,6,6,6,6,6,6,6,0,0,0,0,0,0,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,0,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,0,0,0,0,0};

	bool HasInvalidChar(const string &strSrc)
	{
		const char *p = strSrc.c_str();
		uint8_t nLen;
		for (size_t i = 0; p[i]; )
		{
			if (CharType(p, i, nLen) == CHAR_INVALID)
			{
				return true;
			}
			i += nLen;
		}
		return false;
	}
	bool StdString(const string &strSrc, string &strDst)
	{
		strDst.clear();
		const char *p = strSrc.c_str();
		uint8_t nLen;
		for (size_t i = 0; p[i]; )
		{
			switch (CharType(p, i, nLen))
			{
				case CHAR_INVALID:
					strDst += ' ';
					break;
				case CHAR_UPPERCASE:
					strDst += (char)((unsigned char)p[i]+('a'-'A'));
					break;
				case CHAR_CH_DIGIT:
					strDst += (char)((unsigned char)p[i+1]+('0'-0xb0));
					break;
				case CHAR_CH_LOWERCASE:
					strDst += (char)((unsigned char)p[i+1]+('a'-0xe1));
					break;
				case CHAR_CH_UPPERCASE:
					strDst += (char)((unsigned char)p[i+1]+('a'-0xc1));
					break;
				default:
					strDst.append(p+i, nLen);
					break;
			}
			i += nLen;
		}
		return true;
	}
	string StdString(const string &strSrc)
	{
		string strDst;
		StdString(strSrc, strDst);
		return strDst;
	}

	bool LowerCaseString(const string &strSrc, string &strDst)
	{
		strDst.clear();
		const char *p = strSrc.c_str();
		uint8_t nLen;
		bool bLastIsSpace = false;
		for (size_t i = 0; p[i]; )
		{
			switch (CharType(p, i, nLen))
			{
				case CHAR_UPPERCASE:
					strDst += (char)((unsigned char)p[i]+('a'-'A'));
					break;
				case CHAR_CH_DIGIT:
					strDst += (char)((unsigned char)p[i+1]+('0'-0xb0));
					break;
				case CHAR_CH_LOWERCASE:
					strDst += (char)((unsigned char)p[i+1]+('a'-0xe1));
					break;
				case CHAR_CH_UPPERCASE:
					strDst += (char)((unsigned char)p[i+1]+('a'-0xc1));
					break;
				default:
					if (p[i] == ' ' && strncmp(p+i, "¡¡", 2) == 0)
					{
						bLastIsSpace = true;
					}
					else if(bLastIsSpace)
					{
						strDst += ' ';
						bLastIsSpace = false;
						strDst.append(p+i, nLen);
					}
					else
					{
						strDst.append(p+i, nLen);
					}
					//strDst.append(p+i, nLen);
					break;
			}
			i += nLen;
		}
		return true;
	}
	string LowerCaseString(const string &strSrc)
	{
		string strDst;
		LowerCaseString(strSrc, strDst);
		return strDst;
	}

	bool LowerCaseStringNew(const string &strSrc, string &strDst)
	{
		strDst.clear();
		const char *p = strSrc.c_str();
		uint8_t nLen;
		bool bLastIsSpace = false;
		for (size_t i = 0; p[i]; )
		{
			switch (CharType(p, i, nLen))
			{
				
				/*case CHAR_UPPERCASE:
					strDst += (char)((unsigned char)p[i]+('a'-'A'));
					break;*/
				case CHAR_CH_DIGIT:
					strDst += (char)((unsigned char)p[i+1]+('0'-0xb0));
					break;
				case CHAR_CH_LOWERCASE:
					strDst += (char)((unsigned char)p[i+1]+('a'-0xe1));
					break;
				case CHAR_CH_UPPERCASE:
					strDst += (char)((unsigned char)p[i+1]+('a'-0xe1));
					break;
				default:
					if (p[i] == ' ' && strncmp(p+i, "¡¡", 2) == 0)
					{
						bLastIsSpace = true;

					}
					else if(bLastIsSpace)
					{
						strDst += ' ';
						bLastIsSpace = false;
						strDst.append(p+i, nLen);
					}
					else
					{
						strDst.append(p+i, nLen);
					}
					break;
			}
			i += nLen;
		}
		return true;
	}

	string LowerCaseStringNew(const string &strSrc)
	{
		string strDst;
		LowerCaseStringNew(strSrc, strDst);
		return strDst;
	}

	bool EraseUnprintable(string &strIn)
	{
		for (size_t i = 0; i < strIn.length(); i++)
		{
			if (strIn[i] > 0 && !isprint(strIn[i]))
			{
				strIn[i] = ' ';
			}
		}
		return true;
	}


