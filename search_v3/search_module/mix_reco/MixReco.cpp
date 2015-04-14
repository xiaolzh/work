#include "MixReco.h"


void CMixReco::Reco(const string& sKey, const vector<SResult>& vResIn, vector<SResult>& vResOut, CBitMap& bmForbiden)
{
	SResult sr;
	
	vector<long long> vPk;
	vector<int>  vDoc;
	int fid = GetFieldId("product_id");
	if (fid == -1) return;
	void* m_pfl = m_vProfiles[fid];
	if (m_pfl == NULL) return; 
	if (sKey == "�������� ����")
	{
		vPk.push_back(22809061);
		vPk.push_back(22785729);
		m_funcGetDocsByPkPtr(m_pSearcher, fid, vPk, vDoc); 
		for(int i = 0; i < vDoc.size(); ++i)
		{
			if (vDoc[i]!=-1 && !bmForbiden.TestBit(vDoc[i]))
			{	
				sr.nDocId = vDoc[i];
				sr.nScore = i*i/3;
				vResOut.push_back(sr);
			}
		}
	}
	if (sKey == "123")
	{
		if (!bmForbiden.TestBit(34801))
		{	
			sr.nDocId = 34801;
			sr.nScore = 4;
			vResOut.push_back(sr);
		}

		if (!bmForbiden.TestBit(33358))
		{	
			sr.nDocId = 33358;
			sr.nScore = 4;
			vResOut.push_back(sr);
		}
	}


	if (vResIn.empty())
	{
		return;
	}
	vPk.clear();
	vDoc.clear();
	if (m_funcFrstInt64(m_pfl, vResIn[0].nDocId) == 22928412)
	{
		vPk.push_back(22918753);
		vPk.push_back(22937322);
		m_funcGetDocsByPkPtr(m_pSearcher, fid, vPk, vDoc); 
		for(int i = 0; i < vDoc.size(); ++i)
		{
			if (vDoc[i]!=-1 && !bmForbiden.TestBit(vDoc[i]))
			{	
				sr.nDocId = vDoc[i];
				sr.nScore = i*i/3;
				vResOut.push_back(sr);
			}
		}
	}

}

void CMixReco::NeedSynSearch(const vector<SResult>& vResIn, const string& sKey, vector<string>& vSyn)
{
	if (sKey == "������")
	{
		vSyn.push_back("ɳ����");
		vSyn.push_back("������");
		vSyn.push_back("ɳ����");
	}

	if (sKey == "zhengnengliang")
	{
			vSyn.push_back("������");
		}

	if (sKey == "123")
	{
		vSyn.push_back("45 ������");
		vSyn.push_back("67");
	}

}

void CMixReco::ComputeWeight4WordMatch(vector<SFieldOff>& vFieldsOff, vector<Term>& vTerms, SResult& rt)
{

}
#ifndef _WIN32
// linux dll
CMixReco mix_reco;
#endif

