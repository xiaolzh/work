
#ifndef FILEOPERATION_H
#define FILEOPERATION_H
//keys Speparate with \r\n 
#include <string>
#include <vector>
#include <iostream>
#include <cstdio>
#include <typeinfo>
#include <errno.h>
#include "hash_wrap.h"
using namespace std;
#ifdef _WIN32
using namespace stdext;
#endif


#define REPLACE_CHAR ','

template<class T,size_t size>
struct	STRING_T
{	
	T weight;
	char chBuf[size];
};

template<class K,class V>
bool Load2PairFile_V(const char* pchFileName,vector<pair<K,V> > &vec)
{
	FILE *fpIn=fopen(pchFileName,"rb");

	if(!fpIn)
	{
		//ErrorLog("���ļ�ʧ��! ����λ��: %s,%d",__FILE__,__LINE__);
		fclose(fpIn);
		return false;
	}
	char chBuf[255];
	char chBuf1[96];
	char chBuf2[96];
	int n;
	int nNum;
	while(!feof(fpIn))
	{
		if(!fgets(chBuf,255,fpIn))
			break;
		n=strlen(chBuf);
		while(n>0 && (chBuf[n-1]=='\r' || chBuf[n-1]=='\n')) 
			chBuf[(n--)-1]='\0';
		sscanf(chBuf,"%s%s",chBuf1,chBuf2);
		K k;
		if (typeid(K)==typeid(string))
		{
			k=(K)chBuf1;
		}
		else if (typeid(K)==typeid(int))
		{
			nNum=atoi(chBuf1);
			k=*(K*)&(nNum);
		}	
		V v;
		if (typeid(V)==typeid(string))
		{
			v=(V)chBuf2;
		}
		else if (typeid(V)==typeid(int))
		{
			nNum=atoi(chBuf2);
			v=*(V*)&(nNum);
		}	
		vec.push_back(make_pair(k,v));

	}
	fclose(fpIn);	
	return true;
}

template<class KEY>
inline KEY PrepareKR(char*buf)
{
	return (KEY)atoll(buf);
}
template<>
inline string PrepareKR<string>(char*buf)
{
	char* p=buf;
	while(buf&&*buf)
	{
		if(*buf<0)
		{
			++buf;
			if(!*buf)break;
			++buf;
		}
		else
		{
			if(*buf==REPLACE_CHAR)
				*buf=' ';
			++buf;
		}
	}
	return p;
}

template<class KEY>
inline KEY PrepareK(char*buf)
{
	return (KEY)atoll(buf);
}
template<>
inline string PrepareK<string>(char*buf)
{
	return buf;
}

template<class K,class V>
bool Load2PairFileR(const char* pchFileName,hash_map<K,V> &hm)
{
	FILE *fpIn=fopen(pchFileName,"rb");

	if(!fpIn)
	{
		//ErrorLog("���ļ�ʧ��! ����λ��: %s,%d",__FILE__,__LINE__);
		fclose(fpIn);
		return false;
	}
	char chBuf[1024];
	char chBuf1[1024];
	char chBuf2[1024];
	int n;
	while(!feof(fpIn))
	{
		if (!fgets(chBuf,255,fpIn))
			break;
		n=strlen(chBuf);
		while(n>0 && (chBuf[n-1]=='\r' || chBuf[n-1]=='\n')) 
			chBuf[(n--)-1]='\0';
		sscanf(chBuf,"%s%s",chBuf1,chBuf2);
		K k=PrepareKR<K>(chBuf1);
		V v=PrepareKR<V>(chBuf2);
		hm[k]=v;
	}
	fclose(fpIn);	
	return true;
}


template<class K,class V>
bool Load2PairFile(const char* pchFileName,hash_map<K,V> &hm)
{
	FILE *fpIn=fopen(pchFileName,"rb");

	if(!fpIn)
	{
		//ErrorLog("���ļ�ʧ��! ����λ��: %s,%d",__FILE__,__LINE__);
		fclose(fpIn);
		return false;
	}
	char chBuf[1024];
	char chBuf1[1024];
	char chBuf2[1024];
	int n;
	while(!feof(fpIn))
	{
		if (!fgets(chBuf,255,fpIn))
			break;
		n=strlen(chBuf);
		while(n>0 && (chBuf[n-1]=='\r' || chBuf[n-1]=='\n')) 
			chBuf[(n--)-1]='\0';
		sscanf(chBuf,"%s%s",chBuf1,chBuf2);
		K k=PrepareK<K>(chBuf1);
		V v=PrepareK<V>(chBuf2);
		hm[k]=v;
	}
	fclose(fpIn);	
	return true;
}

template<class T>
inline void ToString(char *pBuf,const T& t)
{
	sprintf(pBuf,"%lld",(long long)t);
}

template<>
inline void ToString<string>(char *pBuf,const string& t)
{
	sprintf(pBuf,"%s",t.c_str());
}

template<class T>
inline unsigned int ToUInt(const T& t)
{
	return (unsigned int)t;
}

