#ifndef _ABSTRACT_INFO_SHOW_H_
#define _ABSTRACT_INFO_SHOW_H_
#include <iostream>
#include "TaDicW.h"
#include <vector>
#include <string>
#include <set>
#include "GlobalDef.h"
#include "PackDefine.h"
#include "TimeUtil.h"
using namespace std;

class CAbstractInfo
{
	public:
		CAbstractInfo();	

		~CAbstractInfo(){}	
		//init punction list	
		void InitUnDealSet();

		// get offset from red text field
		void GetInterpunctionOffset(string& str); 

		// make the abstract window which will be processed 
		void MakeAbstractWind(string& str);

		// compute the score of window
		int ComputeAbstractWind(string& str,VEC_PR_II& vpii);

		//init query vector,load query vector to datrie tree
		bool Init(vector< string >& vecRed,string& strIn,string& strOut);

		//red text and make auto abstract
		void RedText(string& str,VEC_PR_II& vpii,int iPos,string& strOut);	

		void RedText(string& str,VEC_PR_II& vpii,string& strOut);

		//cut abstract info	
		int GetSubAbstract(string& input, int maxLen,string& rs);

		bool Process(vector< string >& vecRed,string& strIn,string& strOut);

		//process query and red text
		void QueryProcess(string& str);

		//init vector
		void Init();

	private:
		vector< KPunc > interpunctionOffset;
		vector< string > interpunctionList; 
		vector< KWind >  vecWindList;
		bool bProcess; 
};
#endif
