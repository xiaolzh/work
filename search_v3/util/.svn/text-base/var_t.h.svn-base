/*
*by liugang 2012.4.9
*a set for any signed inner type number; convert between string and number
*
*/


#ifndef VAR_T_H
#define VAR_T_H

#include <stdio.h>

#include <map>
#include <string>
#include <typeinfo>

using namespace std;

#ifdef _WIN32
#define atoll _atoi64
#endif

struct var_base_s
{
	virtual bool to_type(const char* str) = 0;

	virtual const char* to_string(char* buf) = 0;
	
	virtual const char* get_min_buf() = 0;
	virtual const char* get_max_buf() = 0;
	virtual const char* get_type() = 0;
};


typedef map<string, var_base_s*> VAR_SET;

template<class T>
inline void var_t_to_string(const T& t, char* buf)
{
	long long l;
	l = t;
	sprintf(buf, "%lld", l);
}

template<>
inline void var_t_to_string<float>(const float& t, char* buf)
{
	sprintf(buf, "%f", t);
}

template<>
inline void var_t_to_string<double>(const double& t, char* buf)
{
	sprintf(buf, "%f", t);
}


template<class T>
inline T var_t_to_type(const char* str_buf, T)
{
	long long l = atoll(str_buf);
	return (T)l;
}

template<>
inline float var_t_to_type<float>(const char* str_buf, float)
{
	double f = atof(str_buf);
	return (float)f;
}

template<>
inline double var_t_to_type<double>(const char* str_buf, double)
{
	return atof(str_buf);
}

template<class T>
struct var_s:public var_base_s
{
	var_s()
	{
		memset(m_data, 0, sizeof(m_data));
		memset(m_min_buf, 0, sizeof(m_min_buf));
		memset(m_max_buf, 0, sizeof(m_max_buf));
	}
	bool init(const char* name, T val, T min, T max, VAR_SET& var_set)
	{
		VAR_SET::iterator it = var_set.find(name);
		if (it != var_set.end() )
		{
			return true;
		}
		else
		{
			var_set.insert(make_pair(name, (var_base_s*)this));
		}
		m_min = min;
		m_max = max;
		if (m_min <= val && val <= m_max)
		{
			*this = val;
			var_t_to_string(min, m_min_buf);
			var_t_to_string(max, m_max_buf);
			sprintf(m_type_buf, "%s", typeid(T).name());
			return true;
		}


		return false;
	}

	virtual bool to_type(const char* str)
	{
		T t = var_t_to_type(str, T());
		if (m_min <=t && t <=m_max)
		{
			*this = t;
			return true;
		}
		return false;
	}

	virtual const char* to_string(char* buf)
	{
		var_t_to_string(*(T*)m_data, buf);
		return buf;
	}


	virtual const char* get_min_buf()
	{
		return m_min_buf;
	}

	virtual const char* get_max_buf()
	{
		return m_max_buf;
	}

	virtual const char* get_type()
	{
		return m_type_buf;
	}


	var_s& operator=(const T t)
	{
		*(T*)m_data = t;
		return *this;
	}

	operator T()
	{
		return *(T*)m_data;
	}
	T m_min;
	T m_max;

	char m_data[64];
	char m_min_buf[64];
	char m_max_buf[64];
	char m_type_buf[64];
};
#endif
