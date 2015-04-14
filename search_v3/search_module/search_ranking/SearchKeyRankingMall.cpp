#include "SearchKeyRanking.h"
#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <iostream>
using namespace std;

#define TEST_BRAND_QUERY 0

static bool loadDict(const string& file, set<string>& dict) {
	ifstream ifs(file.c_str());
	if (!ifs) {
		//COMMON_LOG(SL_ERROR, "can't find file: %s", file.c_str());
		return false;
	}
	string line;
	while(getline(ifs, line)) {
		boost::trim(line);
		dict.insert(line);
	}
	return true;
}

bool CSearchKeyRanking::InitMall(SSearchDataInfo* psdi, const string& strConf)
{

	//加载词典
	if (false == loadDict(m_strModulePath + "product_word.txt", m_pdtWords)) {
		return false;
	}
	if (false == loadDict(m_strModulePath + "brand_word.txt", m_brdWords)) {
		return false;
	}

	//加载百货反馈数据
	m_key2Cate.read(m_strModulePath + "query_cate_direction");
	m_key2CateBrd.read(m_strModulePath + "query_cate_brand");
	m_key2Pid.read(m_strModulePath + "query_pid_direction");
	m_pid2Core.read(m_strModulePath + "pid_core_word");
	printf("init mall: load b2c feedback data\n");
	//加载混排数据
	string b2c_pub_rate_path(m_strModulePath + "b2c_pub_key_rate");
	if (false == m_percentPub2Mall.load_serialized_hash_file(b2c_pub_rate_path.c_str(), pair<int, int>(-1, -1))) {
		printf("failed to load b2c_pub_keyrate\n");
		return false;
	}
	printf("init merge data: load b2c_pub_keyrate\n");

	//字段编号
	TITLENAME = 9;
	TITLESYN = 8;
	BOTTOMCATE = 7;
	OTHERCATE = 6;
	BRAND = 5;
	SUBTITLE = 4;
	TMinNum = 4;
	m_fid2fi.resize(m_vFieldInfo.size());
	//根据配置文件search.conf获得字段名
	//COMMON_LOG(SL_INFO, "init mall: set field id");
	TITLE_FID = GetFieldId("product_name");
	if (TITLE_FID == -1) {
		COMMON_LOG(SL_ERROR, "product_name field does not exist");
		return false;
	} else {
		m_fid2fi[TITLE_FID] = TITLENAME;
	}
	COMMON_LOG(SL_INFO, "product_name field ok");

	TITLESYN_FID = GetFieldId("title_synonym");
	if (TITLESYN_FID == -1) {
		COMMON_LOG(SL_ERROR, "title_synonym field does not exist");
		return false;
	} else {
		m_fid2fi[TITLESYN_FID] = TITLESYN;
	}
	COMMON_LOG(SL_INFO, "title_synonym field ok");

	BRAND_FID = GetFieldId("brand_name");
	if (BRAND_FID == -1) {
		COMMON_LOG(SL_ERROR, "brand_name field does not exist");
		return false;
	} else {
		m_fid2fi[BRAND_FID] = BRAND;
	}
	COMMON_LOG(SL_INFO, "brand_name field ok");

	BOTTOMCATE_FID = GetFieldId("cat_paths_name");
	if (BOTTOMCATE_FID == -1) {
		COMMON_LOG(SL_ERROR, "cat_paths_name field does not exist");
		return false;
	} else {
		m_fid2fi[BOTTOMCATE_FID] = BOTTOMCATE;
	}
	COMMON_LOG(SL_INFO, "cat_paths_name field ok");
	
	OTHERCATE_FID = BOTTOMCATE_FID;
/*
	OTHERCATE_FID = GetFieldId("other_cat");
	if (OTHERCATE_FID == -1) {
		COMMON_LOG(SL_ERROR, "other_cat field does not exist");
		return false;
	} else {
		m_fid2fi[OTHERCATE_FID] = OTHERCATE;
	}
	COMMON_LOG(SL_INFO, "other_cat field ok");
*/
	SUBNAME_FID = GetFieldId("subname");
	if (SUBNAME_FID == -1) {
		COMMON_LOG(SL_ERROR, "subname field does not exist");
		return false;
	} else {
		m_fid2fi[SUBNAME_FID] = SUBTITLE;
	}
	COMMON_LOG(SL_INFO, "subname field ok");

	PRODUCTID_FID = GetFieldId("product_id");
	if (PRODUCTID_FID == -1) {
		COMMON_LOG(SL_ERROR, "id field does not exist");
		return false;
	}
	COMMON_LOG(SL_INFO, "product_id field ok");

	SHARE_PID_FID = GetFieldId("is_share_product");
	if (SHARE_PID_FID == -1) {
		COMMON_LOG(SL_ERROR, "is_share_product field does not exist");
		return false;
	}
	COMMON_LOG(SL_INFO, "is_share_product field ok");

	SALE_WEEK_FID = GetFieldId("sale_week");
	if (SALE_WEEK_FID == -1) {
		COMMON_LOG(SL_ERROR, "sale_week field does not exist");
		return false;
	}
	COMMON_LOG(SL_INFO, "sale_week field ok");

	SALE_WEEK_AMT_FID = GetFieldId("sale_week_amt");
	if (SALE_WEEK_AMT_FID == -1) {
		COMMON_LOG(SL_ERROR, "sale_week_amt field does not exist");
                return false;
	}
	INPUT_DATE_FID = GetFieldId("modifytime");//timestamp since 1970
	if (INPUT_DATE_FID == -1) {
		COMMON_LOG(SL_ERROR, "modifytime field does not exist");
		return false;
	}
	COMMON_LOG(SL_INFO, "modifytime field ok");

	COMMON_LOG(SL_INFO, "init mall: success");
	return true;
}

