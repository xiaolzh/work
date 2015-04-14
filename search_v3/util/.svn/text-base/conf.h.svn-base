#ifndef CONF_H
#define CONF_H

#include "UtilDef.h"
#include <stdio.h>
#include <stdlib.h>

extern  hash_map<string,pair<int*,string> > g_hmGlbConfInt;
extern  hash_map<string,pair<float*,string> > g_hmGlbConfFloat;

typedef hash_map<string,pair<int*,string> > HSSIP;
typedef hash_map<string,pair<float*,string> > HSSFP;

//float var prefix with "f_"
//int var prefix with "i_"
inline bool LoadConf(const char* pConfName)
{

	HSSIP &hssip=g_hmGlbConfInt;
	HSSFP &hssfp=g_hmGlbConfFloat;
	HSSIP::iterator iip;
	HSSFP::iterator ifp;

	FILE* fp=fopen(pConfName,"r");
	FALSE_RETURN_STRERROR(!fp);
	char chBuf[1024]={0};
	int n;
	while(!feof(fp))
	{
		if (!fgets(chBuf,1024,fp))
			break;
		n=strlen(chBuf);
		if (n<4&&chBuf[1]!='_')
			continue;
		char* p=chBuf+n-1;
		while(p>=chBuf&&(*p=='\r' || *p=='\n'))
			*p--='\0';
		p=chBuf;
		while(p<chBuf+n&&*p!='=')
			++p;
		if (p==(char*)chBuf+n)
		{
			continue;
		}
		*p='\0';
		++p;

		if (chBuf[0]=='f')
		{
			ifp=hssfp.find(chBuf);
			if (ifp!=hssfp.end())
			{
				*(ifp->second.first)=atof(p);
			}

		}

		if (chBuf[0]=='i')
		{
			iip=hssip.find(chBuf);
			if (iip!=hssip.end())
			{
				*(iip->second.first)=atof(p);
			}
		}

	}

	fclose(fp);
	return true;

}


inline bool StoreConf(const char* pConfName)
{

	FILE* fp=fopen(pConfName,"w");
	FALSE_RETURN_STRERROR(!fp);

	HSSIP &hssip=g_hmGlbConfInt;
	HSSFP &hssfp=g_hmGlbConfFloat;
	HSSIP::iterator iip;
	HSSFP::iterator ifp;

	for (iip=hssip.begin();iip!=hssip.end();++iip)
	{
		fprintf(fp,"%s=%d\n",iip->first.c_str(),*(iip->second.first));
	}

	for (ifp=hssfp.begin();ifp!=hssfp.end();++ifp)
	{
		fprintf(fp,"%s=%f\n",ifp->first.c_str(),*(ifp->second.first));
	}
	fclose(fp);
	return true;
}

inline bool RestoreConf(const char* pConfBakName,const char* pConfName)
{
	;
	if (!LoadConf(pConfBakName))
	{
		return false;
	}

	if (!StoreConf(pConfName))
	{
		return false;
	}
	return true;

}


inline void SetConf(HSS& hssIn,bool bStore,string& strShow)
{
	string sPre;
	string sRealkey;
	float fVal;
	int   iVal;

	HSSIP &hssip=g_hmGlbConfInt;
	HSSFP &hssfp=g_hmGlbConfFloat;
	HSSIP::iterator iip;
	HSSFP::iterator ifp;


	for (HSSI hssi=hssIn.begin();hssi!=hssIn.end();++hssi)
	{
		if (hssi->first.length()<2)
		{
			continue;
		}

		sPre=string(hssi->first.c_str(),2);
		if (sPre=="f_")
		{
			ifp=hssfp.find(hssi->first);
			if (ifp!=hssfp.end())
			{
				*(ifp->second.first)=atof(hssi->second.c_str());
			}

		}

		if (sPre=="i_")
		{
			iip=hssip.find(hssi->first);
			if (iip!=hssip.end())
			{
				*(iip->second.first)=atoi(hssi->second.c_str());
			}

		}
	}
	char buf[1024];
	strShow="<?xml version=\"1.0\" encoding=\"GB2312\"?>";
	strShow+="<info>";
	strShow+="<int_element>";
	for (iip=hssip.begin();iip!=hssip.end();++iip)
	{
		sprintf(buf,"<element name=\"%s\" value=\"%d\" comment=\"%s\"/>",iip->first.c_str(),*(iip->second.first),iip->second.second.c_str());
		strShow+=buf;
	}
	strShow+="</int_element>";
	strShow+="<float_element>";
	for (ifp=hssfp.begin();ifp!=hssfp.end();++ifp)
	{
		sprintf(buf,"<element name=\"%s\" value=\"%f\"  comment=\"%s\"/>",ifp->first.c_str(),*(ifp->second.first),ifp->second.second.c_str());
		strShow+=buf;
	}
	strShow+="</float_element>";
	strShow+="</info>";

}

#endif
