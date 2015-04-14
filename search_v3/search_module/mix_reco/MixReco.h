/*
*模块内存管理原则
*1传入的指针都不需要用户释放
*2用户动态生成的IAnalysisData对象系统自己释放
*/


#ifndef MIX_RECO_H
#define MIX_RECO_H

#include "ExModule.h"

class CMixReco:public CExModule
{
public:
	virtual ~CMixReco(){;}//请在派生类中重载清理动态资源，模块可能多次载入

	virtual void Reco(const string& sKey, const vector<SResult>& vResIn, vector<SResult>& vResOut, CBitMap& bmForbiden);

	virtual void NeedSynSearch(const vector<SResult>& vResIn, const string& sKey, vector<string>& vSyn);
	
	virtual void ComputeWeight4WordMatch(vector<SFieldOff>& vFieldsOff, vector<Term>& vTerms, SResult& rt);
};
#endif	
