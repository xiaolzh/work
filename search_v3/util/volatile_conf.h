#ifndef VOLATILE_CONF_H
#define VOLATILE_CONF_H

#include <stdio.h>
#include <stdlib.h>
#include "var_t.h"

inline bool var_load_conf(const char* name, VAR_SET& vset)
{
	FILE* fp = fopen(name,"r");
	if(!fp) return false;

	int n;
	char buf[1024] = {0};
	VAR_SET::iterator it;
	while(!feof(fp))
	{
		if (!fgets(buf, sizeof(buf) - 1 , fp))
			break;
		n = strlen(buf);
		if (n < 4)
			continue;

		char* p = buf + n - 1;
		while(p >= buf && (*p == '\r' || *p=='\n') )
		{
			*p-- = '\0';
			--n;
		}

		p = buf;
		while(p < buf + n && *p!= '=')
			++p;
		if (p==(char*)buf + n)
			continue;
		*p = '\0';
		++p;

		it = vset.find(buf);
		if (it != vset.end())
			it->second->to_type(p);
	}

	fclose(fp);
	return true;

}


inline bool var_store_conf(const char* name, VAR_SET& vset)
{
	FILE* fp = fopen(name, "w");
	if(!fp) return false;

	char buf[256];
	VAR_SET::iterator it;
	for (it = vset.begin(); it != vset.end(); ++it)
	{
		fprintf(fp, "%s=%s\n",it->first.c_str(), it->second->to_string(buf));
	}
	fclose(fp);
	return true;
}

inline bool var_restore_conf(const char* bak_name, const char* name, VAR_SET& vset)
{
	if (!var_load_conf(bak_name, vset))
		return false;

	if (!var_store_conf(name, vset))
		return false;

	return true;
}

inline void var_set_one_opt(const char* name, const char* value, VAR_SET& vset)
{
	VAR_SET::iterator it = vset.find(name);
	if (it != vset.end())
		it->second->to_type(value);
}

#endif
