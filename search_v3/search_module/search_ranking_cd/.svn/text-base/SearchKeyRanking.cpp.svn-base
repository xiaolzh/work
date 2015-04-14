#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include "SearchKeyRanking.h"
#include "effective_wtree.h"
#include "Class.h"
#include "BitMap.h"
#include "../../segment_new/gbk_encode_convert.h"
#include "TimeUtil.h"

CSearchKeyRanking::~CSearchKeyRanking()
{
	//需要手动释放的对象请手动清理
	fprintf(stderr, "CEgKeyRanking  leave\n");
	if(m_ip_location != NULL)
	{
		ddip_free(m_ip_location);
		m_ip_location = NULL;
	}
}

/*
 *初始化函数需要具体模块重载
 *Parameter psdi 搜索数据传递
 *Parameter strConf 模块各自的配置文件地址
 *return true初始化成功，false初始化失败 
 */

static inline bool filt_zero_score(const SResult& rs)
{
	return rs.nScore <= 0;
}

const FieldToId CSearchKeyRanking::field_id[] = {
	{"catalog", 1},
	{"content", 2},
	{"abstract", 3},
	{"series_name", 4},
	{"subname", 5},
	{"isbn_search", 5},
	{"brand_name", 6},
	{"publisher", 6},
	{"cat_paths_name", 7},
	{"actors", 8},
	{"director", 8},
	{"singer", 8},
	{"author_name", 8},
	{"title_synonym", 9},
	{"product_name", 10},
	{"title_primary", 11},
	{"title_sub", 12}
};

const int CSearchKeyRanking::ScoreBitMap[] = {
	0x0,
	0x1,
	0x3,
	0x7,
	0xf,
	0x1f,
	0x3f,
	0x7f,
	0xff
};

const char* CSearchKeyRanking::field[] = {
	"product_id",
	"is_share_product",
	"sale_day",
	"sale_week",
	"sale_month",
	"sale_day_amt",
	"sale_week_amt",
	"sale_month_amt",
	"modifytime",
	"first_input_date",
	"dd_sale_price",
	"pre_sale",
	"num_images",
	"publish_date",
	"total_review_count",
	"city_stock_status",
	"stock_status"
};

void CSearchKeyRanking::MoveLeftBit(int& score, int value, int moveNum)
{
	score += (value << moveNum);
	/*if(score >> 31)
	{
		cout <<"score:"<<score<<"value: "<<value<<"moveNum: "<<moveNum<< "bit process error!" << endl;
	}*/
}

static inline int GetMaxValFromVect(vector<SResult>& vect)
{
	int max = 0;
	for(size_t i = 0; i < vect.size(); i++)
	{
		if(vect[i].nDocId > max)
		{
			max = vect[i].nDocId;
		}
	}
	return max;
}

bool CSearchKeyRanking::Init(SSearchDataInfo* psdi, const string& strConf)
{
	CModule::Init(psdi, strConf);//always called 

	InitData();
	if(!InitCommon(psdi, strConf) || !LoadFile())
	{
		COMMON_LOG(SL_ERROR, "CSearchKeyRanking init failed");
		return false;
	}
	//whj
	if(!InitPersonalityLocationStockDict())
	{
#ifdef DEBUG
		cout << "init ip locat dict error!" << endl;
#endif
		COMMON_LOG(SL_ERROR, "load Ip 2 Loaction file failed");
	}
	COMMON_LOG(SL_NOTICE, "CSearchKeyRanking init ok");
	return true;
}

bool CSearchKeyRanking::InitData()
{
	for(int i = 0; i < 10000; i++)
	{
		m_salAmtScr.push_back((5 + (int)floor(1/(1 + exp((-1) * log10(0.1 * i) + 2)) * 120)) / 10);
	}
	for(int i = 0; i < 1000; i++)
	{
		m_salNumScr.push_back((5 + (int)floor(1/(1 + exp((-1) * log10(0.5 * i) + 1)) * 100)) / 10);
	}
	for(int i = 0; i < 100; i++)
	{
		m_commentScr.push_back((5 + (int)floor(1/(1 + exp((-1) * log10(0.5 * i) + 1)) * 100)) / 10);
	}
	return true;
}

bool CSearchKeyRanking::LoadFile()
{
	//加载反馈数据
	m_pid2Sub.read(m_strModulePath + "substrs");
	string path = m_strModulePath;
	path.append("author_pub");
	m_aurpub.load_serialized_hash_file(path.c_str(), -1);
	m_cid2Cids.read(m_strModulePath + "ext_cate");
	m_key2Cate.read(m_strModulePath + "query_cate_direction");
	m_brdkey2Cate.read(m_strModulePath + "query_cate_brand");
	m_key2Pid.read(m_strModulePath + "query_pid_direction");
	m_pid2Core.read(m_strModulePath + "pid_core_word");
	printf("init: load feedback data\n");

	//加载词典
	string file = m_strModulePath + "product_word.txt";
	string line;
	ifstream ifs(file.c_str());
	if (!ifs) 
	{
		COMMON_LOG(SL_ERROR, "can't find file: %s", file.c_str());
		return false;
	}
	while(getline(ifs, line)) 
	{
		boost::trim(line);
		m_pdtWords.insert(line);
	}
	ifs.close();
	ifs.clear();
	file = m_strModulePath + "brand_word.txt";
	ifs.open(file.c_str());
	if(!ifs)
	{
		COMMON_LOG(SL_ERROR, "can't find file: %s", file.c_str());
		return false;
	}
	while(getline(ifs, line))
	{
		boost::trim(line);
		m_brdWords.insert(line);
	}
	ifs.close();

	//加载混排数据
	string key_cate_rate_path(m_strModulePath + "key_cate_rate");
	if(false == m_percentPub2Mall.read(key_cate_rate_path.c_str()))
	{
		printf("failed to load key_cate_rate\n");
		return false;
	}
	printf("init merge data: load key_cate_rate\n");
	return true;
}

