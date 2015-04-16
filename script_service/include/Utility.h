#ifndef _UTILITY_H_
#define _UTILITY_H_
#include "UH_Define.h"
#include "publicfunction.h"
#define DOMAIN_NUM 60

const var_1 domains[DOMAIN_NUM][8] = { ".com", ".cn", ".com.cn", ".gov", ".net", ".edu.cn", ".net.cn", ".org.cn", ".co.jp", ".gov.cn",
	 									 ".co.uk", "ac.cn", ".edu", ".tv",".info", ".ac", ".ag", ".am", ".at", ".be", ".biz", ".bz",
										 ".cc", ".de", ".es", 
								".eu", ".fm", ".gs", ".hk", ".in", ".info", ".io", ".it", ".jp", ".la", ".md", ".ms", ".name", 
								".nl", ".nu", ".org", ".pl", ".ru", ".sc", ".se", ".sg", ".sh", ".tc", ".tk", ".tv", ".tw", ".us", ".co",
								".uk", ".vc", ".vg", ".ws", ".il", ".li", ".nz" };
bool static IsFolderEmpty(var_1 *folder)
{
	DIR *dirp;
	struct dirent *direntp;
	bool res = true;
	dirp = opendir(folder);
	if (dirp == NULL)
		return true;
	while((direntp=readdir(dirp)) != NULL)
	{
		if(strlen(direntp->d_name) == 2 && strncmp(direntp->d_name, "..", 2) == 0)
		{
			continue;
		}
		if(strlen(direntp->d_name) == 1 && strncmp(direntp->d_name, ".", 1) == 0)
		{
			continue;
		}
		res = false;
	}
	closedir(dirp);
}

bool static ClearFolder(var_1 *folder)
{
	var_4 iRet;
	DIR *dirp;
	struct dirent *direntp;
	bool res = true;
	dirp = opendir(folder);
	var_1 file[512];
	if (dirp == NULL)
		return true;
	while((direntp=readdir(dirp)) != NULL)
	{
		if(strlen(direntp->d_name) == 2 && strncmp(direntp->d_name, "..", 2) == 0)
		{
			continue;
		}
		if(strlen(direntp->d_name) == 1 && strncmp(direntp->d_name, ".", 1) == 0)
		{
			continue;
		}
		iRet = snprintf(file, 512, "%s/%s", folder, direntp->d_name);
		if (iRet <= 0)
		{
			res = false;
			break;
		}
		if(unlink(file) < 0)
		{
			res = false;
			break;
		}
	}
	closedir(dirp);
	return res;
}

template<class Type>
inline void FreeObj(Type &obj)
{
	if (obj)
	{
		free(obj);
		obj = NULL;
	}
	return;
}

template<class Type>
inline void DeleteObj(Type &obj)
{
	if (obj)
	{
		delete(obj);
		obj = NULL;
	}
	return;
}

inline void CloseFile(FILE *&fp)
{
	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}
	return;
}
inline void CloseFile(var_4 fh)
{
	if(fh != -1)
	{
		close(fh);
		fh = -1;
	}
	return;
}
template<class Type>
static var_4 SaveFile(var_1 *filename, Type obj, var_4 nCount, var_4 hasHead)
{
    FILE *fp = NULL;
    if (filename == NULL)
        return -1;
    fp = fopen(filename, "wb");
    if (fp == NULL)
        return -2;
	if (hasHead == 1) // first write the number of objs 
	{
		if (fwrite(&nCount, 4, 1, fp) != 1)
   	 	{
    	    CloseFile(fp);
       		return -3;
   		}
	}
    if (fwrite(obj, sizeof(*obj), nCount, fp) != nCount)
    {
        CloseFile(fp);
        return -4;
    }
    CloseFile(fp);
	return 0;
}

static var_4 LoadFile(var_1 *fileName, var_1 **fileBase)
{
    FILE *fp = NULL;
	if (fileName == NULL || fileBase == NULL)
		return -1;
    var_8 fileSize = 0;
	// open file
	fp = fopen(fileName, "rb");
	if (fp == NULL)
		return -2;
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	if (fread(*fileBase, fileSize, 1, fp) != 1)
	{
		CloseFile(fp);
		return -4;
	}
	return 0;
}

static var_4 LoadFile(var_1 *fileName, var_1 **fileBase, var_4 &fileHandle, var_8 &fileSize)
{
	if (fileName == NULL || fileBase == NULL)
	{
		return -1;
	}
	// close handles first
	if (fileHandle != -1)
	{
		CloseFile(fileHandle);
	}
	// open file
	struct stat st;
	fileHandle = open(fileName, O_RDWR | O_NOATIME, (mode_t)0644);
	if (fileHandle == -1)
	{
		return -2;
	}
	if (fstat(fileHandle, &st) != 0)
	{
		CloseFile(fileHandle);
		return -3;
	}
	fileSize = st.st_size;
	if (fileSize <= 0)
	{
		CloseFile(fileHandle);
		return -4;
	}
	// mmap file
	*fileBase = (char*)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_FILE|MAP_SHARED, fileHandle, 0);
	if (*fileBase == (char*)MAP_FAILED)
	{
		CloseFile(fileHandle);
		fileHandle = -1;
		return -5;
	}
	return 0;
}

