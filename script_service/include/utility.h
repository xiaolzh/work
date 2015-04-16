#ifndef ___UTILITY_H_
#define ___UTILITY_H_

#include <iostream>
#include <vector>
using namespace std;

#include "UH_Define.h"

template<typename T1, typename T2>
struct simple_pair
{
    T1 left;
    T2 right;
};

template<typename T1, typename T2>
bool operator==(const simple_pair<T1, T2>& lhs, const simple_pair<T1, T2>& rhs)
{
    if (lhs.left == rhs.left && lhs.right == rhs.right)
        return true;
    return false;
}

inline var_4 sys_error_code()
{
#ifdef _WIN32_ENV_
    return GetLastError();
#else
    return errno;
#endif
}

template<typename T>
inline var_vd _swap(T& lhs, T& rhs)
{
    T temp = lhs;
    lhs = rhs;
    rhs = temp;
}

inline var_1* strnstr(/*const */var_1* start, const var_1* end, var_1* pos, var_u4 len)
{
    if ((end - start) < len)
    {
        return NULL;
    }
    for (; end >= (start + len); ++start)
    {
        if (0 == strncmp(start, pos, len))
        {
            return start;
        }
    }
    return NULL;
}

template<typename T>
var_vd _quick_sort(T* _term_infos, var_4 _low, var_4 _high, bool _cmp(const T&, const T&))
{
    assert(NULL != _term_infos);
    if (_low >= _high)
    {   // length equal to 1 or ...
        return;
    }

    var_4 mid = _low + ((_high - _low ) >> 1);
    if (_low <= (_high - 2))
    {   // length more than 3
        if (_cmp(_term_infos[_low], _term_infos[mid]))
        {
            _swap(_term_infos[_low], _term_infos[mid]);
        }
        if (_cmp(_term_infos[_low], _term_infos[_high]))
        {
            _swap(_term_infos[_low], _term_infos[_high]);
        }
        if (_cmp(_term_infos[mid], _term_infos[_high]))
        {
            _swap(_term_infos[mid], _term_infos[_high]);
        }
        if (_low == (_high - 2))
        {// é•¿åº¦ä¸º3
            return;
        }
    }
    else
    {// é•¿åº¦ä¸º2
        if (_cmp(_term_infos[_low], _term_infos[_high]))
        {
            _swap(_term_infos[_low], _term_infos[_high]);
        }
        return;
    }
    T pivot = _term_infos[mid];
    var_4 i = _low, j = _high;
    while (i <= j)
    {
        while (_cmp(_term_infos[j], pivot))
        {
            --j;
        }
        while (_cmp(pivot, _term_infos[i]))
        {
            ++i;
        }
        if (i <= j)
        {
            _swap(_term_infos[i], _term_infos[j]);
            --j;
            ++i;
        }
    }

    if (_low < j)
    {
        _quick_sort(_term_infos, _low, j, _cmp);
    }
    if (_high > i)
    {
        _quick_sort(_term_infos, i, _high, _cmp);
    }
}

template<typename T>
class static_array
{
public:
    static_array() {}
	
    ~static_array() {}

    var_vd reset()
    {
        m_pointer = NULL;
        m_size = 0u;
        m_capacity = 0u;
    }

    T* m_pointer;
    var_u4  m_size;
    var_u4  m_capacity;
};

static var_4 get_tag(var_1 *xml, var_4 xml_len, const var_1 *tag_name, var_1 *tag_value, var_1 if_cdata)
{
	var_1 *spos = 0;
	var_1 *epos = 0;
	var_1 start_tag[32] = {0};
	var_1 end_tag[32]   = {0};
	var_4 len = 0;
	if (if_cdata == 0)
	{   
		sprintf(start_tag, "%s%s%s", "<", tag_name, ">");
		sprintf(end_tag, "%s%s%s", "</", tag_name, ">");
	}   
	else
	{   
		sprintf(start_tag, "%s%s%s", "<", tag_name, "><![CDATA[");
		sprintf(end_tag, "%s%s%s", "]]></", tag_name, ">");
	}   
	spos = strstr(xml, start_tag);
	if (spos != NULL && spos - xml < xml_len)
	{   
		epos = strstr(spos, end_tag);
		if (epos != NULL && epos - xml < xml_len)
		{   
			len = epos - spos - strlen(start_tag);
			strncpy(tag_value, spos + strlen(start_tag), len);
		}   
	}
	tag_value[len] = 0x0;
	return len;
}