bool CSearchKeyRanking::InitCommon(SSearchDataInfo* psdi, const string& strConf) 
{
	m_vProfile.push_back(make_pair("city_stock_status", &m_stockStatusProfile));
	m_vProfile.push_back(make_pair("stock_status", &m_stockProfile));
	m_vProfile.push_back(make_pair("cat_paths", &m_clsProfile));
	m_vProfile.push_back(make_pair("sale_day", &m_saleDayProfile));
	m_vProfile.push_back(make_pair("sale_week", &m_saleWeekProfile));
	m_vProfile.push_back(make_pair("sale_month", &m_saleMonthProfile));
	m_vProfile.push_back(make_pair("sale_day_amt", &m_saleDayAmtProfile));
	m_vProfile.push_back(make_pair("sale_week_amt", &m_saleWeekAmtProfile));
	m_vProfile.push_back(make_pair("sale_month_amt", &m_saleMonthAmtProfile));
	m_vProfile.push_back(make_pair("first_input_date", &m_inputDateProfile));
	m_vProfile.push_back(make_pair("modifytime", &m_modifyTime));
	m_vProfile.push_back(make_pair("is_share_product", &m_isShareProductProfile));
	m_vProfile.push_back(make_pair("product_id", &m_isPidProfile));
	m_vProfile.push_back(make_pair("dd_sale_price", &m_salePriceProfile));
	m_vProfile.push_back(make_pair("pre_sale", &m_preSaleProfile));
	m_vProfile.push_back(make_pair("num_images", &m_numImagesProfile));
	m_vProfile.push_back(make_pair("publish_date", &m_pubDateProfile));
	m_vProfile.push_back(make_pair("total_review_count", &m_totalReviewCountProfile));
	m_vProfile.push_back(make_pair("is_publication", &m_isPublicationProfile));

	for(vector<pair<string, void**> >::iterator it = m_vProfile.begin(); it != m_vProfile.end(); it++)
    {
		string keyStr = it->first;
		*it->second = FindProfileByName(keyStr.c_str());
		if(*it->second == NULL)
		{
			COMMON_LOG(SL_ERROR, "%s profile does not exist", keyStr.c_str());
			return false;
		}
	}

	m_vFieldIndex.resize(sizeof(field_id) / sizeof(FieldToId));
	m_fid2fi.resize(m_vFieldInfo.size());

	int index;
	for(int i = 0; i < m_vFieldIndex.size(); i++)
	{
		index = GetFieldId(field_id[i].field);
		m_vFieldIndex[i] = index;
		if(index == -1)
		{
        	COMMON_LOG(SL_ERROR, "%s field does not exist", field_id[i].field);
        	return false;
    	} 
		else 
		{
        	m_fid2fi[index] = field_id[i].id;
    	}
	}

	for(int i = 0; i < sizeof(field)/sizeof(char*); i++)
	{
		index = GetFieldId(field[i]);
        if (index == -1)
        {
            COMMON_LOG(SL_ERROR, "%s field does not exist", field[i]);
            return false;
        }
	}
	return true;
}

IAnalysisData* CSearchKeyRanking::QueryAnalyse(SQueryClause& qc)
{
	CDDAnalysisData* pa = new CDDAnalysisData;
	pa->m_hmUrlPara = qc.hmUrlPara;			//保存全部URL参数 
	pa->m_otherSortField = qc.firstSortId;	//取其他排序字段

	pa->bAdvance = qc.bAdvance;
	pa->m_vTerms.resize(qc.vTerms.size());
	pa->m_vTerms.assign(qc.vTerms.begin(), qc.vTerms.end());
	if(qc.cat == 0)
	{
		pa->m_searchType = CDDAnalysisData::FULL_SITE_SEARCH;
	}
	else if(!isMall(qc.cat))
	{
		pa->m_searchType = CDDAnalysisData::PUB_SEARCH;
	}
	else
	{
		if(isCloth(qc.cat))
		{
			pa->m_searchType = CDDAnalysisData::CLOTH_SEARCH;
		}
		else if(is3C(qc.cat))
		{
			pa->m_searchType = CDDAnalysisData::C3_SEARCH;
		}
		else
		{
			pa->m_searchType = CDDAnalysisData::OTHER_SEARCH;
		}
	}
	
	vector<QueryItem> queryItems;
	QueryParse(qc.key, qc.vTerms, queryItems);
	size_t queryKeyCnt = queryItems.size();
	pa->m_AnalysisPart.type.resize(queryKeyCnt, -1);
	pa->m_AnalysisPart.pubtype.resize(queryKeyCnt, -1);
	pa->m_AnalysisPart.dis.resize(queryKeyCnt, 0);
	pa->m_AnalysisPart.ifSP.resize(queryKeyCnt, 0);
	pa->m_AnalysisPart.vCluster.resize(4);
	pa->m_AnalysisPart.key = qc.key;

	pa->m_AnalysisPart.ifBrdQuery = false;
	pa->m_AnalysisPart.ifPdtQuery = false;
	pa->m_AnalysisPart.ifAutPubSearch = -1;
	pa->m_AnalysisPart.needJudgeDis = true;
	/*pa->m_AnalysisPart.relCnt_fb = 0;
	pa->m_AnalysisPart.relCnt_prd = 0;
	pa->m_AnalysisPart.relCnt_brd = 0;
	pa->m_AnalysisPart.relCnt_ti = 0;
	pa->m_AnalysisPart.relDocCnt = 0;*/
	pa->m_AnalysisPart.tDocCnt = 0;
	pa->m_AnalysisPart.tDocCnt_stock = 0;
	
	size_t singleCnt = 0;		//单短字数目
	bool aurpub = false;		//关键词是否是人名出版社
	int pdtCnt = 0;				//产品中心词数
	bool hasBrd = false;		//是否有品牌词
	bool hasCom = false;		//是否有一般词
	bool needDis = false;		//是否需要计算词距
	string pdtWord = "";
	
	for(int i = 0; i < queryKeyCnt; i++)
	{
		string key = queryItems[i].word;
		pa->m_AnalysisPart.queryStr.append(key);//有效字符串获取
		if(queryItems[i].length <= 2)
		{ 
			++singleCnt;
		}
		if(false == queryItems[i].field.empty())
		{
			pa->m_AnalysisPart.ifLimitSearch = true;
		}
		pa->m_AnalysisPart.dis[i] = queryItems[i].fwdDis;
		pa->m_AnalysisPart.ifSP[i] = queryItems[i].fwdSP;
		
		if(m_aurpub[key] != -1 || m_brdWords.find(key) != m_brdWords.end())
		{
			if(m_aurpub[key] != -1)
			{
				aurpub = true;
				pa->m_AnalysisPart.ifAutPubSearch = pa->m_AnalysisPart.pubtype[i] = m_aurpub[key];	//人名出版物搜索
			}

			if(m_brdWords.find(key) != m_brdWords.end())
			{
				pa->m_AnalysisPart.type[i] = 2;    //品牌词
				hasBrd = true;
			}
			else if(m_pdtWords.find(key) != m_pdtWords.end())
			{ 
				pa->m_AnalysisPart.type[i] = 1;	//产品中心词
				++pdtCnt;
				pdtWord = key;
			}
			else
			{
				pa->m_AnalysisPart.type[i] = 0;	//一般词
				hasCom = true;
			}
		}
		else if(m_pdtWords.find(key) != m_pdtWords.end())
		{ 
			pa->m_AnalysisPart.type[i] = 1;	//产品中心词
			++pdtCnt;
			pdtWord = key;
		}
		else
		{
			pa->m_AnalysisPart.type[i] = 0;	//一般词
			hasCom = true;
		}

		//if(i > 0)
		//	cout<<"type0: "<<pa->m_AnalysisPart.type[i-1]<<"type1: "<<pa->m_AnalysisPart.type[i]<<endl;
		if(i > 0 && 0 == queryItems[i].fwdDis && 0 == pa->m_AnalysisPart.type[i-1] && 0 == pa->m_AnalysisPart.type[i])
		{ 
			needDis = true;
		}
	}
	
	string queryStr = pa->m_AnalysisPart.queryStr;	//有效查询串
	string key = pa->m_AnalysisPart.key; //原始查询串
	if(singleCnt == queryKeyCnt)
	{
		pa->m_AnalysisPart.ifAllSingleWord = true;
	}
	
	if(false == needDis || queryStr.size() > 12)
	{
		pa->m_AnalysisPart.needJudgeDis = false;
	}

	int kcate_count = m_key2Cate[queryStr].count;
	if(kcate_count != 0)
	{
		//类别反馈
		pa->m_AnalysisPart.ifHasFeedback = true;
		HASHVECTOR kcate_vec = m_key2Cate[queryStr];
		u64 cate_id = 0;
		int weight = 0;
		for(int k = 0; k < kcate_count; k++) 
		{
			memcpy(&cate_id, kcate_vec.data+k*kcate_vec.size, sizeof(u64));
			memcpy(&weight, kcate_vec.data+k*kcate_vec.size+sizeof(u64), sizeof(int));
			pa->m_AnalysisPart.vCate.push_back(pair<u64, int>(cate_id, weight));
		}
	}
	/*int pid_count = m_key2Pid[queryStr].count;
	if(pid_count != 0)
	{
		//单品反馈
		HASHVECTOR pid_vec = m_key2Pid[queryStr];
		int pid = 0;
		for(int k = 0; k < pid_count; k++)
		{
			memcpy(&pid, pid_vec.data+k*pid_vec.size, 4);
			pa->m_AnalysisPart.vPid.push_back(pid);
		}
	}*/
	if(aurpub)
	{
		for(int i = TMINNUM; i < m_vFieldIndex.size(); i++)
		{
			pa->m_queryFieldIds.push_back(m_vFieldIndex[i]);
		}
	}
	if(m_pdtWords.find(queryStr) != m_pdtWords.end() || (1 == pdtCnt && hasBrd && !hasCom))
	{
		//产品搜索/产品+品牌 搜索
		pa->m_AnalysisPart.ifPdtQuery = true;
		pa->m_AnalysisPart.needJudgeDis = false;
		//表示当前query只到倒排索引的标题、类别和品牌字段中搜索
		for(int i = TMINNUM; i < m_vFieldIndex.size(); i++)
		{
			pa->m_queryFieldIds.push_back(m_vFieldIndex[i]);
		}
		pa->m_AnalysisPart.pdtWord = m_pdtWords.find(queryStr) != m_pdtWords.end() ? queryStr : pdtWord;

		/*if(pa->m_AnalysisPart.pdtWord.size() > 2
			&& pa->m_AnalysisPart.pdtWord[pa->m_AnalysisPart.pdtWord.size() - 2] < 0)
		{
			string tmpNg = queryStr.substr(queryStr.size() - 2, 2);
			if(m_pdtWords.find(tmpNg) != m_pdtWords.end())
			{
				pa->m_AnalysisPart.pdtNg = tmpNg;
			}
		}*/
	}
	else if(m_brdWords.find(queryStr) != m_brdWords.end() || (hasBrd && !hasCom && 0 == pdtCnt) 
			|| m_brdWords.find(key) != m_brdWords.end())
	{
		pa->m_AnalysisPart.ifBrdQuery = true;
		int index = m_vFieldIndex[BOTTOMCATE];
		for(int i = TMINNUM; i < m_vFieldIndex.size(); i++)
		{
			if(index == m_vFieldIndex[i])
				continue;
			pa->m_queryFieldIds.push_back(m_vFieldIndex[i]);
		}

		/*HASHVECTOR vCat = m_brdkey2Cate[queryStr];
		int cate_count = vCat.count;
		if(cate_count != 0)
		{
			u64 cate = 0;
			int weight = 0;
			for(size_t c = 0; c < cate_count; ++c)
			{
				memcpy(&cate, vCat.data + c*vCat.size, sizeof(u64));
				memcpy(&weight, vCat.data + c*vCat.size + sizeof(u64), sizeof(int));
				pa->m_AnalysisPart.vBrdCate.push_back(pair<u64, int>(cate, weight));
			}
		}*/
	}
	else if(!aurpub)
	{
		for(int i = 0; i < m_vFieldIndex.size(); i++)
		{
			pa->m_queryFieldIds.push_back(m_vFieldIndex[i]);
		}
	}
	//whj
	if(GetUserIp2Location(pa) == false)
	{
#ifdef DEBUG
		printf("ip 2 location error!\n");
#endif
	}
	//cout<<"pa->m_AnalysisPart.ifBrdQuery: "<<pa->m_AnalysisPart.ifBrdQuery<<"query: "<<pa->m_AnalysisPart.key<<endl;
	return pa;
}

