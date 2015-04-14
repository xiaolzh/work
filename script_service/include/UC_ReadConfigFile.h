// ReadConfigFile.h

#ifndef _UC_READ_CONFIG_FILE_H_
#define _UC_READ_CONFIG_FILE_H_

#include "UH_Define.h"

#define UCRCF_SM_1	' '		// separation mark
#define UCRCF_SM_2	'\t'	// separation mark

#define UCRCF_COMMENT_1		';'		// comment mark
#define UCRCF_COMMENT_2		'#'		// comment mark

class UC_ReadConfigFile
{
public:
	// 构造, 析构函数
	UC_ReadConfigFile()
	{
		m_cpFile = NULL;
	}	
	~UC_ReadConfigFile()
	{
		if(m_cpFile)
			fclose(m_cpFile);
	}
	// 初始化配置文件
	var_4 InitConfigFile(const var_1* filename)
	{
		m_cpFile = fopen(filename, "r");
		if(m_cpFile == NULL)
			return -1;

		return 0;
	}
	// 取字符串类型参数
	var_4 GetFieldValue(const var_1* szpField, var_1* szpValue)
	{
		var_1 szaReadBuf[1024];
		
		fseek(m_cpFile, 0, SEEK_SET);
		
		while(fgets(szaReadBuf, 1024, m_cpFile))
		{
			cp_drop_useless_char(szaReadBuf);

			var_1* p = szaReadBuf;
			
			while(*p && (*p == UCRCF_SM_1 || *p == UCRCF_SM_2))
				p++;
			
			if(*p == 0 || *p == UCRCF_COMMENT_1 || *p == UCRCF_COMMENT_2)
				continue;

			var_1* tag_beg = p;
			var_1* tag_end = strchr(tag_beg, '=');
			
			if(tag_end == NULL)
				return -1;
			
			p = tag_end--;

			while(tag_end > tag_beg && (*tag_end == UCRCF_SM_1 || *tag_end == UCRCF_SM_2))
				tag_end--;
			
			*(++tag_end) = 0;

#ifdef _WIN32_ENV_
			if(_stricmp(szpField, tag_beg))
#else
			if(strcasecmp(szpField, tag_beg))
#endif		
				continue;
			
			p++;
			while(*p && (*p == UCRCF_SM_1 || *p == UCRCF_SM_2))
				p++;
			
			if(*p == 0)
				return -1;
			
			var_1* val_beg = p;
			var_1* val_end = val_beg;
			
			if(*val_beg == '\'' || *val_beg == '\"')
			{				
				val_end++;
				
				while(*val_end && *val_end != *val_beg)
					val_end++;
				
				if(*val_end == 0)
					return -1;
				
				val_beg++;
			}
			else
			{
				while(*val_end && *val_end != UCRCF_SM_1 && *val_end != UCRCF_SM_2 && *val_end != UCRCF_COMMENT_1 && *val_end != UCRCF_COMMENT_2)
					val_end++;
			}

			memcpy(szpValue, val_beg, val_end - val_beg);
			szpValue[val_end - val_beg] = 0;

			return 0;
		}
		
		return -1;
	}
	// 取长整形类型参数
	var_4 GetFieldValue(const var_1* szpField, var_4& lrValue)
	{		
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = (var_4)atol(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_u4& lrValue)
	{		
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = (var_u4)atol(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_8& lrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = atol(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_u8& lrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		lrValue = (var_u8)atol(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_f4& frValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		frValue = atof(szaReadBuf);
		return 0;
	}
	var_4 GetFieldValue(const var_1* szpField, var_d8& drValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		drValue = atof(szaReadBuf); // atof return double type
		return 0;
	}
	// 取短整形类型参数
	var_4 GetFieldValue(const var_1* szpField, var_u2& usrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		usrValue = (var_u2)atol(szaReadBuf);
		return 0;
	}

	var_4 GetFieldValue(const var_1* szpField, var_1& usrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		usrValue = *szaReadBuf;
		return 0;
	}

	// 取IP地址及端口号参数
	var_4 GetMacIPAndPort(const var_1* szpField, var_1* szpValue, var_u2& usrValue)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		
		var_1* p = szaReadBuf;
		var_1* q = p + strlen(p) - 1;
		while(q > p && *q != ':')
		{
			if(*q < '0' || *q > '9')
				break;
			q--;
		}
		if(*q != ':')
			return -1;
		*q++ = 0;
		usrValue = (var_u2)atol(q);
		strcpy(szpValue, p);

		return 0;
	}
	// 取IP地址及2个端口号参数
	var_4 GetMacIPAndPort(const var_1* szpField, var_1* szpIp, var_u2& usrPort_1, var_u2& usrPort_2)
	{
		var_1 szaReadBuf[1024];
		if(GetFieldValue(szpField, szaReadBuf))
			return -1;
		
		var_1* p = szaReadBuf;
		var_1* q = p + strlen(p) - 1;
		
		while(q > p && *q != ':')
		{
			if(*q < '0' || *q > '9')
				break;
			q--;
		}
		if(*q != ':')
			return -1;	
		q++;
		usrPort_2 = (var_u2)atol(q);
		q -= 2;
		if(q < p)
			return -1;
		
		while(q > p && *q != ':')
		{
			if(*q < '0' || *q > '9')
				break;
			q--;
		}
		if(*q != ':')
			return -1;
		*q++ = 0;
		usrPort_1 = (var_u2)atol(q);		
		
		strcpy(szpIp, p);
		
		return 0;
	}
	// 得到当前版本号
	const var_1* GetVersion()
	{
		// v1.000 - 2008.08.26 - 初始版本
		// v1.100 - 2009.03.31 - 增加跨平台支持
		// v2.000 - 2013.04.15 - 重写核心函数,支持引号,分割符定义,注释符号定义
		return "v2.000";
	}

private:
	FILE* m_cpFile;
};

#endif // _UC_READ_CONFIG_FILE_H_
