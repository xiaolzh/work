#ifdef _WIN32
	#include <Windows.h>
#endif
#include "StringTrim.h"
#include "stdio.h"
#include <set>
#include <string>
#include <vector>
//#include "segdll.h"
using namespace std;
#pragma warning(disable:4996)
/*int StrPreTreatment(const char* srcStr,char* resStr)
{	

	char*tempStr=new char[strlen(srcStr)+1];
	char* start=tempStr;
	strcpy(tempStr,srcStr);

	int utf8Orgbk=UTF8OrGBK(tempStr);
	if(utf8Orgbk==UTF8)
		Utf8ToGBK(tempStr);
	if (utf8Orgbk==UNKNOWN)		
	{
		*resStr=0;
		delete []start;
		return 0;
	}


	int GBKUncommon=0;
	while (*tempStr) 
	{
		if(0<*tempStr&&*tempStr<127)
		{
			if(!isalnum(*tempStr)&&!IsValidSign(*tempStr))
				*tempStr=' ';
			//if('A'<=*tempStr&&*tempStr<='Z')
			//	*tempStr+='a'-'A';
			tempStr++;
		}
		else if (129<=(unsigned char)*tempStr&&(unsigned char)*tempStr<=254)
		{		
			//**半个中文字符中止
			if(!*(tempStr+1))
			{
				*tempStr=' ';
				break;
			}

			switch(IsGBKWord(*tempStr,*(tempStr+1))) 
			{
			case GBKUNCOMMON:
				++GBKUncommon;
				if(GBKUncommon>=2)
				{
					*resStr=0;
					delete []start;
					return 0;
				}
				break;					
			case GBKWORD:
				tempStr+=2;
				break;
			case GBKSYMBOL:
				*tempStr=*(tempStr+1)=' ';
				tempStr+=2;
				break;		
			default://**半个中文字符			
				*resStr=0;
				delete []start;
				return 0;
			}				
		}
		else
		{
			*tempStr=' ';
			tempStr++;
		}
	}  
	//**LTRIM RTRIM;
	char* tmpstr=start;
	char*tmpEnd=start+strlen(start);
	if(tmpEnd>start)
		--tmpEnd;

	while(' '<=*tmpstr&&*tmpstr<'0')
	{
		*tmpstr++=' ';
	}
	while(tmpEnd!=start&&(' '<=*tmpstr&&*tmpstr<'0')) //c++可能被除去
	{
		*tmpstr--=' ';
	}
	//**LTRIM RTRIM;

	//**除去多余空格和重复单词
	set<string> sets;
	vector<string> vecs;
	char seps[]=" ";	
	char*token=strtok(start,seps);

	while(token!=0)
	{
		if(token=="")
			continue;	
		if(!sets.count(token))
		{
			sets.insert(token); //除去重复的
			vecs.push_back(token);
		}

		token=strtok(NULL,seps);		

	}
	int i=0;
	string str;
	for(vector<string>::iterator iter=vecs.begin();iter!=vecs.end();iter++,i++)
	{	
		str+=(*iter).c_str();
		int t=vecs.size()-2;
		if(i<=t)
			str+=" ";		   		
	}


	delete [] start;
	strcpy(resStr,str.c_str());
	return strlen(resStr);
}
*/


int IsGBKWord(unsigned char ch1,unsigned char ch2)
{
	if ((ch1 >= 129)&&(ch2 <= 254))
	{
		if (((ch2 >= 64)&&(ch2 < 127)) ||((ch2 > 127)&&(ch2 <= 254)))
		{//**
			int posit = (ch1 - 129) * 190 + (ch2 - 64) - (ch2/128);
			//posit = posit * 2;
			// gbk/3
			if(ch1>=0x81&&ch1<=0xA0&&ch2>=0x40&&ch2<=0xFE)
				return GBKUNCOMMON;
			// gbk/4
			if(ch1>=0xAA&&ch1<=0xFE&&ch2>=0x40&&ch2<=0xA0)
				return GBKUNCOMMON;
			if (posit>=0 && posit<=6079)
				return GBKWORD;
			else if(posit>=7790 && posit<=23845)
			{
				return GBKWORD;
			}
			else
				return GBKSYMBOL;
		}
		else
			return HALFGBK;
	}
	else	
		return NONGBK;

}

/*
inline void Utf8ToGBK(char*src) {
	int len=MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)src, -1, NULL,0);
	//unsigned short * wszGBK = new unsigned short[len+1];
	WCHAR * wszGBK = new WCHAR [len+1];

	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (LPCTSTR)src, -1, wszGBK, len);

	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char *szGBK=new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte (CP_ACP, 0, wszGBK, -1, szGBK, len, NULL,NULL);

	strcpy(src, szGBK); //gbk比utf8长度要小
	delete[] szGBK;
	delete[] wszGBK;
}
*/
	//pre-condition src中都是完好的字符；
void EnCnStrTolower(char *src)
{

	while (*src) 
	{
		while(0<*src)
		{
			if ('A'<=*src&&*src<='Z') 
				*src+='a'-'A';
			src++;
		}
		while(*src<0)
		{
			if(!*(src+1))
			{
				src++;
				break;
			}
			src+=2;
		}
	}
}
void EnCnStrToUpper(char *src)
{

	while (*src) 
	{
		while(0<*src)
		{
			if ('a'<=*src&&*src<='z') 
				*src+='A'-'a';
			src++;
		}
		while(*src<0)
		{
			if(!*(src+1))
			{
				src++;
				break;
			}
			src+=2;
		}
	}
}

char* EnCnStrFind(char *src , char c)
{

	while (*src) 
	{
		while(0<*src)
		{
			if(*src == c)
				return src;
			src++;
		}
		while(*src<0)
		{
			if(!*(src+1))
			{
				src++;
				break;
			}
			src+=2;
		}
	}
}



bool IsValidSign(char ch)
{

	const char *p="+#./-";
	while(*p)
	{
		if(ch==*p)
			return true;
		p++;
	}
	return false;
}
bool StrRemoveBrackets(const char* strSrc,string& strOut)
{

	unsigned short uLeftCN=*(unsigned short*)"（";
	unsigned short uRightCN=*(unsigned short*)"）";
	char cLeftEN='(';
	char cRightEN=')';
	bool bChange=false;


	const char* p=strSrc;
	const char* q;
	while(*p)
	{
		if (*p<0)
		{
			if (p[1]==0) 
			{
				break;
			}

			if (*(unsigned short*)p==uLeftCN)
			{
				do 
				{
					p+=2;
					if (p[1]==0) 
					{
						++p;
					}
				} while (*p&&*(unsigned short*)p!=uRightCN);
				if (*p)
				{
					p+=2;
				}
				bChange=true;
			}
			else
			{
				strOut+=*p;
				++p;
				strOut+=*p;
				++p;
			}
		}
		else
		{
			if (*p==cLeftEN)
			{

				do
				{
					++p;
				}while(*p&&*p!=cRightEN);

				if (*p)
				{
					++p;
				}
				bChange=true;
			}
			else
			{
				strOut+=*p;
				++p;
			}
		}
	}
	return bChange;
}