static var_vd GetTagValue(var_1 *xml, const var_1 *tag_name, var_1 *tag_value)
{
	var_1 *sPos = 0, *ePos = 0;
	var_1 startTag[32] = {0};
	var_1 endTag[32]   = {0};
	snprintf(startTag, 32, "%s%s%s", "<", tag_name, ">");
	snprintf(endTag, 32, "%s%s%s", "</", tag_name, ">");
	memset(tag_value, 0, strlen(tag_value));
	sPos = strstr(xml, startTag);
	if (sPos != NULL)
	{
		ePos = strstr(sPos, endTag);
		if (ePos != NULL)
		{
			strncpy(tag_value, sPos + strlen(startTag), ePos - sPos - strlen(startTag));
		}
	}
	return;
}
// 例如原url：http://image.baidu.com/channel/wallpaper#%E7%A7%8B%E6%84%8F%E6%B5%93%E6%B5%93&&8&0
// 截断后的url: baidu.com
static var_4 GetDomain(var_1 *url, var_1 *domain)
{
	var_4 i, j, iLen=0;
	var_1 *sPos = 0, *ePos = 0;

	for (i = 0; i < DOMAIN_NUM; i++)
	{
		ePos = strstr(url, domains[i]);
		if (ePos != NULL)
		{
			break;
		}
	}
//	memset(domain, 0, strlen(domain));
//	find it	
	if (i != DOMAIN_NUM)
	{
		sPos = ePos;
		while (sPos != url)
		{
			sPos--;
			// 字母（ A～Z，a～z，大小写等）、数字（0～9）和连接符（－）
			if (*sPos == '.' || !((*sPos >= 'A' && *sPos <= 'Z')||(*sPos >= 'a' && *sPos <= 'z') || (*sPos >= '0' && *sPos <= '9')||*sPos == '-'))
			{
				break;
			}
		}
//		if (sPos != url)
		{
			iLen = ePos - sPos - 1 + strlen(domains[i]);
			strncpy(domain, sPos + 1, iLen);
			domain[iLen] = '\0';
		}
	}
	return iLen;
}

static var_4 CreateFile(var_1 *filename, var_1** fileBase, var_u8 fileSize, var_4 objSize)
{
	var_4 fileHandle = -1;
	if (filename == NULL)
	{
		return -1;
	}
	fileHandle = open(filename, O_RDWR | O_CREAT | O_TRUNC | O_NOATIME, (mode_t)0644);
	if (fileHandle == -1)
	{
		return -2;
	}
	if (lseek64(fileHandle, (off64_t)(fileSize - objSize), SEEK_SET) == -1)
	{
	    CloseFile(fileHandle);
	    return -3;
	}
	var_1* block = (var_1*)malloc(objSize);
	if (write(fileHandle, block, objSize) != objSize)
	{
	    CloseFile(fileHandle);
		FreeObj(block);
	    return -4;
	}
	if (read(fileHandle, *fileBase, fileSize) < 0)
	{
		CloseFile(fileHandle);
		FreeObj(block);
		return -5;
	}
	CloseFile(fileHandle);
	FreeObj(block);
	return 0;
}

static var_4 CreateFile(var_1 *filename, var_1**fileBase, var_4 &fileHandle, var_8 &fileSize, var_4 objSize)
{
	if (filename == NULL || fileBase == NULL)
	{
		return -1;
	}
    fileHandle = open(filename, O_RDWR | O_CREAT | O_TRUNC | O_NOATIME, (mode_t)0644);
	if (fileHandle == -1)
	{
		return -2;
	}
	// set filesize
	if (lseek64(fileHandle, (off64_t)(fileSize - objSize), SEEK_SET) == -1)
	{
	    CloseFile(fileHandle);
	    return -3;
	}
	var_1* block = (var_1*)malloc(objSize);	
	if (write(fileHandle, block, objSize) != objSize)
	{
	    CloseFile(fileHandle);
		FreeObj(block);
	    return -4;
	}
	*fileBase = (var_1*)mmap(NULL, fileSize, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, fileHandle, 0);
	if (*fileBase == (var_1*)MAP_FAILED)
	{
		CloseFile(fileHandle);
		FreeObj(block);
		return -5;
	}
	FreeObj(block);
	return 0;
}
static var_u8 GetTimeDiff(struct timeval begin, struct timeval end)
{
	struct timeval diff;
	if (end.tv_usec >= begin.tv_usec)
	{
		diff.tv_sec = end.tv_sec - begin.tv_sec;
		diff.tv_usec = end.tv_usec - begin.tv_usec;
	}
	else
	{
		diff.tv_sec = end.tv_sec - begin.tv_sec - 1;
		diff.tv_usec = 1000000 -begin.tv_usec + end.tv_usec;
	}
	return (diff.tv_sec * 1000000 + diff.tv_usec);
}
// 冒号和分号换成空格
static var_vd SymbolToSpace(var_1 *srcbuf, var_4 iLen)
{
	for (var_4 i = 0;  i < iLen; i++)
	{
		if (srcbuf[i] == ':' || srcbuf[i] == ';' || srcbuf[i] == '#')
		{
			srcbuf[i] = ' ';
		}
	}
	return;
}

static var_vd UpperToLower(var_1 *srcbuf, var_4 iLen)
{
	for (var_4 i = 0; i < iLen; i++)
	{
		if (srcbuf[i] <= 'Z' && srcbuf[i] >= 'A')
		{
			srcbuf[i] += 32;
		}
	}
	return;
}
#endif

