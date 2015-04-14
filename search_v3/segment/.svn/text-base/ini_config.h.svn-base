#ifndef INI_INICONFIG_H
#define INI_INICONFIG_H

#include "common_libs.h"
#include <fstream>
#include <list>
#include <map>
#include <assert.h>
#include <wctype.h>
#include <stdio.h>

namespace segment{
class CIniConfig
{
#define CHECK_GETVALUE(key,defaultValue )	assert( key &&  *key  ); CStringMapConstItr it = m_element.find( key  ); if( it == m_element.end() || it->second->value.empty() ) return defaultValue;	
#define CHECK_SETVALUE(key,comment )		assert( key &&  *key  ); CStringMapItr it = m_element.find(  key  ); CNode *node;if( it == m_element.end() ) {m_line.push_back(CNode());node = &*m_line.rbegin(); node->bRemoved = false; m_element[key] = node;} else	{node = it->second;}node->comment = comment?comment:"";	
	typedef struct
	{
		std::string key;
		std::string value;
		std::string comment;		
		bool bRemoved;	
		void trim()
		{
			trim( key);trim(value);trim(comment);
		}

	private:
		void trim(std::string &str) const
		{
			while( !str.empty()&& isspace( (unsigned char)*str.begin() ))
				str.erase( str.begin());			
			while( !str.empty()&& isspace( (unsigned char)*str.rbegin() )) 
				str.erase( str.end() - 1 );			
		}

	}CNode;

	typedef struct 
	{		
		bool operator( )(const char *a,const char *b )const
		{
			return strcmp(a,b) < 0;			
		}
	}KeySort;

public:
	CIniConfig()
	{	
	}
	~CIniConfig()
	{
	}	
	bool remove(const char *key)
	{
		CStringMapItr it = m_element.find( key  );
		if ( it == m_element.end()) return false;
		it->second->bRemoved = true;
		m_element.erase( it );
		return true;
	}

	bool load( const char *file )
	{
		std::ifstream fs( file );
		if ( !fs ) return false;
		std::string str;
		m_line.clear();
		m_element.clear();
		CNode *node;
		std::string::size_type p1,p2;
		std::string key,value,comment;
		while ( std::getline( fs,str ) )
		{
			m_line.push_back(CNode());
			node = &*m_line.rbegin();
			node->bRemoved = false;
			if (  std::string::npos != (p1=str.find('=') ))
			{
				node->key = str.substr(0,p1++);				
				if ( std::string::npos != (p2 = str.find('#', p1)))
				{					
					node->value = str.substr(p1,p2 - p1);
					node->comment = str.substr(++p2);
				}else
				{
					node->value = str.substr(p1);
				}
			}
			node->trim();

			if (! node->key.empty() )
				m_element[node->key.c_str()] = node;					
		}
		fs.close();
		return true;
	}

	void save(const char *file)
	{
		std::ofstream fs(file);

		std::string value;

		for(std::list < CNode >::const_iterator it = m_line.begin();it != m_line.end();it++)
		{
			if ( it->bRemoved) continue;
			if ( it->key.empty() )
			{
				if(it->comment.empty())  
					fs  << std::endl;
				else 
					fs  << "#" << it->comment   << std::endl;				
			}
			else
			{							
				if(it->comment.empty())  
				{
					fs <<  it->key << "=" << it->value  <<  std::endl;
				}else
				{
					fs <<  it->key << "=" << it->value << "	#" <<  it->comment <<  std::endl;
				}			
			}			
		}
		fs.close();
	}

	//////////////////////////////////////////////////////////////////////////	
	char get(const char *key,char defaultValue) const
	{
		CHECK_GETVALUE(key,defaultValue);		
		const char *v = it->second->value.c_str();
		int value = 0;
		if ( *v == '\\' )
		{			
			if ( *(v+1) == 'x' )
			{
				int offset = 1;
				char ch;
				for ( int i = (int)it->second->value.size() - 1; i > 1 ; i--,offset <<= 4)
				{					
					ch  = it->second->value[i];
					if ( iswdigit( ch ) )
					{
						value += ( (ch - '0') *  offset);
					}else if( ch >= 'a' && ch <= 'f')
					{
						value += ( 10 +  (ch - 'a') *  offset);
					}else if( ch >= 'A' && ch <= 'F')
					{
						value += ( 10 +  (ch - 'A') *  offset);
					}else
					{
						value = defaultValue;
						break;
					}									
				}				
			}
			else if ( *(v+1) == 'r' )
				value = '\r';
			else if ( *(v+1) == 'n' )
				value = '\n';	
			else if ( *(v+1) == 't' )
				value = '\t';	
			else if ( *(v+1) == 'b' )
				value = '\b';
			else 
				value = *(v+1);
		}
		else
		{
			value = it->second->value[0];	
		}
		return (char)value;
	}