void CSearchKeyRanking::QueryAnalyseMall(CDDAnalysisData* pa, SQueryClause& qc)
{
	//初始化pa
	vector<QueryItem> queryItems;
	QueryParse(qc.key, qc.vTerms, queryItems);
	size_t queryKeyCnt = queryItems.size();		//query词数
	pa->m_mallAnalysisPart.type.resize(queryKeyCnt, 0);
	pa->m_mallAnalysisPart.keysRepeat.resize(queryKeyCnt, 0);
	pa->m_mallAnalysisPart.ifBrdQuery = false;
	pa->m_mallAnalysisPart.ifPdtQuery = false;
	pa->m_mallAnalysisPart.needJudgeDis = false;
	pa->m_mallAnalysisPart.relCnt_fb = 0;
	pa->m_mallAnalysisPart.relCnt_prd = 0;
	pa->m_mallAnalysisPart.relCnt_brd = 0;
	pa->m_mallAnalysisPart.relCnt_ti = 0;
	//@todo: pa中还有其它变量需要初始化

	int pdtCnt = 0;				//产品词数
	bool hasBrd = false;	//有品牌词
	bool hasCom = false;	//有一般词
	string pdtWord = "";	//产品词（唯一）
	bool appearDisFeature = false;

	for (size_t i = 0; i < queryKeyCnt; ++i)
	{
		string key = queryItems[i].word;
		pa->m_mallAnalysisPart.querystr += key;

		if (m_pdtWords.find(key) != m_pdtWords.end())
		{	//产品词
			pa->m_mallAnalysisPart.type[i] = 1;
			++pdtCnt;
			pdtWord = key;
		}
		else if (m_brdWords.find(key) != m_brdWords.end())
		{	//品牌词
			pa->m_mallAnalysisPart.type[i] = 2;
			hasBrd = true;
		}
		else
		{	//一般词
			pa->m_mallAnalysisPart.type[i] = 0;
			hasCom = true;
		}

		if (0 == pa->m_mallAnalysisPart.keysRepeat[i])
		{	//重复词检验及记录
			//int id = queryItems[i].keyID;
			string word = queryItems[i].word;
			int count = 0;
			for (size_t j = i+1; j < queryKeyCnt; ++j)
			{
				//if (id == queryItems[j].keyID)
				if (word == queryItems[j].word)
				{
					pa->m_mallAnalysisPart.keysRepeat[j] = ++count + (i << 8);
				}
			}
		}

		if (i > 0 && 0 == queryItems[i].fwdDis &&
			0 == pa->m_mallAnalysisPart.type[i-1] && 0 == pa->m_mallAnalysisPart.type[i])
		{	//连续两个一般词需计算词距
			appearDisFeature = true;
		}
	}
	if (true == pa->m_mallAnalysisPart.needJudgeDis &&
		(pa->m_mallAnalysisPart.querystr.size() > 12 || false == appearDisFeature) )
	{	//是否需计算词距(长query或未出现需要计算词距特征则不计算词距)
		pa->m_mallAnalysisPart.needJudgeDis = false;
	}

	int cate_count = m_key2Cate[pa->m_mallAnalysisPart.querystr].count;
	if (cate_count != 0)
	{	//类别反馈
		HASHVECTOR cate_vec = m_key2Cate[pa->m_mallAnalysisPart.querystr];
		for (int k=0; k<cate_count; k++) {
			u64 cate_id = 0;
			int weight = 0;
			memcpy(&cate_id, cate_vec.data+k*cate_vec.size, sizeof(u64));
			memcpy(&weight, cate_vec.data+k*cate_vec.size+sizeof(u64), sizeof(int));
			pa->m_mallAnalysisPart.vCate.push_back(pair<u64, int>(cate_id, weight));
			{
				char cate_path[40];
				TranseID2ClsPath(cate_path, cate_id, 6);
			}
		}
	}
	int pid_count = m_key2Pid[pa->m_mallAnalysisPart.querystr].count;
	if (pid_count != 0)
	{	//单品反馈
		HASHVECTOR pid_vec = m_key2Pid[pa->m_mallAnalysisPart.querystr];
		for (int k=0; k<pid_count; k++) { //获取query的反馈pid
			int pid = 0;
			memcpy(&pid, pid_vec.data+k*pid_vec.size, 4);
			pa->m_mallAnalysisPart.vPid.push_back(pid);
		}
	}

	if ( m_pdtWords.find(pa->m_mallAnalysisPart.querystr) != m_pdtWords.end()
		|| (1 == pdtCnt && hasBrd && !hasCom) )
	{	//产品搜索/品牌+产品搜索
		pa->m_mallAnalysisPart.ifPdtQuery = true;
		pa->m_mallAnalysisPart.needJudgeDis = false;

		//表示当前query只到倒排索引的标题, 类别和品牌字段中搜索
		pa->m_queryFieldIds.push_back(TITLE_FID);
		pa->m_queryFieldIds.push_back(SUBNAME_FID);
		pa->m_queryFieldIds.push_back(TITLESYN_FID);
		pa->m_queryFieldIds.push_back(BRAND_FID);
		pa->m_queryFieldIds.push_back(BOTTOMCATE_FID);

		pa->m_mallAnalysisPart.pdtWord = m_pdtWords.find(pa->m_mallAnalysisPart.querystr) != m_pdtWords.end() ?
			pa->m_mallAnalysisPart.querystr : pdtWord;

		if ( pa->m_mallAnalysisPart.pdtWord.size() > 2
			&& pa->m_mallAnalysisPart.pdtWord[pa->m_mallAnalysisPart.pdtWord.size() - 2] < 0)
		{	//汉语语素，即最后一个字
			string tmpNg = pa->m_mallAnalysisPart.querystr.substr(
				pa->m_mallAnalysisPart.querystr.size() - 2, 2);
			if (m_pdtWords.find(tmpNg) != m_pdtWords.end())
			{	//产品语素, 也就是产品词的最后一个字
				pa->m_mallAnalysisPart.pdtNg = tmpNg;
			}
		}
	}
	else if (m_brdWords.find(pa->m_mallAnalysisPart.querystr) != m_brdWords.end()
		|| (hasBrd && !hasCom && 0 == pdtCnt) )
	{	//品牌搜索(整个query是品牌或者query中所有词都是品牌词)
		pa->m_mallAnalysisPart.ifBrdQuery = true;
		pa->m_mallAnalysisPart.needJudgeDis = false;

		//表示当前query只到倒排索引的标题, 类别和品牌字段中搜索
		pa->m_queryFieldIds.push_back(TITLE_FID);
		pa->m_queryFieldIds.push_back(SUBNAME_FID);
		pa->m_queryFieldIds.push_back(TITLESYN_FID);
		pa->m_queryFieldIds.push_back(BRAND_FID);

		//map<string, vector<pair<int,int> > >::const_iterator iter =
		//	m_key2CateBrd.find(pa->m_mallAnalysisPart.querystr);
		HASHVECTOR vCat = m_key2CateBrd[pa->m_mallAnalysisPart.querystr];
		//if (iter != m_key2CateBrd.end())
		int cate_count = vCat.count;
		if (cate_count != 0) //对品牌query的反馈类别编号
		{	//有品牌反馈
			int num = 0;
			u64 cate = 0;
			int weight = 0;
			//for (size_t c = 0; c < iter->second.size(); ++c)
			for (size_t c = 0; c < cate_count; ++c)
			{
				//cate = iter->second[c].first;
				//cate = vCat[c].first;
				//weight = iter->second[c].second;
				//weight = vCat[c].second;
				memcpy(&cate, vCat.data+c*vCat.size, sizeof(u64));
				memcpy(&weight, vCat.data+c*vCat.size+sizeof(u64), sizeof(int));
				if (pa->m_mallAnalysisPart.cid2cnum.find(cate) == pa->m_mallAnalysisPart.cid2cnum.end())
				{
					pa->m_mallAnalysisPart.cid2cnum[cate] = num;
					pa->m_mallAnalysisPart.cateStat.push_back(CateStat());
					if (num < pa->m_mallAnalysisPart.cateStat.size())
					{
						pa->m_mallAnalysisPart.cateStat[num].cateScore = weight; //类别的反馈权重
						++num;
					}
				}
			}
			if (num != (int)pa->m_mallAnalysisPart.cateStat.size())
			{	//check
				COMMON_LOG(SL_ERROR, "brand-query:%s logic error!", pa->m_mallAnalysisPart.querystr.c_str());
				pa->m_mallAnalysisPart.cid2cnum.clear();
				pa->m_mallAnalysisPart.cateStat.clear();
			}
		}
	} else {
		//对于其它情况，当前query到所有字段中搜索
		pa->m_queryFieldIds.push_back(TITLE_FID);
		pa->m_queryFieldIds.push_back(SUBNAME_FID);
		pa->m_queryFieldIds.push_back(TITLESYN_FID);
		pa->m_queryFieldIds.push_back(BRAND_FID);
		pa->m_queryFieldIds.push_back(BOTTOMCATE_FID);
	}
}