void CSearchKeyRanking::ComputeWeight(IAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//影响性能的关键函数 
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;
	unsigned long long cls = m_funcFrstInt64(m_clsProfile, rt.nDocId);

	if(isMall(cls))
	{
		if(isCloth(cls))
		{
			if(pda->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH || pda->m_searchType == CDDAnalysisData::CLOTH_SEARCH)
			{
#ifdef DEBUG
	cout<<"ComputeWeightCloth"<<endl;
#endif
				ComputeWeightCloth(pda, me, rt);
			}
		}
		else if(is3C(cls))
		{
			if(pda->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH || pda->m_searchType == CDDAnalysisData::C3_SEARCH)
			{
				ComputeWeight3C(pda, me, rt);
			}
		}
		else
		{
			if(pda->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH || pda->m_searchType == CDDAnalysisData::OTHER_SEARCH)
			{
#ifdef DEBUG
	cout<<"ComputeWeightOther"<<endl;
#endif
				ComputeWeightOther(pda, me, rt);
			}
		}
		rt.nWeight += TypeIndicator;
	}
	else if(cls != 0)
	{
		if(pda->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH || pda->m_searchType == CDDAnalysisData::PUB_SEARCH)
		{
			ComputeWeightPub(pda, me, rt);
		}
	}
	else
	{
		ComputeWeightPub(pda, me, rt);
	}
	
    //whj
	int docID = rt.nDocId;
    int blocation = pda->m_bit_city_location;
    int locationStock = 1;	//用户所在区域缺货:0为缺货，1为不缺货
    int stock = m_funcFrstInt(m_stockProfile, me.id);	//判断区域缺货
    //区域无库存后排
	if(stock == 1)
	{
		rt.nScore += (locationStock << 8);
	}
	else if(stock == 2)
	{
		if(JudgeLocationStock(docID, blocation, locationStock))
		{
			rt.nScore += (locationStock << 8);
		}
		else
		{
			rt.nScore += (1 << 8);
		}
	}
}