template<>
inline unsigned int ToUInt<string>(const string& t)
{
	return 0;
}

template<class T>
inline void ToStringR(char *pBuf,const T& t)
{
	sprintf(pBuf,"%lld",(long long)t);
}
template<>
inline void ToStringR<string>(char *pBuf,const string& t)
{
	const char* pSrc = t.c_str();
	
	while(*pSrc)
	{
		
		if(*pSrc<0)
		{
			if(!*(pSrc+1))
				break;
			*pBuf=*pSrc;
			++pBuf;++pSrc;
			*pBuf=*pSrc;
			++pBuf;++pSrc;
		}
		else
		{
			if(isspace(*pSrc))
				*pBuf=REPLACE_CHAR;
			else
				*pBuf=*pSrc;
			++pBuf;++pSrc;
		}
	}
	*pBuf='\0';
}


template<class K,class V>
bool Write2PairFile(const char* pchFileName,hash_map<K,V> &hm)
{
	FILE *fpOut=fopen(pchFileName,"wb");

	if(!fpOut)
	{
		//ErrorLog("�� %s �ļ�ʧ��! ����λ��: %s,%d",pchFileName,__FILE__,__LINE__);
		fclose(fpOut);
		return false;
	}
	char chKey[1024];
	char chVal[1024];
	for (typename hash_map<K,V>::iterator it=hm.begin();it!=hm.end();++it)
	{
		ToString(chKey,it->first);
		ToString(chVal,it->second);
		fprintf(fpOut,"%s %s\n",chKey,chVal);
	}
	fclose(fpOut);
	return	true;
}

template<class K,class V>
bool Write2PairFileR(const char* pchFileName,hash_map<K,V> &hm)
{
	FILE *fpOut=fopen(pchFileName,"wb");

	if(!fpOut)
	{
		//ErrorLog("�� %s �ļ�ʧ��! ����λ��: %s,%d",pchFileName,__FILE__,__LINE__);
		fclose(fpOut);
		return false;
	}
	char chKey[1024];
	char chVal[1024];
	for (typename hash_map<K,V>::iterator it=hm.begin();it!=hm.end();++it)
	{
		ToStringR(chKey,it->first);
		ToStringR(chVal,it->second);
		fprintf(fpOut,"%s %s\n",chKey,chVal);
	}
	fclose(fpOut);
	return	true;
}






template<class T>
bool LoadKeysFile(const char* pchFileName,vector<T> &vec)
{
	FILE *fpIn=fopen(pchFileName,"rb");

	if(!fpIn)
	{
		//ErrorLog("���ļ�ʧ��! ����λ��: %s,%d",__FILE__,__LINE__);
		return false;
	}
	char chBuf[255];
	int n;
	while(!feof(fpIn))
	{
		if(!fgets(chBuf,255,fpIn))
			break;
		n=strlen(chBuf);
		while(n>0 && (chBuf[n-1]=='\r' || chBuf[n-1]=='\n')) 
			chBuf[(n--)-1]='\0';

		T t=PrepareK<T>(chBuf);
		vec.push_back(t);
	}
	fclose(fpIn);	
	return true;
}

template<class T>
bool LoadKeysFile(const char* pchFileName,hash_set<T> &hs)
{
	FILE *fpIn=fopen(pchFileName,"rb");
	if(!fpIn)
	{
		//ErrorLog("���ļ�ʧ��! ����λ��: %s,%d",__FILE__,__LINE__);
		return false;
	}
	char chBuf[255];
	int n;
	while(!feof(fpIn))
	{
		if(!fgets(chBuf,255,fpIn))break;
		n=strlen(chBuf);
		while(n>0 && (chBuf[n-1]=='\r' || chBuf[n-1]=='\n')) 
			chBuf[(n--)-1]='\0';
		T t=PrepareK<T>(chBuf);
		hs.insert(t);
	}
	fclose(fpIn);	
	return true;		
}


//���� STRING->INT �ṹ�ļ� �����ϣ��
bool LoadKeysCnts(const char* pchFileName,hash_map<string,int> &hmStrCnt);

//����ṹ bHead ��ʾ�Ƿ��е�һ��������ʶ�ṹ��С ��
template<class T>
bool LoadStruct(const char*pchFileName,vector<T>& vecTemplate,bool bHead=true);
//д��ṹ
template<class T>
bool WriteStruct(const char*pchFileName,vector<T>& vecTemplate,bool bHead=true);

//��HASH_MAP string_Tд���ļ���
template<class T,size_t size>
bool WriteHash_StringT(hash_map<string,T>& hmStringT,const char*pchFileName);

//��HASH_MAP string_T�����ļ���
template<class T,size_t size>
bool LoadHash_StringT(hash_map<string,T>& hmStringT,const char*pchFileName);


