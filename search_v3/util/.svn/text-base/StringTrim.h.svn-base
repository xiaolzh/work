#ifndef STRINGTRIM_H
#define STRINGTRIM_H

#include <cstring>
#include <ctype.h>
#include <string>
using std::string;

enum {GBKWORD,GBKSYMBOL,GBKUNCOMMON,HALFGBK,NONGBK};
enum {GBK,UTF8,HALFCODE,UNKNOWN};
//**src中都是完好的字符；
void EnCnStrTolower(char *src);
void EnCnStrToUpper(char *src);

//int UTF8OrGBK(const char* srcStr);
bool IsValidSign(char ch);
inline void Utf8ToGBK(char*src) ;
//**中英文字符预处理，包括去中英文符号，中文乱码，半个中文字符，以及将英文大写转小写//
int IsGBKWord(unsigned char ch1,unsigned char ch2);
//**resStr 为返回结果字符串　其空间长度为strlen(srcStr)+2; 返回值为字符串长度；
int  StrPreTreatment(const char* srcStr,char* resStr);

// 字符串去掉括号极其括号中的内容
bool StrRemoveBrackets(const char* strSrc,string& strOut);




#endif