//过滤0值结果
static inline bool filter_zero_weight(const SResult& di)
{
	return di.nScore <= 0;
}

void CSearchKeyRanking::FilterMallResults(vector<SResult>& vRt) {
	size_t count = vRt.size();
	for (size_t i = 0; i < count; ++i) {
		if (vRt[i].nWeight/TypeIndicator == 0) {//如果是出版物，跳过，用于混排精简
			continue;
		}
		if (0 == (vRt[i].nScore / BaseRelFactor & 1)) {
			vRt[i].nScore = 0;//不相关
		}
	}
	vRt.erase(std::remove_if(vRt.begin(), vRt.end(), filter_zero_weight), vRt.end());
}

void CSearchKeyRanking::CombineComputeWeight_ComQ(
	CDDAnalysisData* pa,
	vector<SResult>& vRt)
{	//一般搜索

	//修改BaseRel, 为精简准备
	//精简条件：相关商品数>=20 && 相关商品的顶级类别集合不为空
	if (pa->m_mallAnalysisPart.relCnt_fb >= 20 && false == pa->m_mallAnalysisPart.topCate_fb.empty())
	{	//可做精简
		set<u64> &RelCate = pa->m_mallAnalysisPart.topCate_fb; //只信任反馈
		for (size_t i = 0; i < vRt.size(); ++i)
		{
			if (vRt[i].nWeight/TypeIndicator == 0) {//如果是出版物，跳过，用于混排精简
				continue;
			}
			if (vRt[i].nWeight / RelIndicator % 10 != 2)		//不匹配反馈
			{	//不相关嫌疑
				bool ifRel = false;
				int docID = vRt[i].nDocId;
				size_t nCnt = 0;
				u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针

				for (int k=0; k<nCnt; k++) {
					u64 top_cate = GetClassByLevel(1, ptr[k]); //获取当前类别的顶级类别
					if (RelCate.find(top_cate)!=RelCate.end()) //@fixed bug: if catePath.size()=0
					{
						ifRel = true;
						break;
					}
				}

				if (false == ifRel && vRt[i].nScore > BaseRelFactor) //排序权重
				{	//标记不相关
					vRt[i].nScore -= BaseRelFactor;
				}
			}
		}
	}
}

void CSearchKeyRanking::CombineComputeWeight_PrdQ(
	CDDAnalysisData* pa,
	vector<SResult>& vRt)
{	//含品类搜索
	//修改BaseRel, 为精简准备
	if (pa->m_mallAnalysisPart.relCnt_fb + pa->m_mallAnalysisPart.relCnt_prd >= 20)
	{	//可做精简
		bool RelType = true;	//只匹配反馈类型
		set<u64> &RelCate = pa->m_mallAnalysisPart.topCate_fb; //只信任反馈
		if (pa->m_mallAnalysisPart.relCnt_fb < 20)
		{	//信任反馈和产品中心
			RelType = false;		//只匹配反馈和产品中心类型
			//cout << "insert" << endl;
			RelCate.insert(pa->m_mallAnalysisPart.topCate_prd.begin(), pa->m_mallAnalysisPart.topCate_prd.end());
		}
		if (false == RelCate.empty())
		{
			for (size_t i = 0; i < vRt.size(); ++i)
			{
				if (vRt[i].nWeight/TypeIndicator == 0) {//如果是出版物，跳过，用于混排精简
					continue;
				}
				int relScr = vRt[i].nWeight / RelIndicator % 10;	//精简相关标识
				if ( (true == RelType && relScr != 2)		//不匹配反馈
					|| (false == RelType && relScr < 1) )	//不匹配反馈或产品中心
				{	//不相关嫌疑
					bool ifRel = false;
					int docID = vRt[i].nDocId;
					size_t nCnt = 0;
					u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针

					for (int k=0; k<nCnt; k++) {
						u64 top_cate = GetClassByLevel(1, ptr[k]); //获取当前类别的顶级类别
						if (RelCate.find(top_cate)!=RelCate.end()) //@fixed bug: if catePath.size()=0
						{
							//cout << "find the top_cate=" << top_cate << endl;
							ifRel = true;
							break;
						}
					}
					if (false == ifRel && vRt[i].nScore > BaseRelFactor)
					{	//标记不相关
						//out[i].weight -= BaseRelFactor;
						vRt[i].nScore -= BaseRelFactor;
					}
				}
			}
		}
	}
}

