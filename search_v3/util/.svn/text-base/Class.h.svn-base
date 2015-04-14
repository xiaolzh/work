#ifndef CLASS_H
#define CLASS_H

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <algorithm>
#include <math.h>
#include "FileOperation.h"
using std::string;
using std::vector;

typedef unsigned long long u64;

static u64 g_classMark[9]=
{
	0x0,
	0xff,
	0xffff,
	0xffffff,
	0xffffffff,
	0xffffffffff,
	0xffffffffffff,
	0xffffffffffffff,
	0xffffffffffffffff
};

/*
* 按照级别获取分类 id-> 常见于一个多级分类取其高级分类
* @para int l 分类的级别 (1-8)
* @para u64 id 分类ID
* return l级分类ID
* 如 58.02 -> getClassBylevel(1, cat) - > 58.00
* 如 58.02.11 -> getClassBylevel(2, cat) - > 58.02.00
*/
inline u64 GetClassByLevel(int l, u64 id)
{
	return id&g_classMark[l] ;
}

/*
* 将分类路径转换为ID
* @para   str 分类串
* @para   len 串长度
* @return 分类ID
* 58.12.13--> (5*16+8) + (1*16+2)<<8 + (1*16+3)<<16
*/
inline u64 TranseClsPath2ID(const char* str ,int len)
{
	vector<char> vec(str, str+len);
	vec.erase(remove(vec.begin(), vec.end(), '.'), vec.end());
	for (size_t i = 0; i < vec.size(); ++i)
	{
		if (vec[i] >'9' || vec[i] < '0')
		{
			return 0;
		}
	}
	len = vec.size();
	if (len == 0 || len&1 || len >16)
	{
		return 0;
	}

	u64 t = 0;
	unsigned char* p = (unsigned char*)&vec[0];
	for (int i = 0; i < len; i+=2)
	{
		t += ((u64)( ((p[i] - '0')<<4) + (p[i+1] - '0') ) << ((i>>1)<<3));
	}
	return t;
}

/*
* 将分类ID转换为路径
* @para   buf 分类串输出缓冲
* @para   id 分类ID
* @para   level 想要展示的路径级别（1-8）
* @return VOID
* 58.12.13 level=1 则显示58
* 58.12.13 level=3 则显示58.12.13
* 58.12.13 level=6 则显示58.12.13.00.00.00
*/
inline void TranseID2ClsPath(char* buf,  u64 id,  int level)
{
	unsigned char c;
	int j = 0;
	for (int i = 0; i < level; ++i)
	{
		c = id & 0xff;
		id >>= 8;
		buf[j++] = (c>>4) + '0';
		buf[j++] = (c & 0xf) + '0';
		if (i!=level-1)
			buf[j++] = '.';
	}
	buf[j]=0;
}

/*
*获取分类的级别
* @para   id 分类ID
* @return 分类级别
*/
inline int GetClsLevel(u64 id)
{
	int level = 0;
	for (; id & 0xff; id>>=8)
	{
		++level;
	}

	return level;
}

inline int GetClsLevelEx(u64 id)
{
	double d=(double)id;
	return ((((((unsigned short*)&d)[3] & 0x7ff0) >>4) -1023) >>3 ) + 1;
}


template<class T>
inline bool CmpClassTmp(const pair<T, int> &l, const pair<T, int> &r)
{
	int res = GetClsLevel((u64)l.first) - GetClsLevel((u64)r.first);

	return res < 0 || 
		res == 0 && l.second > r.second;
}

inline bool CmpClassLexical(unsigned long long l, unsigned long long r)
{
	return lexicographical_compare((unsigned char*)&l, (unsigned char*)&l+8, (unsigned char*)&r, (unsigned char*)&r+8);
} 

/*
*将分类ID按照级别，转换成多个ID,有多少级将会产生多个
* @para   id 分类ID
* @para   vIds 分类ID 数组
* @return void
* 58.12.22--->
*            58
*            58.12
*            58.12.22
*/
inline void TranseClsID2ClsIDs(vector<u64>& vIds,  u64 id)
{
	u64 t = 0;
	u64 c;
	for (int i = 0; i < 8 ; ++i,id>>=8)
	{
		c = id & 0xff;
		if (c == 0)
		{
			break;
		}
		t += (c << (i<<3));
		vIds.push_back(t);
	}
}


//以下三函数不建议外部使用



inline void TransePathes2ClsIds(const char* ptr, vector<u64>& vecClsIds, const char* const spliter = "|,")
{
	vector<string> vecKeys;
	vector<char> vecBuf;
	vecBuf.assign(ptr, ptr+strlen(ptr)+1);
	vecBuf.back() = 0;
	SplitToVecEx(&vecBuf[0], vecKeys, spliter);
	u64 t;
	for (size_t i = 0; i < vecKeys.size(); ++i)
	{
		t=TranseClsPath2ID(vecKeys[i].c_str(), vecKeys[i].length());
		TranseClsID2ClsIDs(vecClsIds, t);
	}
	sort(vecClsIds.begin(), vecClsIds.end(),CmpClassLexical);
	vecClsIds.erase(unique(vecClsIds.begin(),vecClsIds.end()),vecClsIds.end());
}


inline void TransePathes2ClsIds(const char* ptr, string& str, const char* const spliter = "|,")
{
	vector<string> vecKeys;
	vector<char> vecBuf;
	vector<u64>  vecClsIds;
	vecBuf.assign(ptr, ptr+strlen(ptr)+1);
	vecBuf.back() = 0;
	SplitToVecEx(&vecBuf[0], vecKeys, spliter);
	u64 t;
	for (size_t i = 0; i < vecKeys.size(); ++i)
	{
		t=TranseClsPath2ID(vecKeys[i].c_str(), vecKeys[i].length());
		TranseClsID2ClsIDs(vecClsIds, t);
	}
	sort(vecClsIds.begin(), vecClsIds.end(),CmpClassLexical);
	vecClsIds.erase(unique(vecClsIds.begin(),vecClsIds.end()),vecClsIds.end());
	char buf[64];
	for (size_t i = 0;i < vecClsIds.size(); ++i)
	{
		sprintf(buf, "%lld", vecClsIds[i]);
		str += buf;
		str += ',';
	}
	ptr = str.c_str();
}

inline void TransePathes2ClsIdsNoExpend(const char* ptr, string& str, const char* const spliter = "|,")
{
	vector<string> vecKeys;
	vector<char> vecBuf;
	vector<u64>  vecClsIds;
	vecBuf.assign(ptr, ptr+strlen(ptr)+1);
	vecBuf.back() = 0;
	SplitToVecEx(&vecBuf[0], vecKeys, spliter);
	u64 t;
	bool bContain;
	size_t i,j;
	for (i = 0; i < vecKeys.size(); ++i)
	{
		t=TranseClsPath2ID(vecKeys[i].c_str(), vecKeys[i].length());
		bContain=false;
		for(j=0;j<vecClsIds.size();++j)
		{
			if (t==vecClsIds[j])
			{
				bContain=true;
				break;
			}
		}

		if (!bContain)
			vecClsIds.push_back(t);
	}

	char buf[64];
	for (size_t i = 0;i < vecClsIds.size(); ++i)
	{
		sprintf(buf, "%lld", vecClsIds[i]);
		str += buf;
		str += ',';
	}
	ptr = str.c_str();
}
#endif
