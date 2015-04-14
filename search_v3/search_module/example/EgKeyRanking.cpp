#include "EgKeyRanking.h"

CEgKeyRanking::~CEgKeyRanking()
{
	//需要手动释放的对象请手动清理
	fprintf(stderr, "CEgKeyRanking  leave\n");
}

/*
*初始化函数需要具体模块重载
*Parameter psdi 搜索数据传递
*Parameter strConf 模块各自的配置文件地址
*return true初始化成功，false初始化失败 
*/
bool CEgKeyRanking::Init(SSearchDataInfo* psdi, const string& strConf)
{
	CModule::Init(psdi, strConf);//always called 

	if (!InitCommon(psdi, strConf)
		||!InitPub(psdi, strConf)
		||!InitMall(psdi, strConf))
	{
		COMMON_LOG(SL_ERROR, "CEgKeyRanking init failed");
		return false;
	}

	COMMON_LOG(SL_INFO, "CEgKeyRanking init ok");
	return true;
}


bool CEgKeyRanking::InitCommon(SSearchDataInfo* psdi, const string& strConf) {
	m_stockProfile = FindProfileByName("stock");
	if (m_stockProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "stock profile does not exist");
		return false;
	}
	m_clsProfile = FindProfileByName("category_path");
	if (m_clsProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "category_path profile does not exist");
		return false;
	}
	m_saleWeekProfile = FindProfileByName("sale_week");
	if (m_saleWeekProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "sale_week profile does not exist");
		return false;
	}
	m_inputDateProfile = FindProfileByName("input_date");
	if (m_inputDateProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "input_date profile does not exist");
		return false;
	}
	m_isShareProductProfile = FindProfileByName("is_share_product");
	if (m_isShareProductProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "is_share_product profile does not exist");
		return false;
	}
	m_isPidProfile = FindProfileByName("product_id");
	if (m_isPidProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "product_id profile does not exist");
		return false;
	}
	COMMON_LOG(SL_INFO, "init common success");
	return true;
}


IAnalysisData* CEgKeyRanking::QueryAnalyse(SQueryClause& qc)
{
	CDDAnalysisData* pa = new CDDAnalysisData;
	QueryAnalyseCommon(pa, qc);
	if (pa->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH)
	{
		QueryAnalysePub(pa, qc);
		QueryAnalyseMall(pa, qc);
	}
	else if (pa->m_searchType == CDDAnalysisData::PUB_SEARCH)
	{
		QueryAnalysePub(pa,qc);
	}
	else if (pa->m_searchType == CDDAnalysisData::MALL_SEARCH)
	{
		QueryAnalyseMall(pa,qc);
	}
	else
	{
		;
	}
#ifdef DEBUG
	printf("query type = %d\n", pa->m_searchType);
#endif
	return pa;
}


void CEgKeyRanking::QueryAnalyseCommon(CDDAnalysisData* pa, SQueryClause& qc)
{
	pa->m_hmUrlPara = qc.hmUrlPara;//保存全部URL参数 
	pa->m_otherSortField = qc.firstSortId;//取其他排序字段
	if (qc.cat == 0) //全站搜索
	{
		pa->m_searchType = CDDAnalysisData::FULL_SITE_SEARCH;
	}
	else if (isMall(qc.cat))
	{
		pa->m_searchType = CDDAnalysisData::MALL_SEARCH;
	}
	else
	{
		pa->m_searchType = CDDAnalysisData::PUB_SEARCH;
	}

#ifdef DEBUG
	printf("QueryAnalyseCommon query type = %d\n", pa->m_searchType);
#endif
}


void CEgKeyRanking::ComputeWeight(IAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//影响性能的关键函数 
	//cout << "enter ComputeWeight" << endl;
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;

	unsigned long long cls = m_funcFrstInt64(m_clsProfile, rt.nDocId);
	if (isMall(cls))//取该商品第一个分类的数据 判断是否百货
	{
		if (pda->m_searchType != CDDAnalysisData::PUB_SEARCH) //只要不是出版物搜索
		{
			ComputeWeightMall(pda, me, rt);
			rt.nWeight += TypeIndicator; //表示百货
		}
		else //百货搜索中检索出的非百货商品不打分，因为后面会过滤掉
		{
			//rt.nWeight =0； rt.nScore = 0;
		} //
	}
	else if(cls != 0)//如果本商品是出版物
	{
		if (pda->m_searchType != CDDAnalysisData::MALL_SEARCH) //只要不是百货搜索
		{
			ComputeWeightPub(pda, me, rt);
		}
		else //出版物搜索中检索出的百货商品不打分，因为后面会过滤掉
		{ 
			//rt.nWeight =0； rt.nScore = 0;
		} //
	}
	else //分类为0，则商品无分类
	{
		//没分类先利用出版物的打分
		//cout << "no category: doc_id = " << rt.nDocId << " cls = " << cls << endl;
		//ComputeWeightPub(pda, me, rt);
	}
	//cout << "exit ComputeWeight" << endl;
	
}

void CEgKeyRanking::ReRanking(vector<SResult>& vRes, IAnalysisData* pa)
{
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;
	if (pda->m_searchType == CDDAnalysisData::MALL_SEARCH) //确定是百货搜索
	{
		ReRankingMall(vRes, pda);
	}
	else if (pda->m_searchType == CDDAnalysisData::PUB_SEARCH)//确定是出版物搜索
	{
		ReRankingPub(vRes, pda);
	}
	else //全站搜索
	{
		ReRankingFullSite(vRes, pda);
	}
}


