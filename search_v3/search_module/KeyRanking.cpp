#include "KeyRanking.h"
#include "Class.h"
#include "UtilTemplateFunc.h"
static const int CAT_CUSTOM_GROUP = 1;
/*
*初始化函数需要具体模块重载
*Parameter psdi 搜索数据传递
*Parameter strConf 模块各自的配置文件地址
*return true初始化成功，false初始化失败 
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
	* Method:    query分析
	* Returns:   void
	* Parameter: SQueryClause & qc 结构化的查询语句
	* return: IAnalysisData*  返回的query分析数据 用户动态生成，框架负责销毁，生成失败需返回NULL
	*/
IAnalysisData* CKeyRanking::QueryAnalyse(SQueryClause& qc)
{
	CKeySrchAnalysis* p = new CKeySrchAnalysis;
	p->m_hmUrlPara = qc.hmUrlPara;
	SESSION_LOG(SL_NOTICE, "keyranking query analyse");

	return p;
}


/*
* Method:    query分类转换到特定处理模块，每个业务有且仅有一个入口模块，可以有多个辅助模块
* Returns:   void
* Parameter: SQueryClause & qc 结构化的查询语句
* return: CModule*  返回的转换到的模块指针
*/
CModule* CKeyRanking::QueryClassify(SQueryClause& qc)
{
	//通过查询词进行QUERY分类重选模块--例如切换到百货或者图书RANKING，或者百货图书列表
	//一个例子转向默认排序
	MOD_MAP::iterator i;
	for(i = m_mapMods->begin();i != m_mapMods->end(); ++i)
	{
		/*if(i->first == "default_ranking")//调用其他模块的FILL
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
	// 给出from to 则可进行部分排序,提高效率
	
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
	* Method:    计算权重分析
	* Returns:   void
	* Parameter: IAnalysisData * pad 返回的query分析数据
	* Parameter: SMatchElement & me每个文档的匹配信息
	* Parameter: SResult & rt 打分结果
	*/
void CKeyRanking::ComputeWeight(IAnalysisData* Pad, SMatchElement& me, SResult& rt)
{
	/* 
	*modify your code here
	*/


}

	 /*
	*Method:  运行于GROUP_BY之前;
	*Parameter: IAnalysisData* Pad,    返回的query分析数据
	*Parameter: vector<SResult> & vRes 搜索打分结果
	*Parameter: vector<PSS> & vGpName  分组汇总的字段名和信息值
	*Parameter: GROUP_RESULT    vGpRes 传入数组 字段ID对应-->统计信息
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
				vGpName[j].second = "58"; //全站以58为基准统计分类,因为百货一级
			}
		}

	}
}
void CKeyRanking::FillGroupByData(vector<pair<int, vector<SGroupByInfo> > >& vGpInfo)
{
	//调用其它模块示例 :用list ranking填充列表数据 
	MOD_MAP::iterator i = m_mapMods->find("list_ranking");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->FillGroupByData(vGpInfo);
	}
}

/*
*Method:  设置过滤和汇总顺序
*Parameter:IAnalysisData* Pad,    返回的query分析数据
*Parameter:vec 传入默认汇总和过滤顺序，可调整顺序或者增加自定义过滤汇总方法
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
*Method:  自定义汇总函数 不能对vDocIds重新排序，只能删除已有的ID
*Parameter:IAnalysisData* Pad,    返回的query分析数据
*Parameter:vDocIds 文档ID序列
*Parameter:fgNode  汇总标识信息
*Parameter:prGpInfo  pair first 用户可自定义各种负整数，second为汇总信息
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
	// 整理业务时候确认 //先调用默认的
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
