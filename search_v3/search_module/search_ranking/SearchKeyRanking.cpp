#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "SearchKeyRanking.h"
#include "../../util/BitMap.h"

CSearchKeyRanking::~CSearchKeyRanking()
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
bool CSearchKeyRanking::Init(SSearchDataInfo* psdi, const string& strConf)
{
	CModule::Init(psdi, strConf);//always called 

	if (!InitCommon(psdi, strConf)
			||!InitPub(psdi, strConf)
			||!InitMall(psdi, strConf))
	{
		COMMON_LOG(SL_ERROR, "CSearchKeyRanking init failed");
		return false;
	}
	COMMON_LOG(SL_NOTICE, "CSearchKeyRanking init ok");
	return true;
}


bool CSearchKeyRanking::InitCommon(SSearchDataInfo* psdi, const string& strConf) {
	m_stockProfile = FindProfileByName("stock_status");
	if (m_stockProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "stock_status profile does not exist");
		return false;
	}
	m_clsProfile = FindProfileByName("cat_paths");
	if (m_clsProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "cat_paths profile does not exist");
		return false;
	}
	m_saleWeekProfile = FindProfileByName("sale_week");
	if (m_saleWeekProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "sale_week profile does not exist");
		return false;
	}
	
	m_saleWeekAmtProfile = FindProfileByName("sale_week_amt");
        if (m_saleWeekAmtProfile == NULL)
        {
                COMMON_LOG(SL_ERROR, "sale_week_amt profile does not exist");
                return false;
        }	

	m_inputDateProfile = FindProfileByName("first_input_date");
	if (m_inputDateProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "first_input_date profile does not exist");
		return false;
	}

	m_modifyTime = FindProfileByName("modifytime");
	if (m_modifyTime == NULL)
	{
		COMMON_LOG(SL_ERROR, "modifyTime profile does not exist");
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

static inline int GetMaxValFromVect(vector<SResult>& vect)
{
	int max = 0;
	for(size_t i=0; i<vect.size(); i++)
	{
		if(vect[i].nDocId>max)
			max = vect[i].nDocId;
	}
	return max;
}

IAnalysisData* CSearchKeyRanking::QueryAnalyse(SQueryClause& qc)
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
		QueryAnalyseMall(pa, qc);
	}
	else if (pa->m_searchType == CDDAnalysisData::MALL_SEARCH)
	{
		QueryAnalysePub(pa, qc);
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


void CSearchKeyRanking::QueryAnalyseCommon(CDDAnalysisData* pa, SQueryClause& qc)
{
	pa->m_hmUrlPara = qc.hmUrlPara;//保存全部URL参数 
	pa->m_otherSortField = qc.firstSortId;//取其他排序字段

	//pa->bAutoAd = false;
	pa->bAdvance = qc.bAdvance;
	pa->m_vTerms.resize(qc.vTerms.size());
	pa->m_vTerms.assign(qc.vTerms.begin(),qc.vTerms.end());

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


void CSearchKeyRanking::ComputeWeight(IAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//影响性能的关键函数 
	//cout << "enter ComputeWeight" << endl;
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;

	unsigned long long cls = m_funcFrstInt64(m_clsProfile, rt.nDocId);
	if (isMall(cls))//取该商品第一个分类的数据 判断是否百货
	{
		//cout << "bh category: doc_id = " << rt.nDocId << " cls = " << cls << endl;
		if (pda->m_searchType != CDDAnalysisData::PUB_SEARCH) //只要不是出版物搜索
		{
			ComputeWeightMall(pda, me, rt);
			rt.nWeight += TypeIndicator; //表示百货
		}
		else //百货搜索中检索出的非百货商品不打分，因为后面会过滤掉
		{
			//rt.nWeight =0； rt.nScore = 0;
		}
	}
	else if(cls != 0)//如果本商品是出版物
	{
		//cout << "pub category: doc_id = " << rt.nDocId << " cls = " << cls << endl;
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
		ComputeWeightPub(pda, me, rt);
	}
	//cout << "exit ComputeWeight" << endl;

}

void CSearchKeyRanking::ReRanking(vector<SResult>& vRes, IAnalysisData* pa)
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
float CSearchKeyRanking::GetAlpha(const string& query) {
	pair<int, int> rate = m_percentPub2Mall[query];
	if (rate.first>0 || rate.second>0) { //在反馈文件中能够找到
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


void CSearchKeyRanking::ReComputeWeightFullSite(vector<SResult>& vRes, float alpha,int &bhcount) {
	//bhcount = 0;
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
			bhcount++;
		}
	}
}


void CSearchKeyRanking::ReRankingFullSite(vector<SResult>& vRes, CDDAnalysisData* pa)
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
	ReRankingPub(vRes,pa);
	//重新计算混排商品权重
	float alpha = GetAlpha(pa->m_mallAnalysisPart.querystr); //@todo: 这里querystr去掉了标点符号和空格，确认是否
	//和反馈文件中的保持一致
	/*if(alpha > 0.5)
		pa->m_strReserve+="PUB_TEMPLATE";
	else if(alpha < 0.5)
		pa->m_strReserve+="BH_TEMPLATE";
	else 
		pa->m_strReserve+="UNKNOWN_TEMPLATE";*/
#ifdef DEBUG
	cout << "alpha = " << alpha << endl;
#endif
	int bhcount = 0;
	if(true == pa->m_mallAnalysisPart.ifBrdQuery)
		alpha = 0.4; 
	ReComputeWeightFullSite(vRes, alpha,bhcount);
	float bhtmp = 0.0f;
	if(vRes.size() != 0)
	 	bhtmp = bhcount/((int)vRes.size());
	if(alpha > 0.5)
	{
		if(pa->m_strReserve.size() != 0)
                	pa->m_strReserve+=";WEB:PUB_TEMPLATE";
		else
			pa->m_strReserve+="WEB:PUB_TEMPLATE";
	}
        else if(alpha < 0.5)
	{
		if(pa->m_strReserve.size() != 0)
                	pa->m_strReserve+=";WEB:BH_TEMPLATE";
		else
			pa->m_strReserve+="WEB:BH_TEMPLATE";
	}
        else
	{
		if(bhtmp > 0.6)
		{
			if(pa->m_strReserve.size() != 0)
				pa->m_strReserve+=";WEB:BH_TEMPLATE";
			else
				 pa->m_strReserve+="WEB:BH_TEMPLATE";
		}
		else
		{
			if(pa->m_strReserve.size() != 0)
                		pa->m_strReserve+=";WEB:UNKNOWN_TEMPLATE";
			else
				pa->m_strReserve+="WEB:UNKNOWN_TEMPLATE";
		}
	}
	//printf("str=%s\n",pa->m_strReserve.c_str());
	
}

static inline void SwapVectElem(SResult& a, SResult& b)
{
	SResult sr;
	sr = a;
	a = b;
	b = a;
}

bool CSearchKeyRanking::RemoveGoodsByForceAndFB(const string& query,
		vector<SResult>& vRes,vector<SResult>& force_vect)
{
	force_vect.clear();
	int pid = 0;
	int docId_max = 0;
	int pid_flag = 0;
	vector<long long> pid_vect;
	vector<int> docId_vect;
	if(vRes.empty()|| query.empty())
	{
		return false;
	}
	//判断是否有反馈
	HASHVECTOR hvect = m_key2Pid[query];
	cout<<"size is "<<hvect.count<<endl;
	if(hvect.count>0)
	{
		//取得反馈pid
		pid_vect.reserve(hvect.count);
		for(int j=0;j<hvect.count;j++)
		{
			memcpy(&pid,hvect.data+j*hvect.size,sizeof(int));
			pid_vect.push_back((long long)pid);
		}
		//将pid转化为docId
		pid_flag = GetFieldId("product_id");
		m_funcGetDocsByPkPtr(m_pSearcher,pid_flag,pid_vect,docId_vect);
		docId_max = GetMaxValFromVect(vRes);
		//装入BITMAP
		CBitMap bitMap(docId_max+1);
		for(size_t i=0; i<docId_vect.size(); i++)
		{
			if(docId_vect[i]<0 || docId_vect[i]>docId_max)
				continue;
			bitMap.SetBit(docId_vect[i]);
		}
		//调整force_vect的大小
		force_vect.reserve(pid_vect.size());
		//遍历vect，修改其强制pid对应的权重，另其最大
		for(size_t j=0; j<vRes.size(); j++)
		{
			if(vRes[j].nDocId>docId_max || vRes[j].nDocId<0)
				continue;
			if(bitMap.TestBit(vRes[j].nDocId))
			{
				//删除,打散后再强制置顶
				force_vect.push_back(vRes[j]);
				SwapVectElem(vRes[j],vRes[vRes.size()-1]);
				vRes.pop_back();
			}
			if(force_vect.size() >= pid_vect.size())
				break;
		}
		docId_vect.clear();
		pid_vect.clear();
	}
	return true;
}

bool CSearchKeyRanking::ForceMoveGoods(vector<SResult>& vRes,vector<SResult>& force_vect)
{
	if(force_vect.empty())
		return false;
	vRes.insert(vRes.begin(),force_vect.begin(),force_vect.end());
	return true;
}


bool CSearchKeyRanking::ReduceNoImageProductScore(vector<SResult>& vRes)
{
	bool bimage = false;
	for(size_t j=0; j<vRes.size(); j++)
	{
		bimage = m_funcFrstInt(m_num_imagesProfile,vRes[j].nDocId)>0?true:false;
		if(bimage == false)
		{
			//最小得分
			vRes[j].nScore = BaseRelFactor + 1;
		}
	}

	return true;
}


void CSearchKeyRanking::SortForDefault(vector<SResult>& vRes, int from, int to, IAnalysisData* pa)
{
	string query = "";
	vector<SResult> force_vect;
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;
	
	if(pa->bAdvance)
	{
		for(size_t t = 0; t < vRes.size();t++)
		{
			int stock = m_funcFrstInt(m_stockProfile, vRes[t].nDocId)>0?1:0;
			int sale_amt = m_funcFrstInt(m_saleWeekAmtProfile, vRes[t].nDocId);
			vRes[t].nScore = (stock<<24) +sale_amt;
		}
	}
	//将pid转化为DocID，并添加到BitMap中	
#ifdef DEBUG
	clock_t begin,end;
	float cost;
	begin = clock();
#endif
	if(pa->m_hmUrlPara.find("q") != pa->m_hmUrlPara.end())
	{
		query = pa->m_hmUrlPara["q"];
		if(!RemoveGoodsByForceAndFB(query,vRes,force_vect))
		{
#ifdef DEBUG
			printf("Remove Goods by Force File Failed!\n");
#endif
		}
	}
#ifdef DEBUG
	end = clock();
	cost = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("find time is %lf seconds\n", cost);
#endif

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
	//将强制关联的商品置顶
	if(!ForceMoveGoods(vRes,force_vect))
	{
#ifdef DEBUG
		cout<<"ForceMoveGoods error!"<<endl;
#endif
	}
}


//全站排序
void CSearchKeyRanking::SortFullSite(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa)
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
		int modifytime = m_funcFrstInt(m_modifyTime, docID);
		int pid = m_funcFrstInt64(m_isPidProfile, docID);
		vector<char> vBuf;
		vector<char*> vFieldPtr;
		vector<int> vShowFields;
		vShowFields.push_back(GetFieldId("product_name"));
		m_funcDocInfoPtr(vRes[i].nDocId, vShowFields, vFieldPtr, vBuf, m_pSearcher);
		cerr << "debug_weight=" << vRes[i].nWeight << "  rank_score=" << vRes[i].nScore << " title=" << vFieldPtr[0] << " pid=" << pid << endl;
	}
