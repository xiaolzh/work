/*
*ģ���ڴ����ԭ��
*1�����ָ�붼����Ҫ�û��ͷ�
*2�û���̬���ɵ�IAnalysisData����ϵͳ�Լ��ͷ�
*/


#ifndef MIX_RECO_H
#define MIX_RECO_H

#include "ExModule.h"

class CMixReco:public CExModule
{
public:
	virtual ~CMixReco(){;}//��������������������̬��Դ��ģ����ܶ������

	virtual void Reco(const string& sKey, const vector<SResult>& vResIn, vector<SResult>& vResOut, CBitMap& bmForbiden);

	virtual void NeedSynSearch(const vector<SResult>& vResIn, const string& sKey, vector<string>& vSyn);
	
	virtual void ComputeWeight4WordMatch(vector<SFieldOff>& vFieldsOff, vector<Term>& vTerms, SResult& rt);
};
#endif	
