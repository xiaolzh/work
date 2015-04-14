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

	//���شʵ�
	if (false == loadDict(m_strModulePath + "product_word.txt", m_pdtWords)) {
		return false;
	}
	if (false == loadDict(m_strModulePath + "brand_word.txt", m_brdWords)) {
		return false;
	}

	//���ذٻ���������
	m_key2Cate.read(m_strModulePath + "query_cate_direction");
	m_key2CateBrd.read(m_strModulePath + "query_cate_brand");
	m_key2Pid.read(m_strModulePath + "query_pid_direction");
	m_pid2Core.read(m_strModulePath + "pid_core_word");
	printf("init mall: load b2c feedback data\n");
	//���ػ�������
	string b2c_pub_rate_path(m_strModulePath + "b2c_pub_key_rate");
	if (false == m_percentPub2Mall.load_serialized_hash_file(b2c_pub_rate_path.c_str(), pair<int, int>(-1, -1))) {
		printf("failed to load b2c_pub_keyrate\n");
		return false;
	}
	printf("init merge data: load b2c_pub_keyrate\n");

	//�ֶα��
	TITLENAME = 9;
	TITLESYN = 8;
	BOTTOMCATE = 7;
	OTHERCATE = 6;
	BRAND = 5;
	SUBTITLE = 4;
	TMinNum = 4;
	m_fid2fi.resize(m_vFieldInfo.size());
	//���������ļ�search.conf����ֶ���
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
	//��ʼ��pa
	vector<QueryItem> queryItems;
	QueryParse(qc.key, qc.vTerms, queryItems);
	size_t queryKeyCnt = queryItems.size();		//query����
	pa->m_mallAnalysisPart.type.resize(queryKeyCnt, 0);
	pa->m_mallAnalysisPart.keysRepeat.resize(queryKeyCnt, 0);
	pa->m_mallAnalysisPart.ifBrdQuery = false;
	pa->m_mallAnalysisPart.ifPdtQuery = false;
	pa->m_mallAnalysisPart.needJudgeDis = false;
	pa->m_mallAnalysisPart.relCnt_fb = 0;
	pa->m_mallAnalysisPart.relCnt_prd = 0;
	pa->m_mallAnalysisPart.relCnt_brd = 0;
	pa->m_mallAnalysisPart.relCnt_ti = 0;
	//@todo: pa�л�������������Ҫ��ʼ��

	int pdtCnt = 0;				//��Ʒ����
	bool hasBrd = false;	//��Ʒ�ƴ�
	bool hasCom = false;	//��һ���
	string pdtWord = "";	//��Ʒ�ʣ�Ψһ��
	bool appearDisFeature = false;

	for (size_t i = 0; i < queryKeyCnt; ++i)
	{
		string key = queryItems[i].word;
		pa->m_mallAnalysisPart.querystr += key;

		if (m_pdtWords.find(key) != m_pdtWords.end())
		{	//��Ʒ��
			pa->m_mallAnalysisPart.type[i] = 1;
			++pdtCnt;
			pdtWord = key;
		}
		else if (m_brdWords.find(key) != m_brdWords.end())
		{	//Ʒ�ƴ�
			pa->m_mallAnalysisPart.type[i] = 2;
			hasBrd = true;
		}
		else
		{	//һ���
			pa->m_mallAnalysisPart.type[i] = 0;
			hasCom = true;
		}

		if (0 == pa->m_mallAnalysisPart.keysRepeat[i])
		{	//�ظ��ʼ��鼰��¼
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
		{	//��������һ��������ʾ�
			appearDisFeature = true;
		}
	}
	if (true == pa->m_mallAnalysisPart.needJudgeDis &&
		(pa->m_mallAnalysisPart.querystr.size() > 12 || false == appearDisFeature) )
	{	//�Ƿ������ʾ�(��query��δ������Ҫ����ʾ������򲻼���ʾ�)
		pa->m_mallAnalysisPart.needJudgeDis = false;
	}

	int cate_count = m_key2Cate[pa->m_mallAnalysisPart.querystr].count;
	if (cate_count != 0)
	{	//�����
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
	{	//��Ʒ����
		HASHVECTOR pid_vec = m_key2Pid[pa->m_mallAnalysisPart.querystr];
		for (int k=0; k<pid_count; k++) { //��ȡquery�ķ���pid
			int pid = 0;
			memcpy(&pid, pid_vec.data+k*pid_vec.size, 4);
			pa->m_mallAnalysisPart.vPid.push_back(pid);
		}
	}

	if ( m_pdtWords.find(pa->m_mallAnalysisPart.querystr) != m_pdtWords.end()
		|| (1 == pdtCnt && hasBrd && !hasCom) )
	{	//��Ʒ����/Ʒ��+��Ʒ����
		pa->m_mallAnalysisPart.ifPdtQuery = true;
		pa->m_mallAnalysisPart.needJudgeDis = false;

		//��ʾ��ǰqueryֻ�����������ı���, ����Ʒ���ֶ�������
		pa->m_queryFieldIds.push_back(TITLE_FID);
		pa->m_queryFieldIds.push_back(SUBNAME_FID);
		pa->m_queryFieldIds.push_back(TITLESYN_FID);
		pa->m_queryFieldIds.push_back(BRAND_FID);
		pa->m_queryFieldIds.push_back(BOTTOMCATE_FID);

		pa->m_mallAnalysisPart.pdtWord = m_pdtWords.find(pa->m_mallAnalysisPart.querystr) != m_pdtWords.end() ?
			pa->m_mallAnalysisPart.querystr : pdtWord;

		if ( pa->m_mallAnalysisPart.pdtWord.size() > 2
			&& pa->m_mallAnalysisPart.pdtWord[pa->m_mallAnalysisPart.pdtWord.size() - 2] < 0)
		{	//�������أ������һ����
			string tmpNg = pa->m_mallAnalysisPart.querystr.substr(
				pa->m_mallAnalysisPart.querystr.size() - 2, 2);
			if (m_pdtWords.find(tmpNg) != m_pdtWords.end())
			{	//��Ʒ����, Ҳ���ǲ�Ʒ�ʵ����һ����
				pa->m_mallAnalysisPart.pdtNg = tmpNg;
			}
		}
	}
	else if (m_brdWords.find(pa->m_mallAnalysisPart.querystr) != m_brdWords.end()
		|| (hasBrd && !hasCom && 0 == pdtCnt) )
	{	//Ʒ������(����query��Ʒ�ƻ���query�����дʶ���Ʒ�ƴ�)
		pa->m_mallAnalysisPart.ifBrdQuery = true;
		pa->m_mallAnalysisPart.needJudgeDis = false;

		//��ʾ��ǰqueryֻ�����������ı���, ����Ʒ���ֶ�������
		pa->m_queryFieldIds.push_back(TITLE_FID);
		pa->m_queryFieldIds.push_back(SUBNAME_FID);
		pa->m_queryFieldIds.push_back(TITLESYN_FID);
		pa->m_queryFieldIds.push_back(BRAND_FID);

		//map<string, vector<pair<int,int> > >::const_iterator iter =
		//	m_key2CateBrd.find(pa->m_mallAnalysisPart.querystr);
		HASHVECTOR vCat = m_key2CateBrd[pa->m_mallAnalysisPart.querystr];
		//if (iter != m_key2CateBrd.end())
		int cate_count = vCat.count;
		if (cate_count != 0) //��Ʒ��query�ķ��������
		{	//��Ʒ�Ʒ���
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
						pa->m_mallAnalysisPart.cateStat[num].cateScore = weight; //���ķ���Ȩ��
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
		//���������������ǰquery�������ֶ�������
		pa->m_queryFieldIds.push_back(TITLE_FID);
		pa->m_queryFieldIds.push_back(SUBNAME_FID);
		pa->m_queryFieldIds.push_back(TITLESYN_FID);
		pa->m_queryFieldIds.push_back(BRAND_FID);
		pa->m_queryFieldIds.push_back(BOTTOMCATE_FID);
	}
}

//����0ֵ���
static inline bool filter_zero_weight(const SResult& di)
{
	return di.nScore <= 0;
}

void CSearchKeyRanking::FilterMallResults(vector<SResult>& vRt) {
	size_t count = vRt.size();
	for (size_t i = 0; i < count; ++i) {
		if (vRt[i].nWeight/TypeIndicator == 0) {//����ǳ�������������ڻ��ž���
			continue;
		}
		if (0 == (vRt[i].nScore / BaseRelFactor & 1)) {
			vRt[i].nScore = 0;//�����
		}
	}
	vRt.erase(std::remove_if(vRt.begin(), vRt.end(), filter_zero_weight), vRt.end());
}

void CSearchKeyRanking::CombineComputeWeight_ComQ(
	CDDAnalysisData* pa,
	vector<SResult>& vRt)
{	//һ������

	//�޸�BaseRel, Ϊ����׼��
	//���������������Ʒ��>=20 && �����Ʒ�Ķ�����𼯺ϲ�Ϊ��
	if (pa->m_mallAnalysisPart.relCnt_fb >= 20 && false == pa->m_mallAnalysisPart.topCate_fb.empty())
	{	//��������
		set<u64> &RelCate = pa->m_mallAnalysisPart.topCate_fb; //ֻ���η���
		for (size_t i = 0; i < vRt.size(); ++i)
		{
			if (vRt[i].nWeight/TypeIndicator == 0) {//����ǳ�������������ڻ��ž���
				continue;
			}
			if (vRt[i].nWeight / RelIndicator % 10 != 2)		//��ƥ�䷴��
			{	//���������
				bool ifRel = false;
				int docID = vRt[i].nDocId;
				size_t nCnt = 0;
				u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //�������ָ��

				for (int k=0; k<nCnt; k++) {
					u64 top_cate = GetClassByLevel(1, ptr[k]); //��ȡ��ǰ���Ķ������
					if (RelCate.find(top_cate)!=RelCate.end()) //@fixed bug: if catePath.size()=0
					{
						ifRel = true;
						break;
					}
				}

				if (false == ifRel && vRt[i].nScore > BaseRelFactor) //����Ȩ��
				{	//��ǲ����
					vRt[i].nScore -= BaseRelFactor;
				}
			}
		}
	}
}

void CSearchKeyRanking::CombineComputeWeight_PrdQ(
	CDDAnalysisData* pa,
	vector<SResult>& vRt)
{	//��Ʒ������
	//�޸�BaseRel, Ϊ����׼��
	if (pa->m_mallAnalysisPart.relCnt_fb + pa->m_mallAnalysisPart.relCnt_prd >= 20)
	{	//��������
		bool RelType = true;	//ֻƥ�䷴������
		set<u64> &RelCate = pa->m_mallAnalysisPart.topCate_fb; //ֻ���η���
		if (pa->m_mallAnalysisPart.relCnt_fb < 20)
		{	//���η����Ͳ�Ʒ����
			RelType = false;		//ֻƥ�䷴���Ͳ�Ʒ��������
			//cout << "insert" << endl;
			RelCate.insert(pa->m_mallAnalysisPart.topCate_prd.begin(), pa->m_mallAnalysisPart.topCate_prd.end());
		}
		if (false == RelCate.empty())
		{
			for (size_t i = 0; i < vRt.size(); ++i)
			{
				if (vRt[i].nWeight/TypeIndicator == 0) {//����ǳ�������������ڻ��ž���
					continue;
				}
				int relScr = vRt[i].nWeight / RelIndicator % 10;	//������ر�ʶ
				if ( (true == RelType && relScr != 2)		//��ƥ�䷴��
					|| (false == RelType && relScr < 1) )	//��ƥ�䷴�����Ʒ����
				{	//���������
					bool ifRel = false;
					int docID = vRt[i].nDocId;
					size_t nCnt = 0;
					u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //�������ָ��

					for (int k=0; k<nCnt; k++) {
						u64 top_cate = GetClassByLevel(1, ptr[k]); //��ȡ��ǰ���Ķ������
						if (RelCate.find(top_cate)!=RelCate.end()) //@fixed bug: if catePath.size()=0
						{
							//cout << "find the top_cate=" << top_cate << endl;
							ifRel = true;
							break;
						}
					}
					if (false == ifRel && vRt[i].nScore > BaseRelFactor)
					{	//��ǲ����
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
{	//Ʒ������

	//�޸�BaseRel, Ϊ����׼��
	if (pa->m_mallAnalysisPart.relCnt_brd + pa->m_mallAnalysisPart.relCnt_ti >= 20)
	{	//��������
		bool RelType = true;					 //ֻƥ��Ʒ���ֶ�����
		set<u64> &RelCate = pa->m_mallAnalysisPart.topCate_brd; //ֻ����Ʒ���ֶ�ƥ��
		if (pa->m_mallAnalysisPart.relCnt_brd < 20)
		{	//����Ʒ�ƻ�����ֶ�ƥ��
			RelType = false;		//ֻƥ��Ʒ��������ֶ�����
			RelCate.insert(pa->m_mallAnalysisPart.topCate_ti.begin(), pa->m_mallAnalysisPart.topCate_ti.end());
		}
		if (false == RelCate.empty())
		{
			for (size_t i = 0; i < vRt.size(); ++i)
			{
				if (vRt[i].nWeight/TypeIndicator == 0) {//����ǳ�������������ڻ��ž���
					continue;
				}
				//int relScr = out[i].textWeight / RelIndicator % 10;	//������ر�ʶ
				int relScr = vRt[i].nWeight / RelIndicator % 10; //����Ȩ��
				if ( (true == RelType && relScr != 2)		//��ƥ��Ʒ��
					|| (false == RelType && relScr < 1) )	//��ƥ��Ʒ�ƻ����
				{	//���������
					bool ifRel = false;
					int docID = vRt[i].nDocId;
					size_t nCnt = 0;
					u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //�������ָ��

					for (int k=0; k<nCnt; k++) {
						u64 top_cate = GetClassByLevel(1, ptr[k]); //��ȡ��ǰ���Ķ������
						if (RelCate.find(top_cate)!=RelCate.end()) //@fixed bug: if catePath.size()=0
						{
							ifRel = true;
							break;
						}
					}

					if (false == ifRel && vRt[i].nScore > BaseRelFactor)
					{	//��ǲ����
						//out[i].weight -= BaseRelFactor;
						vRt[i].nScore -= BaseRelFactor;
					}
				}
			}
		}
	}

	//����(top30)
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
						pidMaxCnt = pa->m_mallAnalysisPart.cateStat[c].pidCnt; //������Ʒ��
					}
					if (pa->m_mallAnalysisPart.cateStat[c].cateScore > cateMaxWeight)
					{
						cateMaxWeight = pa->m_mallAnalysisPart.cateStat[c].cateScore; //���ķ���Ȩ��
					}
					if (pa->m_mallAnalysisPart.cateStat[c].saleSum > saleMax)
					{
						saleMax = pa->m_mallAnalysisPart.cateStat[c].saleSum; //����е���Ʒ����
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
			//ǰ10������бس��������࣬ǰ30������бس���3����
			if (TEST_BRAND_QUERY)
			{
				cout << "RATE:";
			}
			if (2 == scr2idx.size())
			{
				multimap<int, size_t, greater<int> >::iterator mit = scr2idx.begin();
				int ff = mit->first; //���score
				size_t fs = mit->second; //�����
				++mit;
				int sf = mit->first; //��һ�����score
				size_t ss = mit->second; //��һ�������

				//��ǰ����������·���score
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
			{	//����
				multimap<int, size_t, greater<int> >::iterator fst,snd,thd;
				fst = snd = scr2idx.begin();
				thd = ++snd;
				++thd;
				int ff = fst->first;
				int sf = snd->first;
				int tf = thd->first;
				//��ǰ����������·���score
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
							val = 1 == loopNum ? ff : (2 == loopNum ? sf : tf); //ǰ3�������ĳ����Ȩ��
						}

						if (TEST_BRAND_QUERY)
						{
							cout << " " << val;
						}

						if (val * 10 / sum > 0)
						{	//ǰ10��Ӫ
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(val * 10 / sum);
							pa->m_mallAnalysisPart.cateStat[sit->second].camp.push_back(val * 10 / sum);
							campCnt += val * 20 / sum;
						}
						else if (val * 30 / sum > 0)
						{	//ǰ30��Ӫ
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
						boost::bind(&SResult::nScore, _1), //����Ȩ��
						boost::bind(&SResult::nScore, _2)));

			int cateNum;
			for (size_t i = 0; i < TopN; ++i)
			{
				//cateNum = out[i].textWeight % 100;
				cateNum = vRt[i].nWeight % 100; //����Ȩ�أ������Ȩ��
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
							vRt[i].nScore += (2-c) * 100 +             //����Ȩ��
								pa->m_mallAnalysisPart.cateStat[cateNum-1].cateScore * 10;
							//out[i].textWeight += (c+1) * 100;	//��Ӫ��
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

	//������Ϣ
	int docID = rt.nDocId;
	//cout << "docID = " <<docID << endl;
	//fprintf(stderr,"Single docID=%d\n",docID);

	//�ֶ���Ϣ
	int fieldScr = 0;									//�ֶε÷�[0-4]
	bool ifContainAll_T = false;			//�Ƿ�����Ҫ�ֶγ���
	bool ifContainAll_ti = false;			//�����Ƿ����
	bool ifContainAll_syn = false;		//��ͬ��������(��ͬ���ֶ�)
	bool ifContainAll_bottom = false;	//�ڵ׼�����г���
	bool ifContainAll_upper = false;	//�ڸ߼�����г���
	bool ifInBrand = false;						//Ʒ���ֶ�֧��
	bool ifDisMatch = true;					  //�ʾ��Ƿ�ƥ��
	{
		vector<SFieldOff> fieldPtr = me.vFieldsOff; //��������Щ�ֶ�
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
				{	//����
					ifContainAll_T = true;

					if (fieldNum == TITLENAME)
					{ //����
						ifContainAll_ti = true;
					}
					else if (fieldNum == TITLESYN)
					{ //ͬ����չ
						if (false == ifContainAll_ti)
						{
							ifContainAll_syn = true;
						}
				  }
					else if (fieldNum == BOTTOMCATE)
					{ //�׼���
						ifContainAll_bottom = true;
					}
					else if (fieldNum == OTHERCATE)
					{ //��������
						ifContainAll_upper = true;
					}
					else if (fieldNum == BRAND &&
						(true == pa->m_mallAnalysisPart.ifBrdQuery || 2 == pa->m_mallAnalysisPart.type[0]) )
					{	//Ʒ���ֶ�֧��
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
	//������Ϣ(�����/��Ʒ����/��Ʒ����)
	int relScr = 0;						//������ر�ʶ[0-2]
	int fbCateWeight = 0;
	int fbPidWeight = 0;
	int pdtCoreWeight = 0;
	{
		ComputeSpecialWeight(
			docID, pa, fbCateWeight, fbPidWeight, pdtCoreWeight, relScr);
	}

	//��ҵ��Ϣ(��ҵȨ��/����ϸ��)
	int commerceScr = 0;
	int saleDetail = 0;
	{
		ComputeCommerceWeight(docID, commerceScr, saleDetail);
	}

	//������Ϣ(����Ʒ)
	bool ifPublic = false;
	{
		if (1 == m_funcFrstInt(m_isShareProductProfile, docID)) {
			ifPublic = true;
		}
	}
	//���������Ȩ��
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
	//�û����Ե�Ȩ��
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

	//������Ϣ
	int docID = rt.nDocId;

	//�ֶ���Ϣ
	int fieldScr = 0;									//�ֶε÷�[0-4]
	bool ifContainAll_T = false;			//��Ҫ�ֶ��Ƿ��ȫ
	bool ifContainAll_ti = false;			//�����Ƿ��ȫ(������)
	bool ifContainAll_syn = false;		//�����Ƿ��ȫ(����+ͬ����չ)
	bool ifContainAll_bottom = false;	//�׼�������Ƿ��ȫ
	bool ifContainAll_upper = false;	//�߼�������Ƿ��ȫ
	bool ifInBrand = false;						//Ʒ���ֶ�֧��
	bool ifDisMatch = true;						//�ʾ��Ƿ�ƥ�䣨���⣩
	{
		int sz_T = 0, sz_ti = 0, sz_sys = 0;	//����/����/��ͬ�����
		int sz_b = 0, sz_ob = 0, sz_u = 0; 	  //�׼���/���׼���/�߼��������
		vector<vector<int> > vPosTi(querySz);	//�������λ�ü�¼
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
						{	//����
							if (false == added_T)
							{
								++sz_T;
								added_T = true;
							}
							if (fieldNum == TITLENAME)
							{ //����
								if (false == added_ti)
								{
									++sz_ti;
									added_ti = true;
								}
								//vPosTi[k].push_back(ivtAddr[cnt].offset);
								vPosTi[k].push_back(me.vFieldsOff[k].off);
							}
							else if (fieldNum == TITLESYN)
							{ //ͬ����չ
								if (false == added_ti && false == added_syn)
								{
									++sz_sys;
									added_syn = true;
								}
						    }
							else if (fieldNum == BOTTOMCATE)
							{ //�׼���
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
							{ //��������
								if (false == added_u)
								{
									++sz_u;
									added_u = true;
								}
							}
							else if (fieldNum == BRAND &&
								(pa->m_mallAnalysisPart.ifBrdQuery || 2 == pa->m_mallAnalysisPart.type[k]))
							{	//Ʒ��֧��
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
		{	//�жϴʾ�ƥ��
			if (false == ifContainAll_ti)
			{
				ifDisMatch = false;
			}
			else
			{
				for (int k = 1; k < querySz; ++k)
				{
					//if (0 == vKey[k].dis &&
					//if (0 == pa->m_mallAnalysisPart.vDis[k] &&     //vDis��ʾ������֮��ı����ų��ȣ���query_parser_segment.h
					                                                 //�е�fwdDis, ������ʱ�ȼٶ�Ϊ0
					if(0 == pa->m_mallAnalysisPart.type[k-1] && 0 == pa->m_mallAnalysisPart.type[k]) //��Ϊһ���
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

	//��ҵ��Ϣ(��ҵȨ��/����ϸ��)
	int commerceScr = 0; //[0-6]
	int saleDetail = 0;	 //[0-30]
	{
		ComputeCommerceWeight(docID, commerceScr, saleDetail);
	}

	//������Ϣ(����Ʒ)
	bool ifPublic = false;
	{
		if (1 == m_funcFrstInt(m_isShareProductProfile, docID))
		{
			ifPublic = true;
		}
	}

	//������Ϣ(�����/��Ʒ����/��Ʒ����)
	int relScr = 0;						//������ر�ʶ[0-2]
	int fbCateWeight = 0;
	int fbPidWeight = 0;
	int pdtCoreWeight = 0;
	{

		ComputeSpecialWeight(
			docID, pa, fbCateWeight, fbPidWeight, pdtCoreWeight, relScr);
	}
	//����ʵ��Ȩ��
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
	//����
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
		int& commerceScr, int& saleDetail) const { //��ҵ��������
	int sale = m_funcFrstInt(m_saleWeekAmtProfile, docID);
	int date = m_funcFrstInt(m_modifyTime, docID); //@todo:����date��ʾ�ϼ�ʱ�����1970��1��1�յ�����
	if (date > 0){                                       //��ʵ����Ҫ����������ڵ�����
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

	int cur_secs = time(NULL); //��ʾ��ǰʱ�����1970��1��1�յ�����
	int input_days = (cur_secs - date)/(3600*24); //����Ʒ�Ѿ��ϼܵ�����
	//if (date <= 7) {
	if (input_days <= 7) { //���������ϼܵģ����м�Ȩ
		++commerceScr;
	}
}

void CSearchKeyRanking::BrandQRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{	//Ʒ��������ضȼ���
	//������Ϣ
	int docID = rt.nDocId;
	//�ֶ���Ϣ
	int fieldScr = 0;		//�ֶε÷�[0-4]
	bool ifInTitle = false;						//�Ƿ���ڱ���
	bool ifInSyn = false;							//�Ƿ����ͬ����չ
	bool ifInPartBrand = false;						//�Ƿ�Ʒ�Ʋ���ƥ��
	bool ifInFullBrand = false;                     //�Ƿ�Ʒ����ȫƥ��
	int fbBrandCateWeight = 0;                  //Ʒ�������Ȩ��
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
			{	//Ʒ�ƴ�
				//fid = ivtAddr[cnt].fieldID;
				fid = me.vFieldsOff[k].field;
				if (fid == BRAND_FID)
				{	//Ʒ��
					brandWeight += (float)queryLen/filedLen;
				}
				else if (fid == TITLE_FID)
				{	//����
					ifInTitle = true;
				}
				else if (fid == TITLESYN_FID)
				{	//ͬ����չ
					ifInSyn = true;
				}
				//++cnt;
			}
		}
		//�ж�Ʒ��ƥ��̶�(���ٷִʴ�����Ӱ��)
		if(brandWeight>0.8)
			ifInFullBrand = true;
		else if(brandWeight>0.3)
			ifInPartBrand = true;
		
		fieldScr = ifInFullBrand ? 4 : (ifInPartBrand ? 3 : (ifInTitle ? 2 : (ifInSyn ? 1 : 0)));
	}
	{//Ʒ�Ʒ���
		GetFeedBackBrandCateWeight(docID,pa,fbBrandCateWeight);
	}
	//��ҵȨ��:
	int commerceScr = 0; //[0-6]
	int saleDetail = 0;
	{
		ComputeCommerceWeight(docID, commerceScr, saleDetail);
	}
	int sale = m_funcFrstInt(m_saleWeekProfile, docID);

	//�������¼
	size_t nCnt = 0;
	u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //�������ָ��
	int cateNum = 0;		//��¼�׼����ţ�cateStat���±�+1��
	int relScr = 0;			//������ر�ʶ[0-2]
	if (ifInFullBrand) //��Ʒ���ֶ��е�������Ʒ����
	{
		relScr = 2;
		++pa->m_mallAnalysisPart.relCnt_brd; //��Ʒ���ֶ��е���Ʒ��
		for (int k=0; k<nCnt; k++) { //�����õ�Ʒ�������
			//ע�������id�����ݿ��еĲ�ͬ�����Է������ݵ�idҪת��������
			u64 bottomCate = ptr[k];	//�׼���
			int idx = -1;																	//cateStat�±�
			map<u64, int>::const_iterator cit = pa->m_mallAnalysisPart.cid2cnum.find(bottomCate);
			if (cit != pa->m_mallAnalysisPart.cid2cnum.end())
			{	//���д����
				idx = pa->m_mallAnalysisPart.cid2cnum[bottomCate];
			}
			else
			{	//���޴����
				idx = (int)pa->m_mallAnalysisPart.cateStat.size();
				pa->m_mallAnalysisPart.cid2cnum[bottomCate] = idx;
				pa->m_mallAnalysisPart.cateStat.push_back(CateStat());
			}

			if (idx >= 0 && idx < (int)pa->m_mallAnalysisPart.cateStat.size())
			{
				cateNum = idx + 1;
				++pa->m_mallAnalysisPart.cateStat[idx].pidCnt; //����е���Ʒ��
				pa->m_mallAnalysisPart.cateStat[idx].saleSum += sale;
			}
			u64 top_cate = GetClassByLevel(1, ptr[k]);
			pa->m_mallAnalysisPart.topCate_brd.insert(top_cate); //��ض����༯��
		}
	}
	else if (ifInTitle)
	{
		relScr = 1;
		++pa->m_mallAnalysisPart.relCnt_ti;
		for (int k=0; k<nCnt; k++) { //�����õ�Ʒ�������
			u64 top_cate = GetClassByLevel(1, ptr[k]);
			pa->m_mallAnalysisPart.topCate_ti.insert(top_cate);//��ض�����
		}
	}

	//������Ϣ(����Ʒ)
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
		+ fbBrandCateWeight * Weight_FBCate     //Ʒ�Ʒ���Ȩ��[0,1] 100
		+ commerceScr * Weight_Commerce //0
		+ ifPublic * Weight_Public //10
		+ saleDetail * Weight_Sale; //0

	rt.nWeight += commerceScr * CommerceFactor  //0
		+ relScr * RelIndicator //10000
		+ fieldScr * FieldFactor //1000
		+ cateNum; //Ʒ���ֶΰ���query����Ʒ�����
}
//�õ�Ʒ�Ƶ������Ȩ��
void CSearchKeyRanking::GetFeedBackBrandCateWeight(int docID,CDDAnalysisData* pa, int& fbBrandCateWeight)
{
	bool match = false;
	u64 cate_id = 0;
	u64 bottomCate = 0;
	int weight = 0;
	size_t nCnt = 0;
	fbBrandCateWeight = 0;
	u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //�������ָ��
	int cate_count = m_key2CateBrd[pa->m_mallAnalysisPart.querystr].count;
	if (cate_count != 0)
	{
		//�����
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

//@todo: catePath�е����id�Ǽ�������ģ������ݿ��еĲ�ͬ�����Է���������
//       �����id��Ҫת������catepath�е�һ��
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
	u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //�������ָ��

	if (false == pa->m_mallAnalysisPart.vCate.empty()) //������𼯺ϲ�Ϊ��
	{ //����������÷�
		const vector<pair<u64, int> >& fbCate = pa->m_mallAnalysisPart.vCate;
		//multimap<int, vector<int> >::const_iterator lower = pid2cate.find(pid);
		//if (lower != pid2cate.end())
		//{	//�������Ϣ
		//	multimap<int, vector<int> >::const_iterator upper =
		//		pid2cate.upper_bound(pid);
			bool match = false;

		//	while (lower != upper && false == match)
		//	{	//for every catePath of pid
		//		const vector<int> &catePath = lower->second;
			for (int k=0; k<nCnt && false == match; k++) { //�����õ�Ʒ�������
                //���id->���·���ĸ������id
                //ע�������id�����ݿ��еĲ�ͬ�����Է������ݵ�idҪת��������
				u64 bottomCate = ptr[k];	//@fixed bug: not nCnt, but k �׼���
				/*{
					cout << "bottomCate = " << bottomCate << endl;
					char cate_path[40];
					TranseID2ClsPath(cate_path, bottomCate, 6);
					cout << "cate_path = " << cate_path << endl;
				}*/
				//if (!catePath.empty())
				{
					for (size_t i = 0; i < fbCate.size(); ++i)
					{	//�����׼���
						if (bottomCate == fbCate[i].first) //��ǰ��Ʒ����뷴�������ƥ��
						{
							fbCateWeight = fbCate[i].second;	//weight
							match = true;
							u64 top_cate = GetClassByLevel(1, bottomCate);
							pa->m_mallAnalysisPart.topCate_fb.insert(top_cate);//��ض�����
							++pa->m_mallAnalysisPart.relCnt_fb; //���ڷ�������е���Ʒ��Ŀ
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
	{	//���㵥Ʒ�����÷�
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
	{	//�����Ʒ����Ȩ��
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
			{	//��Ʒƥ��
				pdtCoreWeight = 2;

				//multimap<int, vector<int> >::const_iterator lower = pid2cate.find(pid);
				//if (lower != pid2cate.end())
				//{	//�������Ϣ
				//	multimap<int, vector<int> >::const_iterator upper = pid2cate.upper_bound(pid);

				//	while (lower != upper)
					for (int k=0; k<nCnt; k++) { //�����õ�Ʒ�������
						                         //for every catePath of pid
				//		pa->m_mallAnalysisPart.topCate_prd.insert(lower->second[0]);//��ض�����
						u64 top_cate = GetClassByLevel(1, ptr[k]);
					    pa->m_mallAnalysisPart.topCate_prd.insert(top_cate);
				//		++lower;
					}
					++pa->m_mallAnalysisPart.relCnt_prd; //���Ĵ�ƥ�����Ʒ��Ŀ
					if (0 == relScr)
					{
						relScr = 1;
					}
				//}
				break;
			}
			else if (!pa->m_mallAnalysisPart.pdtNg.empty() && key_size >= 2
				&& (0 == pa->m_mallAnalysisPart.pdtNg.compare(0, 2, keys.data+i*keys.size, key_size-2, 2)))
			{	//����ƥ��
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
	FilterMallResults(vRes); //������
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
