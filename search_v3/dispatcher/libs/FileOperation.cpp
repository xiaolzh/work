#include "FileOperation.h"



//���� STRING->INT �ṹ�ļ� �����ϣ��
bool LoadKeysCnts(const char* pchFileName,hash_map<string,int> &hmStrCnt)
{

	FILE *pFile = fopen(pchFileName, "rb");
	if (pFile == NULL)
	{		
		return false;
	}	

	char chBuffer[1024];
	char word[100];
	int nIndex;
	while (fgets(chBuffer, 1024, pFile) != NULL)
	{
		sscanf(chBuffer, "%s %d", word, &nIndex);		
		hmStrCnt[word] = nIndex;	
	}
	fclose(pFile);	
	return true;
}

//���� STRING->INT �ṹ�ļ� �����ֵ�&����ӳ�� (string ,int) and (int,int)
bool LoadKeysCntsEncode(const char* pchFileName,CKeyDictionary &keyDic,hash_map<int,int> &hmIdCnt)
{

	FILE *pFile = fopen(pchFileName, "rb");
	if (pFile == NULL)
	{		
		return false;
	}	

	char chBuffer[1024];
	char chWord[100];
	int nCnt;
	int nKID;
	while (fgets(chBuffer, 1024, pFile) != NULL)
	{
		sscanf(chBuffer, "%s %d", chWord, &nCnt);	
		nKID=keyDic.Get_Add_KeyID(chWord);		
		hmIdCnt[nKID]=nCnt;		
	}
	fclose(pFile);	
	return true;
}
