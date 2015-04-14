#ifndef DICCOMMON_H
#define DICCOMMON_H

#include <string>
#include <vector>
#include "hash_wrap.h"

namespace segment{

typedef unsigned short WORD;
typedef unsigned long  DWORD;
typedef unsigned char  BYTE;

//zhouhui
typedef int PosType;
typedef short LengthType;
typedef short LevelType;

//-----------------------TYPE_DEFINE--------------------------------//
typedef std::string   S;
typedef std::vector<std::string>         VS;
typedef std::vector<int>            VI;
typedef VS::iterator           VSI;
typedef VI::iterator           VII;
typedef hash_map<int,int>      HII;
typedef hash_map<S,int>        HSI;
typedef hash_map<int,S>        HIS;
typedef hash_map<S,S>          HSS;
typedef std::pair<int,int>          PII;
typedef std::pair<int,std::string>       PIS;
typedef std::pair<std::string,std::string>    PSS;
typedef std::pair<std::string,int>       PSI;
typedef HII::iterator          HIII;
typedef HSI::iterator          HSII;
typedef HIS::iterator          HISI;
typedef HSS::iterator          HSSI;

//--------------------MACRO_DEFINE----------------------------------//
#define FOR_EACH(iter,container)          for(iter=container.begin();iter!=container.end();++iter)
#define FOR_EACH_POS(iter,container)      for(iter=0;iter!=container.size();++iter)
#define FOR_EACH_CLASSIC(iter,start,end)  for(iter=start;iter<end;++iter)

}
#endif
