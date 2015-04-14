#include "KeyRanking.h"
#include "Class.h"
#include "UtilTemplateFunc.h"
static const int CAT_CUSTOM_GROUP = 1;
/*
*��ʼ��������Ҫ����ģ������
*Parameter psdi �������ݴ���
*Parameter strConf ģ����Ե������ļ���ַ
*return true��ʼ���ɹ���false��ʼ��ʧ�� 
*/
bool CKeyRanking::Init(SSearchDataInfo* psdi, const string& strConf)
{
	CModule::Init(psdi, strConf);

	m_pflSalePrice = FindProfileByName("dd_sale_price");
	if (m_pflSalePrice == NULL)
		COMMON_LOG(SL_ERROR, "dd_sale_price profile does not exist");
	m_pflPromoPrice = FindProfileByName("promo_saleprice");
	if (m_pflPromoPrice == NULL)
		COMMON_LOG(SL_ERROR, "promo_saleprice profile does not exist");
	/* 
	*modify your code here
	*/
	COMMON_LOG(SL_NOTICE, "CKeyRanking init ok");
	return true;
}


	/*
	* Method:    query����
	* Returns:   void
	* Parameter: SQueryClause & qc �ṹ���Ĳ�ѯ���
	* return: IAnalysisData*  ���ص�query�������� �û���̬���ɣ���ܸ������٣�����ʧ���践��NULL
	*/
IAnalysisData* CKeyRanking::QueryAnalyse(SQueryClause& qc)
{
	CKeySrchAnalysis* p = new CKeySrchAnalysis;
	p->m_hmUrlPara = qc.hmUrlPara;
	SESSION_LOG(SL_NOTICE, "keyranking query analyse");

	return p;
}


/*
* Method:    query����ת�����ض�����ģ�飬ÿ��ҵ�����ҽ���һ�����ģ�飬�����ж������ģ��
* Returns:   void
* Parameter: SQueryClause & qc �ṹ���Ĳ�ѯ���
* return: CModule*  ���ص�ת������ģ��ָ��
*/
CModule* CKeyRanking::QueryClassify(SQueryClause& qc)
{
	//ͨ����ѯ�ʽ���QUERY������ѡģ��--�����л����ٻ�����ͼ��RANKING�����߰ٻ�ͼ���б�
	//һ������ת��Ĭ������
	MOD_MAP::iterator i;
	for(i = m_mapMods->begin();i != m_mapMods->end(); ++i)
	{
		/*if(i->first == "default_ranking")//��������ģ���FILL
		{
			printf("i am key_ranking, switch to default ranking\n");
			return i->second.pMod;
		}*/
	}
	if(i == m_mapMods->end())
		return this;
}


void  CKeyRanking::SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad)
{
	// ����from to ��ɽ��в�������,���Ч��
	
	string sCust = pad->m_hmUrlPara[US];
	
	if (m_pflSalePrice && m_pflPromoPrice)
	{
		if (sCust == "xprice_desc" || sCust == "xprice_asc")
		{
			int dd_price,promo_price;
			bool bAsc = ( sCust == "xprice_asc");
			for (int i = 0; i < vRes.size(); ++i)
			{
				promo_price = m_funcFrstInt(m_pflPromoPrice, vRes[i].nDocId);
				dd_price = m_funcFrstInt(m_pflSalePrice, vRes[i].nDocId);
				vRes[i].nScore = ( (promo_price == 0) ? dd_price : (promo_price < dd_price ? promo_price : dd_price) );
				if (bAsc)
					vRes[i].nScore = -vRes[i].nScore;
			}
			SortRange(vRes ,from, to);
		}
	}
}
	
	/*
	* Method:    ����Ȩ�ط���
	* Returns:   void
	* Parameter: IAnalysisData * pad ���ص�query��������
	* Parameter: SMatchElement & meÿ���ĵ���ƥ����Ϣ
	* Parameter: SResult & rt ��ֽ��
	*/
void CKeyRanking::ComputeWeight(IAnalysisData* Pad, SMatchElement& me, SResult& rt)
{
	/* 
	*modify your code here
	*/


}

	 /*
	*Method:  ������GROUP_BY֮ǰ;
	*Parameter: IAnalysisData* Pad,    ���ص�query��������
	*Parameter: vector<SResult> & vRes ������ֽ��
	*Parameter: vector<PSS> & vGpName  ������ܵ��ֶ�������Ϣֵ
	*Parameter: GROUP_RESULT    vGpRes �������� �ֶ�ID��Ӧ-->ͳ����Ϣ
	*/
void CKeyRanking::BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, vector<PSS>& vGpName, GROUP_RESULT& vGpRes)
{
	//just a test
	int fieldId;
	hash_map<string,string>::iterator i = pad->m_hmUrlPara.find("t");
	if (i != pad->m_hmUrlPara.end() && i->second == "key_full_site")
	{
		for (int j = 0; j < vGpName.size(); ++j)
		{
			fieldId = GetFieldId(vGpName[j].first.c_str());
			if (fieldId != -1 && m_vFieldInfo[fieldId].nSpecialType == CAT_FIELD)
			{
				vGpName[j].second = "58"; //ȫվ��58Ϊ��׼ͳ�Ʒ���,��Ϊ�ٻ�һ��
			}
		}

	}
}
void CKeyRanking::FillGroupByData(vector<pair<int, vector<SGroupByInfo> > >& vGpInfo)
{
	//��������ģ��ʾ�� :��list ranking����б����� 
	MOD_MAP::iterator i = m_mapMods->find("list_ranking");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->FillGroupByData(vGpInfo);
	}
}