void CSearchKeyRanking::ReRankingMall(vector<SResult>& vRes, CDDAnalysisData* pa)
{
	size_t count = vRes.size();
#ifdef DEBUG
	cout<<"ReRankingMall before: "<<count<<endl;
#endif
	for(int i = 0; i < count; i++)
	{
		if(0 == vRes[i].nWeight / TypeIndicator)	//判断是否是出版物	
		{
			continue;
		}
		if(0 == ((vRes[i].nScore >> BASERELBIT) & ScoreBitMap[BASERELBITNUM]))	//是否满足基本相关
		{
			vRes[i].nScore = 0;
		}
	}
	vRes.erase(std::remove_if(vRes.begin(), vRes.end(), filt_zero_score), vRes.end());
#ifdef DEBUG
	cout<<"ReRankingMall after: "<<vRes.size()<<endl;
#endif
}

void CSearchKeyRanking::SortMall(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa)
{
	SortRange(vRes ,from, to);
#ifdef DEBUG
	int out_size = min((int)vRes.size(), 50);
	for (int i=0; i<out_size; i++)
	{
		int docID = vRes[i].nDocId;
		int date = m_funcFrstInt(m_modifyTime, docID);
		int pid = m_funcFrstInt64(m_isPidProfile, docID);
		vector<char> vBuf;
		vector<char*> vFieldPtr;
		vector<int> vShowFields;
		vShowFields.push_back(m_vFieldIndex[TITLENAME]);
		m_funcDocInfoPtr(vRes[i].nDocId, vShowFields, vFieldPtr, vBuf, m_pSearcher);
		cerr << "debug_weight=" << vRes[i].nWeight << "  rank_score=" << vRes[i].nScore << " title=" << vFieldPtr[0] << " pid=" << pid << endl;
	}
#endif
}

void CSearchKeyRanking::ReRanking(vector<SResult>& vRes, IAnalysisData* pa)
{
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;
	if(pda->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH) 
	{
		ReRankingFullSite(vRes, pda);
	}
    else if(pda->m_searchType == CDDAnalysisData::PUB_SEARCH)
	{
		ReRankingPub(vRes, pda);
	}
	else
	{
		ReRankingMall(vRes, pda);
	}
}

//获取图书、服装/鞋靴、3C、其它百货商品的比例
float CSearchKeyRanking::GetAlpha(const string& query, const int cat)
{
	HASHVECTOR hvect = m_percentPub2Mall[query];
	if(hvect.count != 0)
	{
		//在反馈文件中能够找到
		float alpha;
		memcpy(&alpha, hvect.data+cat*hvect.size, sizeof(float));
		if(alpha > 0.94)
		{
			alpha = 1;
		}
		else if(alpha < 0.07)
		{
			alpha = 0;
		}
		return alpha;
	}
	return 0.5; //默认返回该值
}

void CSearchKeyRanking::ReComputeWeightFullSite(vector<SResult>& vRes, float alpha, int &bhcount)
{
	/*if(1 == alpha)
	{
		for(int i = 0; i < vRes.size(); i++)
		{
			if (vRes[i].nWeight / TypeIndicator == 1)	//是否是百货
			{
				vRes[i].nScore = 0;
			}
		}
	}
	else if(alpha > 0)
	{*/
		for(int i = 0; i < vRes.size(); i++)
		{
			if (vRes[i].nWeight / TypeIndicator == 1)   //是否是百货
			{
				bhcount++;
			}
		}
	/*}
	else
	{
		for(int i = 0; i < vRes.size(); i++)
		{
			if (vRes[i].nWeight / TypeIndicator == 0)	//是否是出版物
			{
				vRes[i].nScore = 0;
			}
		}
	}*/
}

void CSearchKeyRanking::ReRankingFullSite(vector<SResult>& vRes, CDDAnalysisData* pa)
{
	//精简图书、服装/鞋靴、3C、其它百货类别
	//cout<<"before mall: "<<vRes.size()<<endl;
	ReRankingMall(vRes, pa);
	//cout<<"before pub: "<<vRes.size()<<endl;
	ReRankingPub(vRes, pa);
	//cout<<"after pub: "<<vRes.size()<<endl;
	
	//获取混排文件中出版物的比重
	int pubNum = CDDAnalysisData::PUB_SEARCH;
	alpha[pubNum] = GetAlpha(pa->m_AnalysisPart.queryStr, pubNum); //@todo: 这里queryStr去掉了标点符号和空格，确认是否
	if(pa->m_AnalysisPart.ifBrdQuery)
	{
		for(int i = 0; i < vRes.size(); i++)
		{
			if(vRes[i].nWeight / TypeIndicator != 1)   //是否是出版物
			{
				vRes[i].nScore = (int)(vRes[i].nScore * 0.7);
			}
		}
		alpha[pubNum] = 0.4;
	}
#ifdef DEBUG
	cout<<"ReRankingFullSite pub alpha: "<<alpha[pubNum]<<endl;
#endif

	int bhcount = 0;
	ReComputeWeightFullSite(vRes, alpha[pubNum], bhcount);
	
	float bhtmp = 0.0f;
	if(vRes.size() != 0)
		bhtmp = bhcount /(float)vRes.size();
	int pid=m_funcFrstInt64(m_isPidProfile, vRes[0].nDocId);
	if(alpha[pubNum] > 0.5)
	{
		if(pa->m_strReserve.size() != 0)
			pa->m_strReserve += ";WEB:PUB_TEMPLATE";
		else
			pa->m_strReserve += "WEB:PUB_TEMPLATE";
	}
	else if(alpha[pubNum] < 0.5)
	{
		if(pa->m_strReserve.size() != 0)
			pa->m_strReserve += ";WEB:BH_TEMPLATE";
		else
			pa->m_strReserve += "WEB:BH_TEMPLATE";
	}
	else
	{
		if(bhtmp > 0.6)
		{
			if(pa->m_strReserve.size() != 0)
				pa->m_strReserve += ";WEB:BH_TEMPLATE";
			else
				pa->m_strReserve += "WEB:BH_TEMPLATE";
		}
		else
		{
			if(pa->m_strReserve.size() != 0)
				pa->m_strReserve += ";WEB:UNKNOWN_TEMPLATE";
			else
				pa->m_strReserve += "WEB:UNKNOWN_TEMPLATE";
		}
	}
}
static inline void SwapVectElem(SResult& a, SResult& b)
{
	SResult sr;
	sr = a;
	a = b;
	b = sr;
}