#endif
}



void CSearchKeyRanking::FillGroupByData(vector<pair<int, vector<SGroupByInfo> > >& vGpInfo)
{
	//调用其它模块示例 :用list ranking填充列表数据 
	MOD_MAP::iterator i = m_mapMods->find("list_ranking");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->FillGroupByData(vGpInfo);
	}
}


void CSearchKeyRanking::ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
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

void CSearchKeyRanking::QueryRewrite(hash_map<string,string>& hmParams)
{
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->QueryRewrite(hmParams);
	}
}

void CSearchKeyRanking::SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vec)
{
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->SetGroupByAndFilterSequence(pad,vec);
	}

}

void CSearchKeyRanking::CustomFilt(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& fgNode)
{
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->CustomFilt(pad,vDocIds, fgNode);
	}

}

void CSearchKeyRanking::CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
		pair<int, vector<SGroupByInfo> >& prGpInfo)
{
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->CustomGroupBy(pad,vDocIds, gfNode,prGpInfo);
	}

}

void CSearchKeyRanking::SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad)
{
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->SortForCustom(vRes,from, to,pad);
	}
}

void CSearchKeyRanking::BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, vector<PSS>& vGpName, GROUP_RESULT& vGpRes)
{
	MOD_MAP::iterator i = m_mapMods->find("query_pack");
	if ( i != m_mapMods->end())
	{
		i->second.pMod->BeforeGroupBy(pad,vRes,vGpName,vGpRes);
	}

}
#ifndef _WIN32
// linux dll
CSearchKeyRanking search_ranking;
#endif