static var_4 get_tag_pos(var_1 *xml, var_4 xml_len, const var_1 *tag_name, var_4& tag_off, var_4& tag_len, var_1 if_cdata)
{
	var_4 ret = -1;
	var_1 *spos = 0;
	var_1 *epos = 0;
	var_1 start_tag[32] = {0};
	var_1 end_tag[32]   = {0};
	if (if_cdata == 0)
	{   
		sprintf(start_tag, "%s%s%s", "<", tag_name, ">");
		sprintf(end_tag, "%s%s%s", "</", tag_name, ">");
	}   
	else
	{   
		sprintf(start_tag, "%s%s%s", "<", tag_name, "><![CDATA[");
		sprintf(end_tag, "%s%s%s", "]]></", tag_name, ">");
	}   
	spos = strstr(xml, start_tag);
	if (spos != NULL && spos - xml < xml_len)
	{   
		epos = strstr(spos, end_tag);
		if (epos != NULL && epos - xml < xml_len)
		{   
			tag_off = spos - xml + strlen(start_tag);
			tag_len = epos - spos - strlen(start_tag);
			ret = 0;
		}
	}   
	return ret;
}

// parameter type: char * with offset and len
static var_4 get_xml_tag(var_1 *xml, var_4 xml_len, const var_1 *tag_name, var_4& tag_off, var_4& tag_len, var_1 if_cdata)
{
	return get_tag_pos(xml, xml_len, tag_name, tag_off, tag_len, if_cdata);
}

// parameter type: char *
static var_4 get_xml_tag(var_1 *xml, var_4 xml_len, const var_1 *tag_name, var_1 *tag_value, var_1 if_cdata)
{
	return get_tag(xml, xml_len, tag_name, tag_value, if_cdata);
}

// parameter type: int
static var_4 get_xml_tag(var_1 *xml, var_4 xml_len, const var_1 *tag_name, var_4 &tag_value, var_1 if_cdata)
{
	var_1 value[11] = {0};
	var_1 ret = get_tag(xml, xml_len, tag_name, value, if_cdata);
	if (ret > 0)
	{
		tag_value = (var_4)atol(value);
		return 0;
	}
	return -1;
}

// parameter type: unsigned int
static var_4 get_xml_tag(var_1 *xml, var_4 xml_len, const var_1 *tag_name, var_u4 &tag_value, var_1 if_cdata)
{
	var_1 value[11] = {0};
	var_4 ret = get_tag(xml, xml_len, tag_name, value, if_cdata);
	if (ret > 0)
	{
		tag_value = (var_u4)atol(value);
		return 0;
	}
	return -1;
}
// parameter type: unsigned long long
static var_4 get_xml_tag(var_1 *xml, var_4 xml_len, const var_1 *tag_name, var_u8 &tag_value, var_1 if_cdata)
{
	var_1 value[21] = {0};
	var_4 ret = get_tag(xml, xml_len, tag_name, value, if_cdata);
	if (ret > 0)
	{
		tag_value = strtoull(value, NULL, 10);
		return 0;
	}
	return -1;
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
		iRet = sprintf(file,"%s/%s", folder, direntp->d_name);
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

static int SegmentToSrc(char *pDest, const char *pSrc, int iSrcLen)
{
    const char *p = pSrc;
    const char *pEnd = pSrc+iSrcLen;
    char *q = pDest;

    bool bFindWord = false;
    while (p < pEnd)
    {
        if (*p == ' ')
        {
            if (bFindWord)
            {//ÔÚ¿Õ¸ñÇ°ÊÇµ¥´Ê£¬±¾¿Õ¸ñÊÇ¼ä¸ô·û£¬Ìø¹ý
                p++;
                bFindWord = false;
            }
            else
            {//±¾¿Õ¸ñ¾ÍÊÇµ¥´Ê£¬¸´ÖÆ
                *q++ = *p++;
                bFindWord = true;
            }            
        }
        else if (*p < 0)
        {//CN
            *q++ = *p++;
            if (p < pEnd)
            {
                *q++ = *p++;
            }
            bFindWord = true;
        }
        else
        {
            *q++ = *p++;
            bFindWord = true;
        }
    }
    return q-pDest;
}

static int MergeMultiLine(char *pDest, const char *pSrc, int iSrcLen)
{
    const char *p = pSrc;
    const char *pEnd = pSrc+iSrcLen;
    char *q = pDest;

    bool bFindLine = false;
    while (p < pEnd)
    {
        if (*p == '\n')
        {
            if (bFindLine)
            {//ÔÚ»»ÐÐÇ°ÊÇ»»ÐÐ£¬Ìø¹ý
                p++;
            }
            else
            {//±¾»»ÐÐÊÇµÚÒ»¸ö£¬¸´ÖÆ
                *q++ = *p++;
            }            
			bFindLine = true;
        }
		else if (*p == ' ')
		{//copy, but don't change bFindLine value
            *q++ = *p++;
		}
        else if (*p < 0)
        {//CN
            *q++ = *p++;
            if (p < pEnd)
            {
                *q++ = *p++;
            }
            bFindLine = false;
        }
        else
        {
            *q++ = *p++;
            bFindLine = false;
        }
    }
    return q-pDest;
}

/*
static var_vd RemoveFile(const var_1 * const lpszFileName)
{
	if (lpszFileName == NULL)
		return;

	var_u4 nCount=0;
	while (FileExists(lpszFileName) && remove(lpszFileName)!=0)
	{
		cp_sleep(1);

	}
}*/

#endif