bool CSearchKeyRanking::RemoveGoodsByForceAndFB(const string& query, 
		vector<SResult>& vRes, vector<SResult>& force_vect)
{
	force_vect.clear();
	int pid = 0;
	int stock = 0;
	int weight = 0;
	int docId_max = 0;
	int pid_flag = 0;
	vector<long long> pid_vect;
	vector<int> docId_vect;
	if(vRes.empty() || query.empty())
	{
		return false;
	}
	//判断是否有反馈
	HASHVECTOR hvect = m_key2Pid[query];
	if(hvect.count > 0)
	{
		//取得反馈pid
		pid_vect.reserve(hvect.count);
		for(int j = 0; j < hvect.count; j++)
		{
			memcpy(&pid, hvect.data + j * hvect.size, sizeof(int));
			memcpy(&weight, hvect.data + j * hvect.size+sizeof(int), sizeof(int));
			if(weight <= 3)//反馈权重小 过滤
				continue;
			pid_vect.push_back((long long)pid);
		}
		if(pid_vect.empty())
			return false;
		//将pid转化为docId
		pid_flag = GetFieldId("product_id");
		m_funcGetDocsByPkPtr(m_pSearcher, pid_flag, pid_vect, docId_vect);
		docId_max = GetMaxValFromVect(vRes);
		//装入BITMAP
		CBitMap bitMap(docId_max + 1);
		for(size_t i = 0; i < docId_vect.size(); i++)
		{
			if(docId_vect[i] < 0 || docId_vect[i] > docId_max)
			{
				continue;
			}
			bitMap.SetBit(docId_vect[i]);
		}
		//调整force_vect的大小
		//遍历vect，修改其强制pid对应的权重，另其最大
		for(size_t j = 0; j < vRes.size();)
		{
			if(vRes[j].nDocId > docId_max || vRes[j].nDocId < 0)
			{
				j++;
				continue;
			}
			if(bitMap.TestBit(vRes[j].nDocId))
			{
				stock = m_funcFrstInt(m_stockProfile, vRes[j].nDocId) > 0 ? 1 : 0;
				if(stock == 1)
				{
					//删除,打散后再强制置顶
					force_vect.push_back(vRes[j]);
					SwapVectElem(vRes[j], vRes[vRes.size()-1]);
					vRes.pop_back();
				}
				else
					j++;
			}
			else
			{
				j++;
			}
			if(force_vect.size() >= pid_vect.size())
			{
				break;
			}
		}
		docId_vect.clear();
		pid_vect.clear();
	}
	return true;
}

bool CSearchKeyRanking::ForceMoveGoods(vector<SResult>& vRes, vector<SResult>& force_vect)
{
	if(force_vect.empty())
	{
		return false;
	}
	sort(force_vect.begin(),force_vect.end(),greater<SResult>());
	vRes.insert(vRes.begin(), force_vect.begin(), force_vect.end());
	//cout<<"single feedback"<<endl;
	return true;
}

void CSearchKeyRanking::SortForDefault(vector<SResult>& vRes, int from, int to, IAnalysisData* pa)
{
	string query = "";
	vector<SResult> force_vect;
	CDDAnalysisData* pda = (CDDAnalysisData*)pa;	
	if(pa->bAdvance)
	{
		for(size_t t = 0; t < vRes.size(); t++)
		{
			int stock = m_funcFrstInt(m_stockProfile, vRes[t].nDocId) > 0 ? 1 : 0;
			int sale_amt = m_funcFrstInt(m_saleWeekAmtProfile, vRes[t].nDocId);
			vRes[t].nScore = (stock << 24) + sale_amt;
		}
	}
#ifdef DEBUG
	clock_t begin, end;
	float cost;
	begin = clock();
#endif
	if(pa->m_hmUrlPara.find("q") != pa->m_hmUrlPara.end())
	{
		query = pa->m_hmUrlPara["q"];
		if(!RemoveGoodsByForceAndFB(query, vRes, force_vect))
		{
#ifdef DEBUG
			printf("Remove Goods by Force File Failed!\n");
#endif
		}
		//cout<<"wu hai jun: "<<vRes.size()<<endl;
	}
	
	//modify sorted product num	
	to = to < vRes.size() ? to : vRes.size();

#ifdef DEBUG
	end = clock();
	cost = (double)(end - begin) / CLOCKS_PER_SEC;
	printf("wuhaijun find time is %d seconds\n", end-begin);
#endif

	if(pda->m_searchType == CDDAnalysisData::FULL_SITE_SEARCH)		//全站搜索
	{
#ifdef DEBUG
		printf("enter sort for SortFullSite\n");
#endif
		SortFullSite(vRes, from, to, pda);
	}
	else if(pda->m_searchType == CDDAnalysisData::PUB_SEARCH)		//确定是出版物搜索
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
		cout<<"Warning force_vect is NULL!"<<endl;
#endif
	}
	//模板选取
	if(!SelectShowTemplate(pda, vRes,29))
	{
#ifdef DEBUG
		cout<<"Template is error!"<<endl;
#endif
	}

}