void CSearchKeyRanking::CombineComputeWeight_BrdQ(
	CDDAnalysisData* pa,
	vector<SResult>& vRt)
{	//品牌搜索

	//修改BaseRel, 为精简准备
	if (pa->m_mallAnalysisPart.relCnt_brd + pa->m_mallAnalysisPart.relCnt_ti >= 20)
	{	//可做精简
		bool RelType = true;					 //只匹配品牌字段类型
		set<u64> &RelCate = pa->m_mallAnalysisPart.topCate_brd; //只信任品牌字段匹配
		if (pa->m_mallAnalysisPart.relCnt_brd < 20)
		{	//信任品牌或标题字段匹配
			RelType = false;		//只匹配品牌与标题字段类型
			RelCate.insert(pa->m_mallAnalysisPart.topCate_ti.begin(), pa->m_mallAnalysisPart.topCate_ti.end());
		}
		if (false == RelCate.empty())
		{
			for (size_t i = 0; i < vRt.size(); ++i)
			{
				if (vRt[i].nWeight/TypeIndicator == 0) {//如果是出版物，跳过，用于混排精简
					continue;
				}
				//int relScr = out[i].textWeight / RelIndicator % 10;	//精简相关标识
				int relScr = vRt[i].nWeight / RelIndicator % 10; //调试权重
				if ( (true == RelType && relScr != 2)		//不匹配品牌
					|| (false == RelType && relScr < 1) )	//不匹配品牌或标题
				{	//不相关嫌疑
					bool ifRel = false;
					int docID = vRt[i].nDocId;
					size_t nCnt = 0;
					u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针

					for (int k=0; k<nCnt; k++) {
						u64 top_cate = GetClassByLevel(1, ptr[k]); //获取当前类别的顶级类别
						if (RelCate.find(top_cate)!=RelCate.end()) //@fixed bug: if catePath.size()=0
						{
							ifRel = true;
							break;
						}
					}

					if (false == ifRel && vRt[i].nScore > BaseRelFactor)
					{	//标记不相关
						//out[i].weight -= BaseRelFactor;
						vRt[i].nScore -= BaseRelFactor;
					}
				}
			}
		}
	}

	//精排(top30)
	//if (!out.empty() && pa->m_mallAnalysisPart.cateStat.size() > 1)
	if (!vRt.empty() && pa->m_mallAnalysisPart.cateStat.size() > 1)
	{
		static const int Weight[3] = {10, 4, 6};	//cateFB/pidCnt/sale
		{
			int pidMaxCnt = 0;
			int cateMaxWeight = 0;
			int saleMax = 0;
			//collect
			for (size_t c = 0; c < pa->m_mallAnalysisPart.cateStat.size(); ++c)
			{
				if (pa->m_mallAnalysisPart.cateStat[c].pidCnt)
				{
					if (pa->m_mallAnalysisPart.cateStat[c].pidCnt > pidMaxCnt)
					{
						pidMaxCnt = pa->m_mallAnalysisPart.cateStat[c].pidCnt; //类别的商品数
					}
					if (pa->m_mallAnalysisPart.cateStat[c].cateScore > cateMaxWeight)
					{
						cateMaxWeight = pa->m_mallAnalysisPart.cateStat[c].cateScore; //类别的反馈权重
					}
					if (pa->m_mallAnalysisPart.cateStat[c].saleSum > saleMax)
					{
						saleMax = pa->m_mallAnalysisPart.cateStat[c].saleSum; //类别中的商品销量
					}
				}
			}
			//normalize
			multimap<int, size_t, greater<int> > scr2idx;
			for (size_t c = 0; c < pa->m_mallAnalysisPart.cateStat.size(); ++c)
			{
				if (pa->m_mallAnalysisPart.cateStat[c].pidCnt)
				{
					if (pidMaxCnt > 0)
					{
						pa->m_mallAnalysisPart.cateStat[c].pidCnt =
							pa->m_mallAnalysisPart.cateStat[c].pidCnt * 100 / pidMaxCnt;
					}
					if (cateMaxWeight > 0)
					{
						pa->m_mallAnalysisPart.cateStat[c].cateScore =
							pa->m_mallAnalysisPart.cateStat[c].cateScore * 100 / cateMaxWeight;
					}
					if (saleMax > 0)
					{
						pa->m_mallAnalysisPart.cateStat[c].saleSum =
							pa->m_mallAnalysisPart.cateStat[c].saleSum * 100 / saleMax;
					}

					pa->m_mallAnalysisPart.cateStat[c].cateScore =
						(Weight[0] * pa->m_mallAnalysisPart.cateStat[c].cateScore +
						Weight[1] * pa->m_mallAnalysisPart.cateStat[c].pidCnt +
						Weight[2] * pa->m_mallAnalysisPart.cateStat[c].saleSum) / 100;

					scr2idx.insert(
						make_pair(pa->m_mallAnalysisPart.cateStat[c].cateScore, c));
				}
				else
				{
					pa->m_mallAnalysisPart.cateStat[c].cateScore = 0;
				}
			}
			//smooth
			if (scr2idx.size() < 2 || scr2idx.begin()->first <= 0)
			{
				return;
			}
			//前10个结果中必出现两个类，前30个结果中必出现3个类
			if (TEST_BRAND_QUERY)
			{
				cout << "RATE:";
			}
			if (2 == scr2idx.size())
			{
				multimap<int, size_t, greater<int> >::iterator mit = scr2idx.begin();
				int ff = mit->first; //类别score
				size_t fs = mit->second; //类别编号
				++mit;
				int sf = mit->first; //下一个类别score
				size_t ss = mit->second; //下一个类别编号

				//对前两个类别重新分配score
				if (!sf || ff / sf > 4)
				{
					ff = 8;
					sf = 2;
				}
				else
				{
					ff = ff * 10 / (ff + sf);
					sf = sf * 10 / (ff + sf);
				}

				if (TEST_BRAND_QUERY)
				{
					cout << " " << ff << " " << sf << endl;
				}

				pa->m_mallAnalysisPart.cateStat[fs].camp.push_back(ff);
				pa->m_mallAnalysisPart.cateStat[fs].camp.push_back(2*ff);
				pa->m_mallAnalysisPart.cateStat[ss].camp.push_back(sf);
				pa->m_mallAnalysisPart.cateStat[ss].camp.push_back(2*sf);
			}
			else
			{	//多类
				multimap<int, size_t, greater<int> >::iterator fst,snd,thd;
				fst = snd = scr2idx.begin();
				thd = ++snd;
				++thd;
				int ff = fst->first;
				int sf = snd->first;
				int tf = thd->first;
				//对前三个类别重新分配score
				if (!tf)
				{
					tf = 1;
					if (!sf)
					{
						sf = 1;
					}
				}
				if (sf > 2 * tf)
				{
					sf = 2 * tf;
				}
				if (ff > 3 * sf)
				{
					ff = 3 * sf;
				}

				int sum = ff + sf + tf;
				int campCnt = 0;
				int loopNum = 0;

				for (multimap<int, size_t, greater<int> >::iterator sit
					= scr2idx.begin(); sit != scr2idx.end(); ++sit)
				{
					if (sit->second < pa->m_mallAnalysisPart.cateStat.size())
					{
						int val = sit->first;
						if (++loopNum <= 3)
						{
							val = 1 == loopNum ? ff : (2 == loopNum ? sf : tf); //前3个类别中某个的权重
						}

						if (TEST_BRAND_QUERY)
						{
							cout << " " << val;
						}

						if (val * 10 / sum > 0)
						{	//前10阵营
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(val * 10 / sum);
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(val * 10 / sum);
							campCnt += val * 20 / sum;
						}
						else if (val * 30 / sum > 0)
						{	//前30阵营
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(0);
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(val * 30 / sum);
							campCnt += val * 30 / sum;
						}
						else if (campCnt < 30)
						{
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(0);
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(1);
							++campCnt;
						}
					}
				}
				if (TEST_BRAND_QUERY)
				{
					cout << endl;
				}
			}
		}
		//tune
		{
			//size_t TopN = 500 <= out.size() ? 500 : out.size();
			size_t TopN = 500 <= vRt.size() ? 500 : vRt.size();
			std::partial_sort(
					vRt.begin(), vRt.begin() + TopN, vRt.end(),
					boost::bind(
						std::greater<int>(),
						//boost::bind(&QueryRsItem::weight, _1),
						//boost::bind(&QueryRsItem::weight, _2)));
						boost::bind(&SResult::nScore, _1), //排序权重
						boost::bind(&SResult::nScore, _2)));

			int cateNum;
			for (size_t i = 0; i < TopN; ++i)
			{
				//cateNum = out[i].textWeight % 100;
				cateNum = vRt[i].nWeight % 100; //调试权重，类别反馈权重
				if ( cateNum > 0 && cateNum-1 < (int)pa->m_mallAnalysisPart.cateStat.size()
					&& false == pa->m_mallAnalysisPart.cateStat[cateNum-1].camp.empty())
				{
					vector<int>& camp = pa->m_mallAnalysisPart.cateStat[cateNum-1].camp;
					for (size_t c = 0; c < camp.size() && c < 2; ++c)
					{
						if (camp[c] > 0)
						{
							//out[i].weight += (2-c) * 100 +
							//	pa->m_mallAnalysisPart.cateStat[cateNum-1].cateScore * 10;
							vRt[i].nScore += (2-c) * 100 +             //排序权重
								pa->m_mallAnalysisPart.cateStat[cateNum-1].cateScore * 10;
							//out[i].textWeight += (c+1) * 100;	//阵营号
							--camp[c];
							break;
						}
					}
				}
			}
		}
	}
	return;
}