/*
*Method:  ���ù��˺ͻ���˳��
*Parameter:IAnalysisData* Pad,    ���ص�query��������
*Parameter:vec ����Ĭ�ϻ��ܺ͹���˳�򣬿ɵ���˳����������Զ�����˻��ܷ���
*/
void CKeyRanking::SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vFgn)
{
	int fieldId;
	hash_map<string,string>::iterator i = pad->m_hmUrlPara.find("t");
	if (i != pad->m_hmUrlPara.end() && i->second == "key_full_site")
	{
		for (int j = 0; j < vFgn.size(); ++j)
		{
			fieldId = vFgn[j].nField;
			if (fieldId != -1 && m_vFieldInfo[fieldId].nSpecialType == CAT_FIELD && vFgn[j].nType == SFGNode::GROUPBY_TYPE)
			{
				vFgn[j].nCid = -1;
				vFgn[j].nId = CAT_CUSTOM_GROUP;
			}
		}
	}

	for (int j = 0; j < vFgn.size(); ++j)
	{
		fieldId = vFgn[j].nField;

		if (m_vFieldInfo[fieldId].nSpecialType == PARA_FILED /*&& ST == MALL*/)
		{
			unsigned long long cls = 0;
			string path;

			// test only
			path = (pad->m_hmUrlPara["category_path"]+ pad->m_hmUrlPara["-category_path"]);
			// test only

			cls = TranseClsPath2ID(path.c_str(), path.length());
			int nPflId = GetFieldId("category_path");
			if (cls != 0 && nPflId != -1)
			{
				vFgn[j].sgpf.nMax=cls;
				vFgn[j].sgpf.nMin=cls;
				vFgn[j].sgpf.nNot=0;
				vFgn[j].sgpf.nPflId=nPflId;
			}
		}

		if (m_vFieldInfo[fieldId].nSpecialType == CAT_FIELD && vFgn[j].nType == SFGNode::GROUPBY_TYPE)
		{
			vFgn[j].sgpf.nCatlimit = 3;
		}
	}
	;
}

/*
*Method:  �Զ�����ܺ��� ���ܶ�vDocIds��������ֻ��ɾ�����е�ID
*Parameter:IAnalysisData* Pad,    ���ص�query��������
*Parameter:vDocIds �ĵ�ID����
*Parameter:fgNode  ���ܱ�ʶ��Ϣ
*Parameter:prGpInfo  pair first �û����Զ�����ָ�������secondΪ������Ϣ
*/
void CKeyRanking::CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
						   pair<int, vector<SGroupByInfo> >& prGpInfo)
{
	int fieldId;
	hash_map<string,string>::iterator i = pad->m_hmUrlPara.find("t");
	if (i != pad->m_hmUrlPara.end() && i->second == "key_full_site")
	{
		
			if (gfNode.nId=CAT_CUSTOM_GROUP)
			{
				m_funcGpClassPtr(m_vProfiles[gfNode.nField],vDocIds,1000,prGpInfo.second,true, 5*16+8);
				prGpInfo.first=gfNode.nField;
			}
	}
}



void CKeyRanking::ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
							   vector<SResult>& vRes, vector<string>& vecRed, string& strRes)
{
	// ����ҵ��ʱ��ȷ�� //�ȵ���Ĭ�ϵ�
	MOD_MAP::iterator i = m_mapMods->find("default_ranking");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->ShowResult(pad, ctrl, vGpRes, vRes, vecRed, strRes);
	}
	SESSION_LOG(SL_NOTICE, "keyranking ShowResult use default");

	//for test 
	int fid;
	for (fid = 0; fid < m_vFieldInfo.size(); ++fid)
	{
		if ((m_vFieldInfo[fid]).bPk)
		{
			break;
		}
	}
	void* pfl;
	vector<int> vDocIds;
	if (fid != m_vFieldInfo.size())
	{
		pfl = m_vProfiles[fid];

		vector<long long> vl;
		for (int j = ctrl.nF; j != ctrl.nT; ++j)
		{
			vl.push_back(m_funcFrstInt64(pfl, vRes[j].nDocId));
			SESSION_LOG(SL_DEBUG, "docId = %d, key = %lld ", vRes[j].nDocId, vl.back());
		}
		vDocIds.clear();
		m_funcGetDocsByPkPtr(m_pSearcher, fid, vl, vDocIds);
		for (int j = 0; j != vDocIds.size(); ++j)
		{
			SESSION_LOG(SL_DEBUG, "docId = %d, key = %lld ", vDocIds[j],vl[j]);
		}
	}

}

#ifndef _WIN32
// linux dll
CKeyRanking key_ranking;
#endif