//获取图书和百货商品的比例
float CEgKeyRanking::GetAlpha(const string& query) {
	pair<int, int> rate = m_percentPub2Mall[query];
	if (rate.first>0 && rate.second>0) { //在反馈文件中能够找到
		float alpha = float(rate.second)/float(rate.first + rate.second);
		if(alpha > 0.94) {
			 alpha = 1;
		} else if(alpha<0.07) {
			alpha=0;
		}
		return alpha;
	}
	return 0.5; //默认返回该值
}


void CEgKeyRanking::ReComputeWeightFullSite(vector<SResult>& vRes, float alpha) {
	double ret = 0;
	for (int i=0; i<vRes.size(); i++) {
		int score = 0;
		double ret = 0;
		if (vRes[i].nWeight/TypeIndicator == 0) { //出版物
			ret = (double)vRes[i].nScore / (double)m_max_weight_pub;
			ret = ret * alpha;
			score = m_base_weight * ret;
			vRes[i].nScore = score; //用于混排的权重
		} else { //百货
			ret = (double)vRes[i].nScore / (double)m_max_weight_bh;
			ret = ret * (1 - alpha);
			score = m_base_weight * ret;
			vRes[i].nScore = score;
		}
	}
}


void CEgKeyRanking::ReRankingFullSite(vector<SResult>& vRes, CDDAnalysisData* pa)
{
	//此段代码仅示例 -提示如何精简结果，取数据等 （含其他排序字段情况下，把无库存的删掉）
	/*if (((CDDAnalysisData*)pa)->m_otherSortField != -1)
	{
		int i = 0;
		int j = 0;
		for (; i < vRes.size(); ++i)
		{
			if (m_funcFrstInt(m_stockProfile, vRes[i].nDocId) > 0)
			{
				vRes[j++] = vRes[i];
			}
		}
		vRes.resize(j);
	}*/
    //精简百货和图书
#ifdef DEBUG
	cout << ">>>>>>>>>>>>>>>ReRankingFullSite" << endl;
#endif
	ReRankingMall(vRes, pa);

	//重新计算混排商品权重
	float alpha = GetAlpha(pa->m_mallAnalysisPart.querystr); //@todo: 这里querystr去掉了标点符号和空格，确认是否
	                                                         //和反馈文件中的保持一致
#ifdef DEBUG
	cout << "alpha = " << alpha << endl;
#endif
	ReComputeWeightFullSite(vRes, alpha);
}


void CEgKeyRanking::SortForDefault(vector<SResult>& vRes, int from, int to, IAnalysisData* pa)
{
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;
	if (pda->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH) ////全站搜索
	{
#ifdef DEBUG
		printf("enter sort for SortFullSite\n");
#endif
		SortFullSite(vRes, from, to, pda);
	}
	else if (pda->m_searchType == CDDAnalysisData::PUB_SEARCH)//确定是出版物搜索
	{
#ifdef DEBUG
		printf("enter sort for SortPub\n");
#endif
		SortPub(vRes, from, to, pda);
	}
	else //确定是百货搜索
	{
#ifdef DEBUG
		printf("enter sort for SortMall\n");
#endif
		SortMall(vRes, from, to, pda);
	}


}


//全站排序
void CEgKeyRanking::SortFullSite(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa)
{
#ifdef DEBUG
	printf("enter sort for full site\n");
#endif
	// do somethine more 
	SortRange(vRes ,from, to);
#ifdef DEBUG
	int out_size = min((int)vRes.size(), 50);
	for (int i=0; i<out_size; i++) {
		int docID = vRes[i].nDocId;
		int date = m_funcFrstInt(m_inputDateProfile, docID);
		int pid = m_funcFrstInt64(m_isPidProfile, docID);
		vector<char> vBuf;
		vector<char*> vFieldPtr;
		vector<int> vShowFields;
		vShowFields.push_back(GetFieldId("name"));
		m_funcDocInfoPtr(vRes[i].nDocId, vShowFields, vFieldPtr, vBuf, m_pSearcher);
		cerr << "debug_weight=" << vRes[i].nWeight << "  rank_score=" << vRes[i].nScore << " title=" << vFieldPtr[0] << " pid=" << pid << endl;
	}
#endif
}



void CEgKeyRanking::FillGroupByData(vector<pair<int, vector<SGroupByInfo> > >& vGpInfo)
{
	//调用其它模块示例 :用list ranking填充列表数据 
	MOD_MAP::iterator i = m_mapMods->find("list_ranking");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->FillGroupByData(vGpInfo);
	}
}


void CEgKeyRanking::ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
							vector<SResult>& vRes, vector<string>& vecRed, string& strRes)
{
// 整理业务时候确认 //先调用默认的
	/*MOD_MAP::iterator i = m_mapMods->find("default_ranking");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->ShowResult(pad, ctrl, vGpRes, vRes, vecRed, strRes);
	}*/
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->ShowResult(pad, ctrl, vGpRes, vRes, vecRed, strRes);
	}
		
}

	
#ifndef _WIN32
// linux dll
CEgKeyRanking eg4key_ranking;
#endif

