#ifndef _KEY_DICTIONARY_H_
#define _KEY_DICTIONARY_H_
#include <hash_map>
#include <string>

//#include "data_type.h"
using namespace std;
#ifdef _WIN32
using namespace stdext;
#endif

// 关键词字典
typedef hash_map<string, int> table;

class CKeyDictionary
{
private:

	// 字典哈希表
	table KeyDictionary;
	//KEYID->KEYS 下标为KEYID。
	vector<string> vecKeys;

public:
	CKeyDictionary()
	{
	}

	~CKeyDictionary()
	{	
		//Clear();		
	}
	void Clear()
	{
		//KeyDictionary.swap(table());
		KeyDictionary.clear();
		//vecKeys.swap(vector<string>());
		vecKeys.clear();
			
	}



	bool Save(const char *fileFullName)
	{
		FILE *fp = fopen(fileFullName, "w");
		if (fp == NULL)
		{
			// 			clsLog::Log2Server(logError, 
			// 				"clsKeyDictionary::Save出错，未能存储指定文件%s\n", 
			// 				fileFullName);

			return false;
		}
		table::iterator it;
		try
		{
			int keyID = -1;
			char lineInfo[1024];
			for (it=KeyDictionary.begin(); it!=KeyDictionary.end(); it++)
			{
				keyID = (*it).second;
				// fprintf(fp, "%s %d\n", (*it).first, keyID);

				//BUG: 增长了长度判断,以免分出来的词太长,超出缓冲区长度,会导致程序崩溃
				//     查这个问题花了一下午加一晚上的时间
				//fixed at 2006.6.1   mok.  
				if( (*it).first.length()>1000 )
				{
					fprintf(fp,"遇到太长的词,跳过\n");
					continue;
				}
				sprintf(lineInfo, "%s %d\n", (*it).first.c_str(), keyID);
				fputs(lineInfo, fp);
			}
			fflush(fp);
			fclose(fp);
			fp = NULL;
		}
		catch (...)
		{
			// 			clsLog::Log2Server(logError, 
			// 				"保存key字典失败: %s\n", fileFullName);
			return false;
		}
		return true;
	}


	bool Load(const char *fileFullName)
	{
		// 0.出错处理
		if (fileFullName == NULL)
		{
			//clsLog::Log2Server(logError, "ERROR: CKeyDictionary::Load函数出错。fileFullName为空\n");
			//Clear();
			return false;
		}
		FILE *pFile = fopen(fileFullName, "r");
		if (pFile == NULL)
		{
			//clsLog::Log2Server(logError, "ERROR: CKeyDictionary::Load函数出错。打开%s文档出错\n", fileFullName);
			//Clear();
			return false;
		}


		// 2.填入数据
		char chBuffer[1024];
		char word[100];
		int nIndex;
		while (fgets(chBuffer, 1024, pFile) != NULL)
		{
			sscanf(chBuffer, "%s %d", word, &nIndex);
			KeyDictionary[word] = nIndex;	
		}

		int nMaxKeyId=0;
		for (table::iterator i=KeyDictionary.begin();i!=KeyDictionary.end();++i)
		{
			if (i->second>nMaxKeyId)
			{
				nMaxKeyId=i->second;
			}			
		}
		vecKeys.resize(nMaxKeyId+1,"");
		for (table::iterator i=KeyDictionary.begin();i!=KeyDictionary.end();++i)
		{
			vecKeys[i->second]=i->first;			
		}

		fclose(pFile);
		pFile = NULL;
		return true;
	}
	inline	const string& GetKeysFromID(int nID)
	{
		return vecKeys.at(nID);
	}

	inline 	int GetKeyID(const char *word)
	{
		if (!word || strlen(word)<=0)
		{
			return -1;
		}
		table::const_iterator it;
		it = KeyDictionary.find(word);
		if (it != KeyDictionary.end())
		{
			return (*it).second;
		}
		return -1;		
	}

	int Get_Add_KeyID(const char *word)
	{	
		if (!word || strlen(word)<=0)
		{
			return -1;
		}
		else
		{		
			table::iterator it = KeyDictionary.find(word);
			if (it != KeyDictionary.end())
			{
				return (*it).second;
			}
			else
			{
				vecKeys.push_back(word);
				return (KeyDictionary[word]=vecKeys.size()-1);					
			}
		}	
	}
};

#endif