void CSearchKeyRanking::SingleRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt) {
	//cout << "enter SingleRank..." << endl;
	if (1 != me.vTerms.size()) {
		return;
	}

	//基本信息
	int docID = rt.nDocId;
	//cout << "docID = " <<docID << endl;
	//fprintf(stderr,"Single docID=%d\n",docID);

	//字段信息
	int fieldScr = 0;									//字段得分[0-4]
	bool ifContainAll_T = false;			//是否在重要字段出现
	bool ifContainAll_ti = false;			//标题是否出现
	bool ifContainAll_syn = false;		//在同义标题出现(仅同义字段)
	bool ifContainAll_bottom = false;	//在底级类别中出现
	bool ifContainAll_upper = false;	//在高级类别中出现
	bool ifInBrand = false;						//品牌字段支持
	bool ifDisMatch = true;					  //词距是否匹配
	{
		vector<SFieldOff> fieldPtr = me.vFieldsOff; //出现在哪些字段
		int ivtCnt = fieldPtr.size();
		int fid, fieldNum;
		for (int cnt = 0; cnt < ivtCnt; ++cnt)
		{
			//cout << "cnt = " << cnt << endl;
			fid = fieldPtr[cnt].field;
			if (m_fid2fi[fid] >= 0)
			{
				fieldNum = m_fid2fi[fid];
				if (fieldNum >= TMinNum)
				{	//主区
					ifContainAll_T = true;

					if (fieldNum == TITLENAME)
					{ //标题
						ifContainAll_ti = true;
					}
					else if (fieldNum == TITLESYN)
					{ //同义扩展
						if (false == ifContainAll_ti)
						{
							ifContainAll_syn = true;
						}
				  }
					else if (fieldNum == BOTTOMCATE)
					{ //底级类
						ifContainAll_bottom = true;
					}
					else if (fieldNum == OTHERCATE)
					{ //其他级类
						ifContainAll_upper = true;
					}
					else if (fieldNum == BRAND &&
						(true == pa->m_mallAnalysisPart.ifBrdQuery || 2 == pa->m_mallAnalysisPart.type[0]) )
					{	//品牌字段支持
						ifInBrand = true;
					}
				}
			}
		}
		if (ifContainAll_T)
		{
			if (ifContainAll_ti)
			{
				fieldScr = 4;
			}
			else if (ifContainAll_syn || ifContainAll_bottom)
			{
				fieldScr = 3;
			}
			else if (ifContainAll_upper)
			{
				fieldScr = 1;
			}
		}
	}
	//特殊信息(类别反馈/单品反馈/产品中心)
	int relScr = 0;						//精简相关标识[0-2]
	int fbCateWeight = 0;
	int fbPidWeight = 0;
	int pdtCoreWeight = 0;
	{
		ComputeSpecialWeight(
			docID, pa, fbCateWeight, fbPidWeight, pdtCoreWeight, relScr);
	}

	//商业信息(商业权重/销量细节)
	int commerceScr = 0;
	int saleDetail = 0;
	{
		ComputeCommerceWeight(docID, commerceScr, saleDetail);
	}

	//其他信息(公用品)
	bool ifPublic = false;
	{
		if (1 == m_funcFrstInt(m_isShareProductProfile, docID)) {
			ifPublic = true;
		}
	}
	//用于排序的权重
	rt.nScore += FakeScore                                    //FakeScore = 1101000000;
	+ ifContainAll_T * BaseRelFactor  //BaseRelFactor = 10000000;
	+ fbCateWeight	* Weight_FBCate   //Weight_FBCate = 100;
	+ fbPidWeight * Weight_FBPid      //Weight_FBPid = 10;
	+ fieldScr * Weight_Field         //Weight_Field = 60;
	+ pdtCoreWeight * Weight_PdtCore  //Weight_PdtCore = 25;
	+ ifDisMatch * Weight_Dis         //Weight_Dis = 30;
	+ ifInBrand * Weight_Brand        //Weight_Brand = 30;
	+ commerceScr * Weight_Commerce   //Weight_Commerce = 0;
	+ ifPublic * Weight_Public        //Weight_Public = 10;
	+ saleDetail * Weight_Sale;       //Weight_Sale = 0;
	//用户调试的权重
	rt.nWeight += commerceScr*CommerceFactor                  //CommerceFactor = 0;
	+ relScr*RelIndicator     //RelIndicator = 10000;
	+ fieldScr*FieldFactor    //FieldFactor = 1000;
	+ fbCateWeight*FeedbackCateFactor    //FeedbackCateFactor = 100;
	+ fbPidWeight*FeedbackPidFactor      //FeedbackPidFactor = 10;
	+ pdtCoreWeight*ProdCentreFactor;    //ProdCentreFactor = 1;
	return;
}