	float get(const char *key,float defaultValue) const
	{		
		CHECK_GETVALUE(key,defaultValue);
		return (float)atof( it->second->value.c_str() );
	}

	int get(const char *key,int defaultValue) const 
	{
		CHECK_GETVALUE(key,defaultValue);
		return atoi( it->second->value.c_str() );
	}

	const char *get(const char *key,const char *defaultValue) const
	{
		CHECK_GETVALUE(key,defaultValue);
		return  it->second->value.c_str() ;
	}

	//////////////////////////////////////////////////////////////////////////	
	void set( const char *key,const char * value,const char *comment = 0)
	{
		CHECK_SETVALUE(key,comment);
		node->value = value ? value :"";
	}

	void set( const char *key,float value,const char *comment = 0)
	{
		CHECK_SETVALUE(key,comment);
		char buf[12];
		sprintf(buf,"%f",value);
		node->value = buf;
	}

	void set( const char *key,char value,const char *comment = 0)
	{
		CHECK_SETVALUE(key,comment);			
		if ( isalnum( value ) )
			node->value = value;
		else if ( value == '\r')
			node->value = "\\r";
		else if ( value == '\n')
			node->value = "\\n";
		else if ( value == '\t')
			node->value = "\\t";
		else if ( value == '\b')
			node->value = "\\b";
		else{
			char buf[32];
			sprintf(buf,"\\x%x",value);
			node->value = buf;				
		}		
		node->key = key;		
	}	

	void set( const char *key,int value,const char *comment = 0)
	{		
		CHECK_SETVALUE(key,comment);
		char buf[12];
		sprintf(buf,"%d",value);
		node->value = buf;
	}	

private:
	std::list < CNode > m_line;	
	typedef std::map<const char *, CNode * ,KeySort > CStringMap;
	typedef  CStringMap::const_iterator CStringMapConstItr;
	typedef  CStringMap::iterator CStringMapItr;
	CStringMap m_element;
};

class PidConfig 
{
public:

	//para
	bool mode;

	//dict
	std::string	CatalogFilePath;
	std::string Dict_Com;	//普通汉语词典
	std::string Dict_Amb;	//汉语歧义词典
	std::string Dict_AmbF;	//过滤后的歧义词典
	std::string Dict_Pro;	//汉语专有名词词典
	std::string Dict_Eng;	//英语数字专有名词词典
	//index
	std::string Idx_Dat;	//双数组索引树
	std::string Idx_Map;	//扩展汉字映射表

public:
	PidConfig()		
	{		
	}	
	bool InitPara(const char* iniFile)
	{
		//INIFile ini( iniFile);
		typedef enum{
			type_string = 1,
			type_int,
			type_float,
			type_char,
		}config_type;

		const struct
		{		
			const char *name;
			config_type type;
			void *value;			
		}configinfo[]={
			//para
			{"mode",type_int,&mode},

			//dict
			{"CatalogFilePath",type_string,&CatalogFilePath},
			{"Dict_Com",type_string,&Dict_Com},
			{"Dict_Amb",type_string,&Dict_Amb},
			{"Dict_AmbF",type_string,&Dict_AmbF},
			{"Dict_Pro",type_string,&Dict_Pro},
			{"Dict_Eng",type_string,&Dict_Eng},
			{"Idx_Dat",type_string,&Idx_Dat},
			{"Idx_Map",type_string,&Idx_Map}
		};
		int size = (int)(sizeof(configinfo)/sizeof(configinfo[0]));

		CIniConfig config;
		if (!config.load(iniFile)) 
			return false;
		for ( int i = 0; i < size;i++)
		{
			switch( configinfo[i].type )
			{
			case type_string:
				*((std::string *)configinfo[i].value) = config.get( configinfo[i].name,( (std::string *)configinfo[i].value)->c_str()) ;
				break;

			case type_int:
				*((int *)configinfo[i].value) = config.get( configinfo[i].name,*((int *)configinfo[i].value));
				break;

			case type_float:
				*((float *)configinfo[i].value) = config.get( configinfo[i].name,*((float *)configinfo[i].value));
				break;

			case type_char:
				*((char *)configinfo[i].value) = config.get( configinfo[i].name,*((char *)configinfo[i].value));
				break;
			}
		}

		if(!CatalogFilePath.empty())
		{
			Dict_Com=CatalogFilePath+Dict_Com;
			Dict_Amb=CatalogFilePath+Dict_Amb;
			Dict_AmbF=CatalogFilePath+Dict_AmbF;
			Dict_Pro=CatalogFilePath+Dict_Pro;
			Dict_Eng=CatalogFilePath+Dict_Eng;
			Idx_Dat=CatalogFilePath+Idx_Dat;
			Idx_Map=CatalogFilePath+Idx_Map;
		}

		return true;
	}
};
}
#endif