//全站排序
void CSearchKeyRanking::SortFullSite(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa)
{
#ifdef DEBUG
	printf("enter sort for full site\n");
#endif

	int iLimit = to < SCATTER_UPPER_LIMIT ? to : SCATTER_UPPER_LIMIT ;	

	//get page size from m_hmUrlPara
	int page_size = 5;
	hash_map<string,string>::iterator itPS  = pa->m_hmUrlPara.find(PS);
	if(itPS != pa->m_hmUrlPara.end())
	{
		page_size = atoi(itPS->second.c_str());
		if(page_size == 0) 
		{
			page_size = 1; 
		}
		if(page_size > 200)
		{
			page_size = 200;
		}
	}

	// load feedback data
	//printf("enter sort for full site\n");
	int cate_count = m_key2Cate[pa->m_AnalysisPart.queryStr].count;	
	cate_count = cate_count < SCATTER_CAT_LIMIT ? cate_count : SCATTER_CAT_LIMIT;
#ifdef DEBUG
	cout<<"key word: "<<pa->m_AnalysisPart.queryStr<<" feedback cnt: "<<cate_count<<endl;
#endif
	SReferCatInfo sCatInfo;
	int total_rate = 0;	
	int have_scatt = 0;
	memset(&sCatInfo, 0x0, sizeof(SReferCatInfo));
	int iLev = 0;
	if(cate_count)
	{
		HASHVECTOR cate_vec = m_key2Cate[pa->m_AnalysisPart.queryStr];	
		for(int i = 0; i < cate_count; i++)
		{
			u64 cate_id = 0;
			int weight  = 0;
			memcpy(&cate_id, cate_vec.data+i*cate_vec.size, sizeof(u64));
			memcpy(&weight, cate_vec.data+i*cate_vec.size+sizeof(u64), sizeof(int));
			sCatInfo.cid  = cate_id;
			iLev = GetClsLevel(cate_id);
			sCatInfo.iLev = iLev;
			sCatInfo.rate = weight;
			pa->vReferCat.push_back(sCatInfo);	
			total_rate += weight;
#ifdef DEBUG
			char cate_path[40];
			TranseID2ClsPath(cate_path, cate_id, 6);
			cout<<"id: "<<cate_id<<" feedback cat_paths: "<<cate_path<<endl;
#endif
		}
	}

	// adjust category weight
	for(int j = 0; j < pa->vReferCat.size(); j++)
	{
		if(j == pa->vReferCat.size() - 1)	
		{
			pa->vReferCat[j].rate = page_size - have_scatt;	
		}
		else
		{
			pa->vReferCat[j].rate = (int)(page_size * (float)(pa->vReferCat[j].rate/((float)total_rate)));	
			have_scatt += pa->vReferCat[j].rate ;
		}
	}
	
	//brand sort or no need scatter	
	if(pa->vReferCat.size() < 1 || pa->m_AnalysisPart.ifBrdQuery)	
	{
		// do somethine more 
		SortRange(vRes ,from, to);
	}
	else
	{
		//cout<<"pass Scatter"<<" query: "<<pa->m_AnalysisPart.key<<endl;
		ScatterCategory(vRes,iLimit,pa);
	}

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
//=============================================whj======================================================
u64 CSearchKeyRanking::JudgeResultMAinCate(vector<SResult>& vRes, int end)
{
	int pos = 0;
	u64 cid = 0;
	int main_cate = 0;
	int statistic_cate[2] = {0};
	int num = vRes.size() > end ? end : vRes.size();
	int pub = (int)TranseClsPath2ID("01",2);
	int b2c = (int)TranseClsPath2ID("58",2);
	for(int i=0; i<num; i++)
	{
		cid = m_funcFrstInt64(m_clsProfile,vRes[i].nDocId);
		main_cate = (int)GetClassByLevel(1,cid);
		//cout<<"doc:"<<vRes[i].nDocId<<"\t"<<"main cate:"<<main_cate<<endl;
		//统计出版物
		pos = (main_cate == pub ? 0 : (main_cate == b2c ? 1 : 2));
		if(pos == 2 || pos < 0 ) continue;
		statistic_cate[pos] += 1;
	}
	if(num > 10) statistic_cate[1] += 6;
	return statistic_cate[0]>statistic_cate[1] ? 0 : 1;
}

bool CSearchKeyRanking::SelectShowTemplate(CDDAnalysisData* pa,vector<SResult>& vRes, int end)
{
	int flag = 0;
	flag = JudgeResultMAinCate(vRes,end);
	if(flag == 0)
	{
		FindAndReplaceShowTempalte(pa->m_strReserve,"WEB:PUB_TEMPLATE");
	}
	else
	{
		FindAndReplaceShowTempalte(pa->m_strReserve,"WEB:BH_TEMPLATE");
	}
	return true;
}

void CSearchKeyRanking::FindAndReplaceShowTempalte(string& str_reserve,string str_template)
{
	if(str_template.empty())
		return;
	int pos = 0;
	string str_pub = "WEB:PUB_TEMPLATE";
	string str_b2c = "WEB:BH_TEMPLATE";
	string str_unknown = "WEB:UNKNOWN_TEMPLATE";
	if((pos = str_reserve.find(str_pub)) != string::npos)
	{
		str_reserve.erase(str_reserve.begin()+pos,str_reserve.begin()+pos+str_pub.size());
	}
	else if((pos = str_reserve.find(str_pub + ";")) != string::npos)
	{
		str_reserve.erase(str_reserve.begin()+pos,str_reserve.begin()+pos+str_pub.size() + 1);
	}
	else if((pos = str_reserve.find(str_b2c))   != string::npos)
	{
		str_reserve.erase(str_reserve.begin()+pos,str_reserve.begin()+pos+str_b2c.size());
	}
	else if((pos = str_reserve.find(str_b2c + ";"))   != string::npos)
	{
		str_reserve.erase(str_reserve.begin()+pos,str_reserve.begin()+pos+str_b2c.size() + 1);
	}
	else if((pos = str_reserve.find(str_unknown)) != string::npos)
	{
		str_reserve.erase(str_reserve.begin()+pos,str_reserve.begin()+pos+str_unknown.size());
	}
	else if((pos = str_reserve.find(str_unknown + ";")) != string::npos)
	{
		str_reserve.erase(str_reserve.begin()+pos,str_reserve.begin()+pos+str_unknown.size() + 1);
	}
	if(str_reserve.empty())
		str_reserve += str_template;
	else
		str_reserve += ";" + str_template;
}

bool CSearchKeyRanking::LoadCity2BitLocationDict(const string& city_file)
{
	m_city_blocation.clear();
	ifstream ifs(city_file.c_str());
	if (!ifs) {
		return false;
	}
	int blocation = 0;
	string line;
	string str_city = "";
	vector<string> str_vect;
	while(getline(ifs, line))
   	{
		str_vect.clear();
		boost::split( str_vect, line, boost::is_any_of( "\t" ), boost::token_compress_on );
		if(str_vect.size()!=2) 
			continue;
		str_city = str_vect[1];
		boost::trim(str_city);
		blocation = atoi(str_vect[0].c_str());
		m_city_blocation.insert(make_pair(str_city,blocation));
	}
	return true;
}

bool CSearchKeyRanking::LoadIp2LocationDict(const string& ip_file)
{
	m_ip_location = ddip_new();
	if(m_ip_location == NULL)
		return false;
	if(ddip_load(m_ip_location,ip_file.c_str())!=0)
	{
		return false;
	}
	return true;
}

bool CSearchKeyRanking::InitPersonalityLocationStockDict()
{
	if(!LoadIp2LocationDict(m_strModulePath + "ddip.bin.dat"))
	{
		//cout<<"load ip locat  error!"<<endl;
		COMMON_LOG(SL_ERROR, "load Ip 2 Loaction file failed");
		return false;
	}
	if(!LoadCity2BitLocationDict(m_strModulePath + "city_bit.txt"))
	{
		//cout<<"loadCity2BitLocation error!"<<endl;
		COMMON_LOG(SL_ERROR, "load city_bit.txt file failed");
		return false;
	}
	return true;
}

bool CSearchKeyRanking::GetUserIp2Location(CDDAnalysisData* pa)
{
	pa->m_bit_city_location = -1;   //如果运行错误 直接返回-1
	if(pa->m_hmUrlPara.find("ip") != pa->m_hmUrlPara.end())
	{
		string str_city = "";
		ddip_entry_t entry;
		string str_ip = pa->m_hmUrlPara["ip"];
		if(m_ip_location == NULL) return false;
		if(0 != ddip_find_iploc_by_str(m_ip_location,&entry,str_ip.c_str()))
		{
			return false;
		}
		//判断是否为空
		if(entry.loct[1] == NULL)
		{
			return false;
		}
		//获取城市级，并转为GBK编码
		if(false == gconv::_UTF8_to_GBK_(entry.loct[1],str_city))
		{
			return false;
		}
		hash_map<string,int>::const_iterator iter = m_city_blocation.find(str_city);
		if(iter!=m_city_blocation.end())
		{
			pa->m_bit_city_location = iter->second;
		}
		else
			return false;
		return true;
	}
	else
		return false;
}
bool CSearchKeyRanking::JudgeLocationStock(int doc_id,int bit_city_location,int& out) const
{
	out = 1;	
	if(doc_id < 0)
		return false;
	//得到的城市位 位置不对，则直接退出
	if(bit_city_location<0 || bit_city_location>63)
		return false;
	long long stock_status = m_funcFrstInt64(m_stockStatusProfile, doc_id) ;
	if(stock_status<0)
		return false;
	//相应区域是否有货
	out = (stock_status&(1<<bit_city_location)) == 0? 0:1;
	return true;
}

void CSearchKeyRanking::GetFrstMostElem(SResult& sr,vector<SResult> &vRes,int& frst_cnt,SScatterCategory& sc)
{
	if(vRes.size() < FISRT_CATE_LIMIT)
	{
		vRes.push_back(sr);
		frst_cnt++;
	}
	else
	{
		sort(vRes.begin(),vRes.end());
		for(int i = 0; i<vRes.size(); i++)
		{
			if(sr > vRes[i])
			{
				vRes[i] = sr;
				break;
			}
		}
		sc.cnt++;	
	}
}

void CSearchKeyRanking::ScatterCategory(vector<SResult>& vRes,int scatter_upper,CDDAnalysisData* pa)
{
	if(vRes.empty())				
		return ;

	vector<SScatterCategory>  vScatt;
	hash_map< long long,int > hIdToPos;
	hash_map< long long,int >::iterator it; 
		
	u64 clsId = 0;
	int iLev = 0;
	int i,j;
	int total = 0; 
	int ccnt = 0;
	int fst_cat = 0;
	int rel_score = 0;
	int is_pub = 0;
	int stock_status = 0;
	bool no_stock = true;
	SScatterCategory scat;
	vector<SReferCatInfo> &vCat = pa->vReferCat;
	
	vector<SResult> vFrstCat;
	hash_map<int,int> hFrstCat;
	hash_map<int,int>::iterator itFrst;
	vector<u64> vclsId(vRes.size());
			
#ifdef AB	
	TimeUtil tres1;
#endif
	int mm1 = 0;
	int mm2 = 0;
	//stat category cnt
	for(j = 0;j < vRes.size();j++)
	{
		rel_score = ((vRes[j].nScore >> 24) & 0x7);	
		/*is_pub  = m_funcFrstInt(m_isPublicationProfile,vRes[j].nDocId);
		stock_status = m_funcFrstInt(m_stockProfile,vRes[j].nDocId);
		if(is_pub && stock_status == 0)
		{
			no_stock = false;	
		}
		else
		{
			no_stock = true;		
		}*/

		vclsId[j] = m_funcFrstInt64(m_clsProfile,vRes[j].nDocId);
		for(i = 0;i < vCat.size();i++)
		{
			clsId = vCat[i].cid;	
			//iLev = GetClsLevel(clsId);
			if(!vCat[i].rate)
				continue;	
			//if(GetClassByLevel(vCat[i].iLev,m_funcFrstInt64(m_clsProfile,vRes[j].nDocId)) == clsId
			if(GetClassByLevel(vCat[i].iLev,vclsId[j]) == clsId
					//low score or pub no stock push in other category
					&& rel_score >= 2 && no_stock)			
			{
		/*if(vclsId[j]==19018328 ||vclsId[j] == 336933464)
		{
			cout<<"have vclsId!!"<<endl;
		}*/
				ccnt++;
				if((it = hIdToPos.find(clsId)) == hIdToPos.end())
				{
					memset(&scat,0x0,sizeof(SScatterCategory));
					hIdToPos.insert(make_pair(clsId,vScatt.size()));
					scat.cid = clsId;
					scat.sunit = vCat[i].rate;
					scat.iLev = vCat[i].iLev;
					vScatt.push_back(scat);
				}
				
				if(!i)	
				{
					 GetFrstMostElem(vRes[j],vFrstCat,fst_cat,vScatt[hIdToPos[clsId]]);
				}
				else
				{
					vScatt[hIdToPos[clsId]].cnt++;
				}
				break;
			}
		}
	}
	ccnt = ccnt - fst_cat;

#ifdef AB 
	int n1 = tres1.getPassedTime();
	cout<<"first get pos cost : "<<n1<<endl;
#endif
#ifdef AB	
	TimeUtil tres2;
#endif
	for(i = 0;i < vFrstCat.size(); i++)
	{
		hFrstCat.insert(make_pair(vFrstCat[i].nDocId,vFrstCat[i].nDocId));
	}
	memset(&scat,0x0,sizeof(SScatterCategory));
	hIdToPos.insert(make_pair(0,vScatt.size()));
	
	vector<SScatterCategory>::iterator itErase;
	for(itErase=vScatt.begin();itErase!=vScatt.end();)
	{
		if(!(*itErase).cnt)
		{
			itErase=vScatt.erase(itErase);
		}
		else
		{
			itErase++;
		}
	}
	scat.cnt = vRes.size() - ccnt - fst_cat;
	vScatt.push_back(scat);
	
	sort(vFrstCat.begin(),vFrstCat.end(),greater<SResult>());
	scatter_upper = scatter_upper - fst_cat;

	
	//get scatter docid 
	total = 0; 
	vector<int> vIdPos(vScatt.size());
	
	if(vRes.size() - fst_cat <= 0)
	{
		sort(vRes.begin(),vRes.end(),greater<SResult>());			
		return;
	}
	
	vector<SResult> vTmpRes(vRes.size() - fst_cat);
	bool bFlag = true;
	for(i = 0;i < vScatt.size();i++)
	{
		vIdPos[i] = total;
		total += vScatt[i].cnt;
	}
	
	vector<int> vTmpPos = vIdPos;
	for(j = 0;j < vRes.size();j++)
	{
		if((itFrst = hFrstCat.find(vRes[j].nDocId)) != hFrstCat.end())	
		{
			continue;
		}
		
		bFlag = false;	
		rel_score = ((vRes[j].nScore >> 24) & 0x7);	
		/*is_pub  = m_funcFrstInt(m_isPublicationProfile,vRes[j].nDocId);
		stock_status = m_funcFrstInt(m_stockProfile,vRes[j].nDocId);
		if(is_pub && stock_status == 0)
		{
			no_stock = false;	
		}
		else
		{
			no_stock = true;		
		}*/
		
		for(i = 0;i < vScatt.size() - 1;i++)
		{
			clsId = vScatt[i].cid;	
			//iLev = GetClsLevel(clsId);
			//if(GetClassByLevel(vScatt[i].iLev,m_funcFrstInt64(m_clsProfile,vRes[j].nDocId)) == clsId
			if(GetClassByLevel(vScatt[i].iLev,vclsId[j]) == clsId
					//low score or pub no stock push in other category
					&& rel_score >= 2 && no_stock)			
			{
				bFlag = true;	
				vTmpRes[vTmpPos[i]++] = vRes[j];
				break;
			}
		}
		if(!bFlag)
		{
			vTmpRes[vTmpPos[i]++] = vRes[j];	
		}

	}
#ifdef AB 
	int m1 = tres2.getPassedTime();
	cout<<"second get pos cost : "<<m1<<endl;
#endif
#ifdef AB 
	TimeUtil tscat;
#endif
	//get scatter count
	total = 0;
	if(ccnt <= scatter_upper)
	{
		for(i = 0;i < vScatt.size() - 1;i++)		
		{
			vScatt[i].scnt = vScatt[i].cnt;
		}
		vScatt[i].scnt = scatter_upper - ccnt;
	}
	else
	{
		while(total <= scatter_upper)	
		{
			for(i = 0;i < vScatt.size() - 1;i++)		
			{
				total = vScatt[i].scnt < vScatt[i].cnt ? total + (vScatt[i].scnt
						+ vScatt[i].sunit < vScatt[i].cnt ? vScatt[i].sunit:
						vScatt[i].cnt - vScatt[i].scnt) : total;

				vScatt[i].scnt = vScatt[i].scnt + vScatt[i].sunit < vScatt[i].cnt ?
					vScatt[i].scnt + vScatt[i].sunit : vScatt[i].cnt;

			}
		}
	}
#ifdef AB 
	int m7 = tscat.getPassedTime();
	cout<<"scatter cost : "<<m7<<endl;
#endif
#ifdef AB	
	TimeUtil tpart;
#endif
	// sort the data which need scatter
	SResult* beg,*end,*mid;	
	vector<int> vStartPos = vIdPos;
	for(i = 0; i < vScatt.size(); i++)
	{
		if(vScatt[i].cnt > 1)	
		{
			beg = &vTmpRes[vStartPos[i]];		
			end = beg + vScatt[i].cnt;
			mid = beg + vScatt[i].scnt;
			SimplePartialSortDesc(beg,mid,end);
		}
	}
#ifdef AB 
	int m2 = tpart.getPassedTime();
	cout<<"partial sort cost : "<<m2<<endl;
#endif

#ifdef AB 
	TimeUtil twinner;
#endif
	/*int n = 0;
	int m = 0;
	for(i = 0;i < vScatt.size() ; i++)                                                                            
	{    
		int m = vStartPos[i];
		int n = m + vScatt[i].scnt; 
		while(m!=n)
		{
			cout<<vTmpRes[m].nScore<<endl;
			m++; 
		}
		cout<<"cid: "<<vScatt[i].cid<<endl;
		cout<<endl;
	} */
	// collect each buck
	int col_num = vScatt.size() - 1;			
	int valid_col_num = vScatt.size() - 1;
	SResult dummy;
	int unit_base = 0;;
	dummy.nScore = -1 << 31;
	if(col_num % 2)
		col_num++;
	
	vector<_TREE_NODE<SResult, int> > buf(2 * col_num); 		
	vector<SResult*> sbeg(col_num);		
	vector<SResult*> send(col_num);
	total = 0;

	for(i = 0; i < valid_col_num; i++)
	{
		sbeg[i] = &vTmpRes[vStartPos[i]];
		unit_base = vScatt[i].scnt < vScatt[i].sunit ? vScatt[i].scnt
			: vScatt[i].sunit;
		send[i] = &vTmpRes[vStartPos[i]] + unit_base;
		vStartPos[i] = vStartPos[i] + unit_base ;
		total += unit_base;
	}
	
	if(col_num>valid_col_num)
	{
		sbeg[col_num - 1] = &dummy;	
		send[col_num - 1] = &dummy;
	}
	

	vector<_TREE_NODE<SResult, int> > dest(total);	
	int sorted_col=0;	
	int sorted_cnt = 0;
	
	//put first three SResult
	memcpy(&vRes[sorted_cnt],&vFrstCat[0],sizeof(SResult) * vFrstCat.size());			
	sorted_cnt += vFrstCat.size();

	vector<int> vResPos = vStartPos;
	//get scatter result union
	while(sorted_col < valid_col_num)
	{
		winner_tree_merge(&buf[0], &sbeg[0], &send[0], &dest[0], col_num, dummy);	
		for(i = 0; i < total; i++)
		{
			vRes[sorted_cnt++] = dest[i].n;	
		}

		//get new offset
		total = 0;
		for(i = 0;i < valid_col_num; i++)
		{
			if(vStartPos[i] == -1)	
				continue;
			if(vStartPos[i] >= vIdPos[i] + vScatt[i].scnt)
			{
				sorted_col++;
				sbeg[i] = &vTmpRes[0] + vStartPos[i];		
				vResPos[i] = vStartPos[i];
				vStartPos[i] = -1;
			}
			else
			{
				sbeg[i] = &vTmpRes[vStartPos[i]];		
				if(vStartPos[i] + vScatt[i].sunit>vIdPos[i] + vScatt[i].scnt)
				{
					unit_base = vIdPos[i] + vScatt[i].scnt - vStartPos[i];
				}
				else
				{
					unit_base = vScatt[i].sunit;
				}
				send[i] = &vTmpRes[vStartPos[i]] + unit_base;
				vStartPos[i] += unit_base ; 
				total += unit_base;
			}
			
		}
			
	} 
#ifdef AB 
	int m6 = twinner.getPassedTime();
	cout<<"winner tree cost : "<<m6<<endl;
#endif
#ifdef AB 
	TimeUtil tcollect;
#endif
	//collect sorted left data
	if(vScatt[vScatt.size() - 1].scnt)		
	{
		for(i = 0;i < vScatt[vScatt.size() - 1].scnt; i++)
		{
			vRes[sorted_cnt++] = vTmpRes[vStartPos[vScatt.size() - 1]+i];
		}
		vStartPos[vScatt.size() - 1] = vStartPos[vScatt.size() - 1] + vScatt[vScatt.size() - 1].scnt; 
	}
	
	//collect no sorted left data
	for(i = 0; i < vScatt.size(); i++)
	{
		if(vScatt[i].cnt <= vScatt[i].scnt||sorted_cnt+vScatt[i].cnt-vScatt[i].scnt>vRes.size())
		{
			continue;
		}

		memcpy(&vRes[sorted_cnt],&vTmpRes[vResPos[i]],sizeof(SResult) * 
				(vScatt[i].cnt - vScatt[i].scnt));
		sorted_cnt += vScatt[i].cnt - vScatt[i].scnt;
	}
#ifdef AB 
	int m4 = tcollect.getPassedTime();
	cout<<"collect cost time : "<<m4<<endl;
#endif
}

void CSearchKeyRanking::SimplePartialSortDesc(SResult* beg, SResult* mid, SResult* end)
{            
	if (mid - beg > 4)
	{        
		partial_sort(beg,mid,end,greater<SResult>());
	}        
	else     
	{        
		SResult mx,*m, *i, *j;
		for (i = beg; i < mid; ++i )
		{    
			m = i;
			for (j = i+1; j < end; ++j)
			{  
				if (*j > *m)
				{
					m = j;
				}
			}
			mx = *m;
			*m = *i;
			*i = mx;
		}    
	}        
} 

#ifndef _WIN32
// linux dll
CSearchKeyRanking search_ranking;
#endif