void CSearchKeyRanking::MultiRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt) {
	//cout << "enter MultiRank..." << endl;
	int querySz = (int)me.vTerms.size();
	if (querySz < 2)
	{
		return;
	}

	//基本信息
	int docID = rt.nDocId;

	//字段信息
	int fieldScr = 0;									//字段得分[0-4]
	bool ifContainAll_T = false;			//重要字段是否出全
	bool ifContainAll_ti = false;			//标题是否出全(仅标题)
	bool ifContainAll_syn = false;		//标题是否出全(标题+同义扩展)
	bool ifContainAll_bottom = false;	//底级类别中是否出全
	bool ifContainAll_upper = false;	//高级类别中是否出全
	bool ifInBrand = false;						//品牌字段支持
	bool ifDisMatch = true;						//词距是否匹配（标题）
	{
		int sz_T = 0, sz_ti = 0, sz_sys = 0;	//主区/标题/仅同义标题
		int sz_b = 0, sz_ob = 0, sz_u = 0; 	  //底级类/仅底级类/高级类出词数
		vector<vector<int> > vPosTi(querySz);	//标题出词位置记录
		{
			//Invert* ivtAddr;
			int ivtCnt;
			int fid, fieldNum;
			bool added_T, added_ti, added_syn, added_b, added_u;

			for (int k = 0; k < querySz; ++k)
			{	//for one term
				added_T = added_ti = added_syn = added_b = added_u = false;

				//{
					fid = me.vFieldsOff[k].field;
					if (m_fid2fi[fid] >= 0)
					{
						fieldNum = m_fid2fi[fid];
						if (fieldNum >= TMinNum)
						{	//主区
							if (false == added_T)
							{
								++sz_T;
								added_T = true;
							}
							if (fieldNum == TITLENAME)
							{ //标题
								if (false == added_ti)
								{
									++sz_ti;
									added_ti = true;
								}
								//vPosTi[k].push_back(ivtAddr[cnt].offset);
								vPosTi[k].push_back(me.vFieldsOff[k].off);
							}
							else if (fieldNum == TITLESYN)
							{ //同义扩展
								if (false == added_ti && false == added_syn)
								{
									++sz_sys;
									added_syn = true;
								}
						    }
							else if (fieldNum == BOTTOMCATE)
							{ //底级类
								if (false == added_b)
								{
									++sz_b;
									added_b = true;
									if (false == added_ti && false == added_syn)
									{
										++sz_ob;
									}
								}
							}
							else if (fieldNum == OTHERCATE)
							{ //其他级类
								if (false == added_u)
								{
									++sz_u;
									added_u = true;
								}
							}
							else if (fieldNum == BRAND &&
								(pa->m_mallAnalysisPart.ifBrdQuery || 2 == pa->m_mallAnalysisPart.type[k]))
							{	//品牌支持
								ifInBrand = true;
								if (false == added_ti)
								{
									++sz_ti;
									added_ti = true;
								}
							}
						}
					}
					//++cnt;
				//}
			}
		}

		if (sz_T == querySz)
		{
			ifContainAll_T = true;
			if (sz_ti == querySz)
			{
				ifContainAll_ti = true;
				fieldScr = 4;
			}
			else if (sz_ti + sz_sys == querySz)
			{
				ifContainAll_syn = true;
				fieldScr = 3;
			}
			else if (sz_b == querySz)
			{
				ifContainAll_bottom = true;
				fieldScr = 3;
			}
			else if (sz_ti + sz_sys + sz_ob == querySz)
			{
				fieldScr = 2;
			}
			else if (sz_u == querySz)
			{
				ifContainAll_upper = true;
				fieldScr = 1;
			}
		}

		if (true == pa->m_mallAnalysisPart.needJudgeDis)
		{	//判断词距匹配
			if (false == ifContainAll_ti)
			{
				ifDisMatch = false;
			}
			else
			{
				for (int k = 1; k < querySz; ++k)
				{
					//if (0 == vKey[k].dis &&
					//if (0 == pa->m_mallAnalysisPart.vDis[k] &&     //vDis表示两个词之间的标点符号长度，见query_parser_segment.h
					                                                 //中的fwdDis, 这里暂时先假定为0
					if(0 == pa->m_mallAnalysisPart.type[k-1] && 0 == pa->m_mallAnalysisPart.type[k]) //均为一般词
					{
						bool findMatch = false;
						for (size_t l = 0; l < vPosTi[k-1].size() &&
							false == findMatch; ++l)
						{
							//int len = vKey[k].keyLength;
							int len = me.vFieldLen[k];
							for (size_t r = 0; r < vPosTi[k].size(); ++r)
							{
								if (abs(vPosTi[k][r] - vPosTi[k-1][l] - len) <= 2)
								{
									findMatch = true;
									break;
								}
							}
						}
						if (false == findMatch)
						{
							ifDisMatch = false;
							break;
						}
					}
				}
			}
		}
	}

	//商业信息(商业权重/销量细节)
	int commerceScr = 0; //[0-6]
	int saleDetail = 0;	 //[0-30]
	{
		ComputeCommerceWeight(docID, commerceScr, saleDetail);
	}

	//其他信息(公用品)
	bool ifPublic = false;
	{
		if (1 == m_funcFrstInt(m_isShareProductProfile, docID))
		{
			ifPublic = true;
		}
	}

	//特殊信息(类别反馈/单品反馈/产品中心)
	int relScr = 0;						//精简相关标识[0-2]
	int fbCateWeight = 0;
	int fbPidWeight = 0;
	int pdtCoreWeight = 0;
	{

		ComputeSpecialWeight(
			docID, pa, fbCateWeight, fbPidWeight, pdtCoreWeight, relScr);
	}
	//排序实际权重
	rt.nScore += FakeScore
		+ ifContainAll_T * BaseRelFactor
		+ fbCateWeight	* Weight_FBCate
		+ fbPidWeight * Weight_FBPid
		+ fieldScr * Weight_Field
		+ pdtCoreWeight * Weight_PdtCore
		+ ifDisMatch * Weight_Dis
		+ ifInBrand * Weight_Brand
		+ commerceScr * Weight_Commerce
		+ ifPublic * Weight_Public
		+ saleDetail * Weight_Sale;
	//调试
	rt.nWeight += commerceScr*CommerceFactor
		+ relScr*RelIndicator
		+ fieldScr*FieldFactor
		+ fbCateWeight*FeedbackCateFactor
		+ fbPidWeight*FeedbackPidFactor
		+ pdtCoreWeight*ProdCentreFactor;
	return;
}

void CSearchKeyRanking::ComputeWeightMall(CDDAnalysisData* pa, SMatchElement& me, SResult& rt) {
	rt.nDocId = me.id;
	rt.nWeight = rt.nScore = 0;
	//cout << ">>>>>>>>>>>>>here m_mallAnalysisPart.ifBrdQuery = " << pa->m_mallAnalysisPart.ifBrdQuery << endl;
	//cout << "me.vTerms.size()=" << me.vTerms.size() << endl;
	if (true == pa->m_mallAnalysisPart.ifBrdQuery) {
		//cout << "enter <ght< endl;
		BrandQRank(pa, me, rt);
	} else {
		//fprintf(stderr,"size=%d\n",me.vTerms.size());
		1 == me.vTerms.size()?
			SingleRank(pa, me, rt):
			MultiRank(pa, me, rt);
	}
}

void CSearchKeyRanking::ComputeCommerceWeight(const int docID,
		int& commerceScr, int& saleDetail) const { //商业评估函数
	int sale = m_funcFrstInt(m_saleWeekAmtProfile, docID);
	int date = m_funcFrstInt(m_modifyTime, docID); //@todo:这里date表示上架时间距离1970年1月1日的秒数
	if (date > 0){                                       //而实际是要计算出到现在的天数
		int pid = m_funcFrstInt64(m_isPidProfile, docID);
	}
	//printf("sale=%d\n",sale);
	if(sale)
	{
		if(sale <100)
		{
			commerceScr = 1;
			saleDetail = sale/30;
		}
		else if(sale < 500)
		{
			commerceScr = 2;
			saleDetail = 3+sale / 160;
		}
		else if(sale <1000)
		{
			commerceScr = 3;
			saleDetail = 6 + sale/300;
		}
		else if(sale <2000)
		{
			commerceScr = 4;
			saleDetail = 9 + sale/600;
		}
		else
		{
			commerceScr = 4 + sale/2000 > 11 ? 11:(4+sale/2000) ;
			saleDetail = 12 + sale/2000 > 30 ? 30:(12+sale/2000) ;
		}
	}

	int cur_secs = time(NULL); //表示当前时间距离1970年1月1日的秒数
	int input_days = (cur_secs - date)/(3600*24); //该商品已经上架的天数
	//if (date <= 7) {
	if (input_days <= 7) { //如果是最近上架的，进行加权
		++commerceScr;
	}
}