template<class T,size_t size>
bool WriteHash_StringT(hash_map<string,T>& hmStringT,const char*pchFileName)
{

	//��������

	vector<STRING_T<T,size> > vecStringT;
	vecStringT.reserve(hmStringT.size());
	STRING_T<T,size> stringT;
	typename hash_map<string,T>::iterator i;
	for (i=hmStringT.begin();i!=hmStringT.end();++i)
	{
		strcpy(stringT.chBuf,i->first.c_str());
		stringT.weight=i->second;
		vecStringT.push_back(stringT);
	}
	//д���ļ�
	if (!WriteStruct(pchFileName,vecStringT))
	{
		return false;
	}
	return true;

}
//��HASH_MAP string_T�����ļ���
template<class T,size_t size>
bool LoadHash_StringT(hash_map<string,T>& hmStringT,const char*pchFileName)
{

	vector<STRING_T<T,size> > vecStringT;

	if (!LoadStruct(pchFileName,vecStringT))
	{
		return false;
	}

	for (int i=0;i<vecStringT.size();++i)
	{
		hmStringT.insert(make_pair(vecStringT[i].chBuf,vecStringT[i].weight));		
	}
	return true;


}


template<class T>
bool LoadStruct(const char*pchFileName,vector<T>& vecTemplate,bool bHead)
{
	FILE *fpIn=fopen(pchFileName,"rb");

	if(!fpIn)
	{
		//ErrorLog("�� %s �ļ�ʧ��! ����λ��: %s,%d",pchFileName,__FILE__,__LINE__);
		fprintf(stderr,"file:%s , line: %d,filename:%s, error info: %s\n",__FILE__,__LINE__,pchFileName,strerror(errno));
		return false;
	}
	size_t nSize=0;
	if (bHead)
	{
		fread(&nSize,sizeof(int),1,fpIn);	
	}
	else
	{
		fseek(fpIn,0,SEEK_END);
		nSize=ftell(fpIn)/sizeof(T);
		fseek(fpIn,0,SEEK_SET);
	}	
	vecTemplate.resize(nSize);
	if (vecTemplate.size()&&fread(&vecTemplate[0],sizeof(T),nSize,fpIn)!=nSize)
	{
		//ErrorLog("��ȡ�ļ�%sʧ��",pchFileName);
		fprintf(stderr,"file:%s , line: %d,filename:%s, error info: %s\n",__FILE__,__LINE__,pchFileName,strerror(errno));
		fclose(fpIn);
		return false;
	}
	fclose(fpIn);
	return	true;

}
	template<class T>
bool WriteStruct(const char*pchFileName,vector<T>& vecTemplate,bool bHead)
{
	FILE *fpOut=fopen(pchFileName,"wb");

	if(!fpOut)
	{
		//ErrorLog("�� %s �ļ�ʧ��! ����λ��: %s,%d",pchFileName,__FILE__,__LINE__);
		fprintf(stderr,"file:%s , line: %d,filename:%s, error info: %s\n",__FILE__,__LINE__,pchFileName,strerror(errno));
		return false;
	}
	int nSize=vecTemplate.size();
	if (bHead)
	{		
		fwrite(&nSize,sizeof(int),1,fpOut);	
	}
	
	if (vecTemplate.size()&&fwrite(&vecTemplate[0],sizeof(T),nSize,fpOut)!=nSize)
	{
		//ErrorLog("д���ļ�%sʧ��",pchFileName);
		fclose(fpOut);
		return false;
	}
	fclose(fpOut);
	return	true;

}

template<class T>
inline void SplitToVecEx(char* pSrc, vector<T> &vec, const char* pchSplit)
{
	char* psave;
	char* pchCurWord=strtok_r(pSrc,pchSplit,&psave);
	while(pchCurWord)
	{
#ifdef _WIN32
		vec.push_back((T)_atoi64(pchCurWord));
#else
		vec.push_back((T)atoll(pchCurWord));
#endif
		pchCurWord = strtok_r(NULL, pchSplit,&psave);
	}

}
template<>
inline void SplitToVecEx<string>(char* pSrc, vector<string> &vec,  const char* pchSplit)
{
	char* psave;
	char* pchCurWord=strtok_r(pSrc,pchSplit,&psave);
	while(pchCurWord)
	{
		vec.push_back(pchCurWord);
		pchCurWord = strtok_r(NULL, pchSplit,&psave);
	}
}



template<class T>
inline void SplitToVec(char* pSrc, vector<T> &vec)
{
	char* psave;
	const char* pchSplit=",";	
	char* pchCurWord=strtok_r(pSrc,pchSplit,&psave);
	while(pchCurWord)
	{
#ifdef _WIN32
		vec.push_back((T)_atoi64(pchCurWord));
#else
		vec.push_back((T)atoll(pchCurWord));
#endif
		pchCurWord = strtok_r(NULL, pchSplit,&psave);
	}

}
template<>
inline void SplitToVec<string>(char* pSrc, vector<string> &vec)
{
	char* psave;
	const char* pchSplit=",";	
	char* pchCurWord=strtok_r(pSrc,pchSplit,&psave);
	while(pchCurWord)
	{
		vec.push_back(pchCurWord);
		pchCurWord = strtok_r(NULL, pchSplit,&psave);
	}
}




#endif
