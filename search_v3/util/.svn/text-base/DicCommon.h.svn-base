#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <set>
#include <algorithm>
#include <cstring>
#include "hash_wrap.h"
using namespace std;
#ifdef _WIN32
using namespace stdext;
#endif
#define  MAX_LEN 64


typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned char  BYTE;

struct TREE_NODE
{
	BYTE bEnd;
	BYTE bLen;
	WORD wData;	
	WORD wSubCnt;
	WORD wSubBeg;

	//char chBuf[8];
	//byte0:到本节点是否可以为词：0不是词、 1是一个词；
	//byte1:本节点数据长度 ：0为一个字节，1为两个字节
	//byte2:数据第一个字节；byte2为0时无效
	//byte3:数据第二个字节；

	//byte4:本节点包含子节点数；
	//byte5:
	//byte6:第一个子节点起始索引
	//byte7:



};
struct TREE_NODE_W
{
	BYTE bEnd;
	BYTE bLen;
	WORD wData;	
	WORD wSubCnt;
	WORD wSubBeg;
	DWORD dwWeight;
	//char chBuf[8];
	//byte0:到本节点是否可以为词：0不是词、 1是一个词；
	//byte1:本节点数据长度 ：0为一个字节，1为两个字节
	//byte2:数据第一个字节；byte2为0时无效
	//byte3:数据第二个字节；

	//byte4:本节点包含子节点数；
	//byte5:
	//byte6:第一个子节点起始索引
	//byte7:



};
struct KEY_WORD
{
	WORD word[MAX_LEN];
};
struct KEY_WORD_W
{
	WORD word[MAX_LEN];
	DWORD dwWeight;
};



struct  TEST_KEY
{
	char chBuf[MAX_LEN*2];
};