void CSearchKeyRanking::BrandQRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{	//品牌搜索相关度计算
	//基本信息
	int docID = rt.nDocId;
	//字段信息
	int fieldScr = 0;		//字段得分[0-4]
	bool ifInTitle = false;						//是否出在标题
	bool ifInSyn = false;							//是否出在同义扩展
	bool ifInPartBrand = false;						//是否品牌部分匹配
	bool ifInFullBrand = false;                     //是否品牌完全匹配
	int fbBrandCateWeight = 0;                  //品牌类别反馈权重
	{
		int ivtCnt, cnt;
		int fid, fieldNum;
		int queryLen = 0;
		int filedLen = 0;
		float brandWeight = 0.0;
		int querySize = me.vTerms.size();
		for (int k = 0; k < querySize; ++k)
		{	//for one term
			filedLen = me.vFieldLen[k];
			queryLen = me.vTerms[k].len;
			//if (2 == pa->m_mallAnalysisPart.type[k])
			{	//品牌词
				//fid = ivtAddr[cnt].fieldID;
				fid = me.vFieldsOff[k].field;
				if (fid == BRAND_FID)
				{	//品牌
					brandWeight += (float)queryLen/filedLen;
				}
				else if (fid == TITLE_FID)
				{	//标题
					ifInTitle = true;
				}
				else if (fid == TITLESYN_FID)
				{	//同义扩展
					ifInSyn = true;
				}
				//++cnt;
			}
		}
		//判断品牌匹配程度(减少分词带来的影响)
		if(brandWeight>0.8)
			ifInFullBrand = true;
		else if(brandWeight>0.3)
			ifInPartBrand = true;
		
		fieldScr = ifInFullBrand ? 4 : (ifInPartBrand ? 3 : (ifInTitle ? 2 : (ifInSyn ? 1 : 0)));
	}
	{//品牌反馈
		GetFeedBackBrandCateWeight(docID,pa,fbBrandCateWeight);
	}
	//商业权重:
	int commerceScr = 0; //[0-6]
	int saleDetail = 0;
	{
		ComputeCommerceWeight(docID, commerceScr, saleDetail);
	}
	int sale = m_funcFrstInt(m_saleWeekProfile, docID);

	//相关类别记录
	size_t nCnt = 0;
	u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针
	int cateNum = 0;		//记录底级类编号（cateStat的下标+1）
	int relScr = 0;			//精简相关标识[0-2]
	if (ifInFullBrand) //对品牌字段中的所有商品处理
	{
		relScr = 2;
		++pa->m_mallAnalysisPart.relCnt_brd; //在品牌字段中的商品数
		for (int k=0; k<nCnt; k++) { //遍历该单品所有类别
			//注意这里的id与数据库中的不同，所以反馈数据的id要转化！！！
			u64 bottomCate = ptr[k];	//底级类
			int idx = -1;																	//cateStat下标
			map<u64, int>::const_iterator cit = pa->m_mallAnalysisPart.cid2cnum.find(bottomCate);
			if (cit != pa->m_mallAnalysisPart.cid2cnum.end())
			{	//已有此类别
				idx = pa->m_mallAnalysisPart.cid2cnum[bottomCate];
			}
			else
			{	//尚无此类别
				idx = (int)pa->m_mallAnalysisPart.cateStat.size();
				pa->m_mallAnalysisPart.cid2cnum[bottomCate] = idx;
				pa->m_mallAnalysisPart.cateStat.push_back(CateStat());
			}

			if (idx >= 0 && idx < (int)pa->m_mallAnalysisPart.cateStat.size())
			{
				cateNum = idx + 1;
				++pa->m_mallAnalysisPart.cateStat[idx].pidCnt; //类别中的商品数
				pa->m_mallAnalysisPart.cateStat[idx].saleSum += sale;
			}
			u64 top_cate = GetClassByLevel(1, ptr[k]);
			pa->m_mallAnalysisPart.topCate_brd.insert(top_cate); //相关顶级类集合
		}
	}
	else if (ifInTitle)
	{
		relScr = 1;
		++pa->m_mallAnalysisPart.relCnt_ti;
		for (int k=0; k<nCnt; k++) { //遍历该单品所有类别
			u64 top_cate = GetClassByLevel(1, ptr[k]);
			pa->m_mallAnalysisPart.topCate_ti.insert(top_cate);//相关顶级类
		}
	}

	//其他信息(公用品)
	bool ifPublic = false;
	{
		if (1 == m_funcFrstInt(m_isShareProductProfile, docID))
		{
			ifPublic = true;
		}
	}
	rt.nScore += FakeScore  //1101000000
		+ 1 * BaseRelFactor //10000000
		+ fieldScr * (Weight_Field+Weight_Brand+Weight_Dis+ Weight_BrandAppendScore) //60+30+30+150
		+ fbBrandCateWeight * Weight_FBCate     //品牌反馈权重[0,1] 100
		+ commerceScr * Weight_Commerce //0
		+ ifPublic * Weight_Public //10
		+ saleDetail * Weight_Sale; //0

	rt.nWeight += commerceScr * CommerceFactor  //0
		+ relScr * RelIndicator //10000
		+ fieldScr * FieldFactor //1000
		+ cateNum; //品牌字段包含query的商品类别数
}
//得到品牌的类别反馈权重
void CSearchKeyRanking::GetFeedBackBrandCateWeight(int docID,CDDAnalysisData* pa, int& fbBrandCateWeight)
{
	bool match = false;
	u64 cate_id = 0;
	u64 bottomCate = 0;
	int weight = 0;
	size_t nCnt = 0;
	fbBrandCateWeight = 0;
	u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针
	int cate_count = m_key2CateBrd[pa->m_mallAnalysisPart.querystr].count;
	if (cate_count != 0)
	{
		//类别反馈
		HASHVECTOR cate_vec = m_key2CateBrd[pa->m_mallAnalysisPart.querystr];
		for(int k=0; k<nCnt && !match; k++)
		{
			bottomCate = ptr[k];
			for (int k=0; k<cate_count; k++) 
			{
				memcpy(&cate_id, cate_vec.data+k*cate_vec.size, sizeof(u64));
				memcpy(&weight, cate_vec.data+k*cate_vec.size+sizeof(u64), sizeof(int));
				if(bottomCate == cate_id)
				{
					match = true;
					if(weight > 100)
					{
						fbBrandCateWeight = 1;
						break;
					}
				}
			}
		}
	}
}

//@todo: catePath中的类别id是计算出来的，与数据库中的不同，所以反馈数据中
//       的类别id需要转化成与catepath中的一致
void CSearchKeyRanking::ComputeSpecialWeight(
	const int docID,
	CDDAnalysisData* pa,
	int& fbCateWeight,
	int& fbPidWeight,
	int& pdtCoreWeight,
	int& relScr)
{
	fbCateWeight = fbPidWeight = pdtCoreWeight = 0;
	int pid = m_funcFrstInt64(m_isPidProfile, docID);
	size_t nCnt = 0;
	u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针

	if (false == pa->m_mallAnalysisPart.vCate.empty()) //反馈类别集合不为空
	{ //计算类别反馈得分
		const vector<pair<u64, int> >& fbCate = pa->m_mallAnalysisPart.vCate;
		//multimap<int, vector<int> >::const_iterator lower = pid2cate.find(pid);
		//if (lower != pid2cate.end())
		//{	//有类别信息
		//	multimap<int, vector<int> >::const_iterator upper =
		//		pid2cate.upper_bound(pid);
			bool match = false;

		//	while (lower != upper && false == match)
		//	{	//for every catePath of pid
		//		const vector<int> &catePath = lower->second;
			for (int k=0; k<nCnt && false == match; k++) { //遍历该单品所有类别
                //类别id->类别路径的各级类别id
                //注意这里的id与数据库中的不同，所以反馈数据的id要转化！！！
				u64 bottomCate = ptr[k];	//@fixed bug: not nCnt, but k 底级类
				/*{
					cout << "bottomCate = " << bottomCate << endl;
					char cate_path[40];
					TranseID2ClsPath(cate_path, bottomCate, 6);
					cout << "cate_path = " << cate_path << endl;
				}*/
				//if (!catePath.empty())
				{
					for (size_t i = 0; i < fbCate.size(); ++i)
					{	//反馈底级类
						if (bottomCate == fbCate[i].first) //当前商品类别与反馈类别相匹配
						{
							fbCateWeight = fbCate[i].second;	//weight
							match = true;
							u64 top_cate = GetClassByLevel(1, bottomCate);
							pa->m_mallAnalysisPart.topCate_fb.insert(top_cate);//相关顶级类
							++pa->m_mallAnalysisPart.relCnt_fb; //处于反馈类别中的商品数目
							relScr = 2;
							break;
						}
					}
				}
		//		++lower;
			}
		//}
	}
	if (false == pa->m_mallAnalysisPart.vPid.empty())
	{	//计算单品反馈得分
		const vector<int>& fbPids = pa->m_mallAnalysisPart.vPid;
		for (size_t i = 0; i < fbPids.size(); ++i)
		{
			if (pid == fbPids[i])
			{
				fbPidWeight = 1;
				if (0 == fbCateWeight)
				{
					fbCateWeight = 1;
				}
				break;
			}
		}
	}
	//if (true == pa->m_mallAnalysisPart.ifPdtQuery && m_pid2Core.find(pid) != m_pid2Core.end())

	int core_word_count = m_pid2Core[pid].count;
	if (true == pa->m_mallAnalysisPart.ifPdtQuery && (core_word_count != 0))
	{	//计算产品中心权重
		//const vector<string>& keys = m_pid2Core.find(pid)->second;
		HASHVECTOR keys = m_pid2Core[pid];
		/*if (find(keys.begin(), keys.end(), pa->m_mallAnalysisPart.pdtWord) != keys.end()) {
			cout << "docID=" << docID << " pid=" << pid;
			cout << " core_word=";
			for (size_t i = 0; i < keys.size(); ++i) {
				cout << keys[i] << " ";
			}
			cout << endl;
		}*/
		for (int i = 0; i < core_word_count; ++i)
		{
			size_t query_size =  pa->m_mallAnalysisPart.pdtWord.size();
            size_t key_size = strlen(keys.data+i*keys.size);
			if (0 == pa->m_mallAnalysisPart.pdtWord.compare(0, query_size, keys.data+i*keys.size, key_size))
			{	//产品匹配
				pdtCoreWeight = 2;

				//multimap<int, vector<int> >::const_iterator lower = pid2cate.find(pid);
				//if (lower != pid2cate.end())
				//{	//有类别信息
				//	multimap<int, vector<int> >::const_iterator upper = pid2cate.upper_bound(pid);

				//	while (lower != upper)
					for (int k=0; k<nCnt; k++) { //遍历该单品所有类别
						                         //for every catePath of pid
				//		pa->m_mallAnalysisPart.topCate_prd.insert(lower->second[0]);//相关顶级类
						u64 top_cate = GetClassByLevel(1, ptr[k]);
					    pa->m_mallAnalysisPart.topCate_prd.insert(top_cate);
				//		++lower;
					}
					++pa->m_mallAnalysisPart.relCnt_prd; //中心词匹配的商品数目
					if (0 == relScr)
					{
						relScr = 1;
					}
				//}
				break;
			}
			else if (!pa->m_mallAnalysisPart.pdtNg.empty() && key_size >= 2
				&& (0 == pa->m_mallAnalysisPart.pdtNg.compare(0, 2, keys.data+i*keys.size, key_size-2, 2)))
			{	//语素匹配
				pdtCoreWeight = 1;
			}
		}
	}
}

void CSearchKeyRanking::ReRankingMall(vector<SResult>& vRes, CDDAnalysisData* pa)
{
#ifdef DEBUG
	cout << ">>>>>>>>>>>>>>>ReRankingMall" << endl;
#endif
	if (true == pa->m_mallAnalysisPart.ifBrdQuery)
	{
		CombineComputeWeight_BrdQ(pa, vRes);
	}
	else if (true == pa->m_mallAnalysisPart.ifPdtQuery)
	{
		CombineComputeWeight_PrdQ(pa, vRes);
	}
	else
	{
		CombineComputeWeight_ComQ(pa, vRes);
	}
	FilterMallResults(vRes); //精简结果
#ifdef DEBUG
	cout << "FilterMallResults: vRes.size() = " << vRes.size() << endl;
#endif
}



void CSearchKeyRanking::SortMall(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa)
{
	// do somethine more 
	SortRange(vRes ,from, to);
#ifdef DEBUG
	int out_size = min((int)vRes.size(), 50);
	for (int i=0; i<out_size; i++) {
		int docID = vRes[i].nDocId;
		int date = m_funcFrstInt(m_modifyTime, docID);
		int pid = m_funcFrstInt64(m_isPidProfile, docID);
		vector<char> vBuf;
		vector<char*> vFieldPtr;
		vector<int> vShowFields;
		vShowFields.push_back(TITLE_FID);
		m_funcDocInfoPtr(vRes[i].nDocId, vShowFields, vFieldPtr, vBuf, m_pSearcher);
		cerr << "debug_weight=" << vRes[i].nWeight << "  rank_score=" << vRes[i].nScore << " title=" << vFieldPtr[0] << " pid=" << pid << endl;
	}
#endif
}
