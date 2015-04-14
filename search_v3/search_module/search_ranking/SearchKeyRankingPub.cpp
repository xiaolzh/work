#include "SearchKeyRanking.h"
#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <math.h>
//#include "QuerySegment.h"

static inline bool filter_small_ext(const SResult& di)	
{
	return (di.nScore <= 0) ||
		(1 == di.nWeight / CSearchKeyRanking::ClusterFactor % 10 //С����
		 && 1 != di.nWeight / CSearchKeyRanking::BaseRelFactor % 10 //�����
		 && di.nWeight / CSearchKeyRanking::PubCommerceFactor < 6);
	//��ҵ��ֵ��	
}

static inline bool filter_small_ext_all(const SResult& di)	
{
	return (di.nScore <= 0) ||
		(1 == di.nWeight / CSearchKeyRanking::ClusterFactor % 10 //С����
		 && 1 != di.nWeight / CSearchKeyRanking::BaseRelFactor % 10); //�����	
}

//����0ֵ���
static inline bool filter_zero_weight(const SResult& di)
{
	return di.nScore <= 0;	
}

static inline int getSaleScr(int sale)	
{
	if (sale)
	{	//7����������
		if (sale < 300)
		{
			if (sale < 10)
			{
				return 4;
			}
			else if (sale < 80)
			{
				return sale < 35 ? 5 : 6;
			}
			else
			{
				return sale < 160 ? 7 : 8;
			}
		}
		else
		{
			return 9;
		}
	}
	return 0;
}

/*inline void CSearchKeyRanking::GetPathID(const string& path,int &classid)
{
	hash_map<string>::iterator iter;
	iter = clsid.find(path);
	if(iter != clsid.end())
		classid = iter->second;
}*/

vector<string> CSearchKeyRanking::GetStrings(const string& stringList, const string& splitWord)
{
	vector<string> result;
	if (splitWord == "")
	{
		// ���ָ��־Ϊ��ʱ������Ϊȫ���ַ������ر��ָֱ�ӷ��ؼ���
		result.push_back(stringList);
	}
	else
	{
		string stringList_cpy = splitWord + stringList;
		string::size_type lastPos = 0;
		while (stringList_cpy[stringList_cpy.size()-1] == '\n'
				||
				stringList_cpy[stringList_cpy.size()-1] == '\r')
		{
			stringList_cpy.resize(stringList_cpy.size() - 1);
		}
		lastPos = stringList_cpy.find_last_of(splitWord);
		// ȥ��β���ġ�slitWord��
		while (lastPos == stringList_cpy.size()-1)
		{
			stringList_cpy.resize(stringList_cpy.size() - splitWord.size());
		}
		string::size_type firstPos = 0;
		string::size_type secondPos = 0;
		firstPos = stringList_cpy.find(splitWord, 0);
		while (firstPos != string::npos)
		{
			secondPos = stringList_cpy.find(splitWord, firstPos+splitWord.size());
			string curItem; // ��ʱ�洢��ǰ���ָ���Ӵ�
			if (secondPos == string::npos)
			{
				curItem = stringList_cpy.substr(firstPos+splitWord.size(), stringList_cpy.size()-firstPos-splitWord.size());
			}
			else
			{
				curItem = stringList_cpy.substr(firstPos+splitWord.size(), secondPos-firstPos-splitWord.size());
			}
			if (curItem != "")
			{
				// �����ַ����в����зָ����
				result.push_back(curItem);
			}
			// ����ƶ�firstPos
			firstPos = secondPos;
		}
	}
	return result;
}

bool CSearchKeyRanking::LoadFile(SSearchDataInfo* psdi, const string& strConf)
{
	pid2tags.read(m_strModulePath+"query_tags");
	pid2sub.read(m_strModulePath+"substrs");
	string path = m_strModulePath;
	path.append("author_pub");
	AurPubKey.load_serialized_hash_file(path.c_str(),-1);
	Query2Cate.read(m_strModulePath+"query_cate");
	cid2cids.read(m_strModulePath+"ext_cate");
				
}

/*bool CSearchKeyRanking::LoadFile(SSearchDataInfo* psdi, const string& strConf)
{
	
	{       //load hotNPCate
                string hot_cate = "/hotNPCate.txt";
                ifstream fin(hot_cate.c_str());
                if (!fin.is_open())
                {
                        printf("%s file not open.\n",hot_cate.c_str());
                }
                else
                {
                        string line;
                        while (getline(fin, line))
                        {
                                hotNPCate.insert(atoll(line.c_str()));
                        }
                        printf("hotNPCate:%d\n", hotNPCate.size());
                }
        }
}*/

bool CSearchKeyRanking::InitPub(SSearchDataInfo* psdi, const string& strConf)
{
	LoadFile(psdi,strConf);
	m_SalePriceProfile = FindProfileByName("dd_sale_price");
        if (m_SalePriceProfile == NULL)
        {
                COMMON_LOG(SL_ERROR, "sale_price field does not exist");
                return false;
        }
        m_sale_dayProfile =  FindProfileByName("sale_week");
        if (m_sale_dayProfile == NULL)
        {
                COMMON_LOG(SL_ERROR, "sale_week field does not exist");
                return false;
        }
        m_pre_saleProfile =  FindProfileByName("pre_sale");
        if (m_pre_saleProfile == NULL)
        {
                COMMON_LOG(SL_ERROR, "pre_sale field does not exist");
                return false;
        }
        m_num_imagesProfile = FindProfileByName("num_images");
        if (m_num_imagesProfile == NULL)
        {
                COMMON_LOG(SL_ERROR, "num_images field does not exist");
                return false;
        }
        m_TOTAL_REVIEW_COUNTProfile = FindProfileByName("total_review_count");
        if (m_TOTAL_REVIEW_COUNTProfile == NULL)
        {
                COMMON_LOG(SL_ERROR, "total_review_count field does not exist");
                return false;
        }
	
	PMinNum = 6;
	TITLEID =10;
	TITLEPRI = 11;
	TITLESUB = 12;
	TITLEEX = 9;
	AUTHOR = 8;
	PUBNAME = 7;
	ISBN = 6;
	SERIES = 5;
	ABSTRACT = 4;
	CONTENT = 3;
	COMMENT = 2;
	CATALOG = 1;

		

	FID_AUTHOR_NAME = GetFieldId("author_name");
	if (FID_AUTHOR_NAME == -1)
	{
		COMMON_LOG(SL_ERROR, "author_name field does not exist");
		return false;
	}
	else
		fid2fi[FID_AUTHOR_NAME] = AUTHOR;
	COMMON_LOG(SL_INFO, "author_name field is ok!!");	

	FID_TITLE = GetFieldId("product_name");
        if (FID_TITLE == -1)
        {
		COMMON_LOG(SL_ERROR, "product_name field does not exist");
                return false;
        }
	else
		fid2fi[FID_TITLE] = TITLEID;
	COMMON_LOG(SL_INFO, "product_name field is ok!!");
	PTITLE_FID = TITLEID;
	
        FID_TITLEEX = GetFieldId("title_synonym");
        if (FID_TITLEEX == -1)
        {
		COMMON_LOG(SL_ERROR, "title_synonym field does not exist");
                return false;
        }
	else
		fid2fi[FID_TITLEEX] = TITLEEX;
	COMMON_LOG(SL_INFO, "title_synonym field is ok!!");
	TITLE_EX_FID = TITLEEX;
	
        FID_PUBNAME = GetFieldId("publisher");
        if (FID_PUBNAME == -1)
        {
		COMMON_LOG(SL_ERROR, "publisher field does not exist");
                return false;
        }
	else
		fid2fi[FID_PUBNAME] = PUBNAME;
	COMMON_LOG(SL_INFO, "publisher field is ok!!");

        FID_TITLEPRI = GetFieldId("title_primary");
        if (FID_TITLEPRI == -1)
        {
		COMMON_LOG(SL_ERROR, "title_primary field does not exist");
                return false;
        }
	else
		fid2fi[FID_TITLEPRI] = TITLEPRI;
	COMMON_LOG(SL_INFO, "title_primary field is ok!!");
	TITLE_PRI_FID = TITLEPRI;

        FID_TITLESUB = GetFieldId("title_sub");
        if (FID_TITLESUB == -1)
        {
		COMMON_LOG(SL_ERROR, "title_sub field does not exist");
                return false;
        }
	else
		fid2fi[FID_TITLESUB] = TITLESUB;
	COMMON_LOG(SL_INFO, "title_sub field is ok!!");
	TITLE_SUB_FID = FID_TITLESUB;	

	FID_ISBNSEARCH = GetFieldId("isbn_search");
	if(FID_ISBNSEARCH == -1)
	{
		COMMON_LOG(SL_ERROR, "isbn_search field does not exist");	
		return false;
	}
	else
		fid2fi[FID_ISBNSEARCH] = ISBN;
	COMMON_LOG(SL_INFO, "isbn_search field is ok!!");

        FID_SINGER = GetFieldId("singer");
        if(FID_SINGER == -1)
        {
		COMMON_LOG(SL_ERROR, "singer field does not exist");
                return false;
        }
	else
		fid2fi[FID_SINGER] = AUTHOR;
	COMMON_LOG(SL_INFO, "singer field is ok!!");
        /*FID_CONDUCTOR = GetFieldId("conductor");
        if(FID_CONDUCTOR != -1)
        {
                printf("conductor field dose not exist\n");
                return false;
        }
	fid2fi[FID_CONDUCTOR] = AUTHOR;*/
        FID_DIRECTOR = GetFieldId("director");
        if(FID_DIRECTOR == -1)
        {
		COMMON_LOG(SL_ERROR, "director field does not exist");
                return false;
        }
	else
		fid2fi[FID_DIRECTOR] = AUTHOR;
	COMMON_LOG(SL_INFO, "director field is ok!!");
        FID_ACTOR = GetFieldId("actors");
        if(FID_ACTOR == -1)
        {
		COMMON_LOG(SL_ERROR, "actors field does not exist");
                return false;
        }
	else
		fid2fi[FID_ACTOR] = AUTHOR;
	COMMON_LOG(SL_INFO, "director field is ok!!");
	
	FID_SERIES = GetFieldId("series_name");
	if(FID_SERIES == -1)
	{
		COMMON_LOG(SL_ERROR, "series_name field does not exist");
		return false;
	}
	else
		fid2fi[FID_SERIES] = SERIES;
	COMMON_LOG(SL_INFO, "series_name field is ok!!");
	
	FID_ABSTRACT =  GetFieldId("abstract");
        if(FID_ABSTRACT == -1)
        {
		COMMON_LOG(SL_ERROR, "abstract field does not exist");
                return false;
        }
	else
		fid2fi[FID_ABSTRACT] = ABSTRACT;
	COMMON_LOG(SL_INFO, "abstract field is ok!!");
	
	FID_CONTENT = GetFieldId("content");
        if( FID_CONTENT == -1)  
        {
		COMMON_LOG(SL_ERROR, "content field does not exist");
                return false;
        }
	else
		fid2fi[FID_CONTENT] = CONTENT;
	COMMON_LOG(SL_INFO, "content field is ok!!");
		
/*
	FID_COMMENT = GetFieldId("product_info");
        if( FID_COMMENT == -1)
        {
		COMMON_LOG(SL_ERROR, "product_info field does not exist");
                return false;
        }
	else
		fid2fi[FID_COMMENT] = COMMENT;
	COMMON_LOG(SL_INFO, "product_info field is ok!!"); 
*/

	FID_CATALOG = GetFieldId ("catalog");
        if( FID_CATALOG == -1)
        {
		COMMON_LOG(SL_ERROR, "catalog field does not exist");		
                return false;
        }
	fid2fi[FID_CATALOG]= CATALOG;
	COMMON_LOG(SL_INFO, "catalog field is ok!!");
	COMMON_LOG(SL_INFO, "Init is ok!!");
	return true;
}

void CSearchKeyRanking::ComputeWeightPub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//printf("size=%d\n", me.vTerms.size());
        me.vTerms.size() == 1 ? RankingSinglePub(pa,me,rt):RankingMultiPub(pa,me,rt);
}

inline void CSearchKeyRanking::JudgePidHasQueryTag(
                const int id, const CDDAnalysisData *analysisDat,
                bool& fbCateScr, int& fbPidScr)
{ //[0,1],[0,2](ֻ�Ƚϵ�һ����)
        fbCateScr = fbPidScr = 0;
        //�����

	//size_t nCnt = 0;
	//u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //�������ָ��
	const vector<pair<int,int> > &fbCate = analysisDat->pubanalysisdata.objQuery.fbCate;


	u64 cls = (u64)m_funcFrstInt64(m_clsProfile,id);
	//vector<u64> catePath;
	//TranseClsID2ClsIDs(catePath, cls);
	for(size_t t = 0; t < fbCate.size();t++)
	{
		if (GetClassByLevel(GetClsLevel(fbCate[t].first), cls) == fbCate[t].first)
		{
			fbCateScr = 1;
			if(fbCate[t].second > 0)
			{
				fbPidScr = 1;
			}
		}

	}
	//��Ʒ����
	int pid = (int)m_funcFrstInt64(m_isPidProfile,id);
	string queryValidString = analysisDat->pubanalysisdata.objQuery.validString;
        //multimap<int, string>::const_iterator pit
               // = pid2tags.find(pid);
	//vector<pair<string,int> > vec = pid2tags[pid];
	HASHVECTOR hvect;
	hvect = pid2tags[pid];
	char strValue[20];
        if (hvect.count !=0)
        {
                //for (; pit != pid2tags.upper_bound(pid); ++pit)
		for(size_t t = 0; t < hvect.count;t++)
                {
                       //if (queryValidString == hvect[t].first)
			memcpy(&strValue,hvect.data+t*hvect.size,20);
			if(strValue == queryValidString)
                        {
                                fbPidScr = 3;
                                fbCateScr = 1;
                                break;
                        }
                }
        }
}

inline int CSearchKeyRanking::ComputeDis(
                const SMatchElement& me, size_t fidlmt)
{       //����ĳһָ���ֶη�Χ�ڵĳ��ִʴʾ�
        bool fieldHasKeyword = false;
        int accDis = 0;
        vector<int> pos, cmppos;
        int cnt, ivtCnt, minDis, tmpDis, keylen;
        for (size_t k = 0; k < me.vFieldsOff.size(); ++k)
        {
                if((size_t)me.vFieldsOff[k].field == fidlmt)
                {
                        cmppos.push_back((int)me.vFieldsOff[k].off);
                }
                if (!cmppos.empty())
                {
                        if (!pos.empty())
                        {
                                minDis = INT_MAX;
                                for (size_t pi = 0; pi < pos.size(); ++pi)
                                {
                                        for (size_t pj = 0; pj < cmppos.size(); ++pj)
                                        {
                                                tmpDis = abs(cmppos[pj]- pos[pi]);
                                                if (tmpDis < minDis)
                                                {
                                                        minDis = tmpDis;
                                                }
                                        }
                                }
                                if (minDis < INT_MAX)
                                {
                                        accDis += minDis;
                                }
                        }
                        pos.swap(cmppos);
                        //keylen = vKeywords[k].keyLength;
                        keylen = me.vTerms[k].len;
                        for (size_t p = 0; p < pos.size(); ++p)
			{
                                pos[p] += keylen;
                        }
                }


        }
        if (!fieldHasKeyword)
        {
                return -1;
        }
        else
        {
                return accDis;
        }
}

inline bool CSearchKeyRanking::JudgeDis(const CDDAnalysisData *pa,
                const SMatchElement& me, int fidlmt)
{ //��ָ�������ж��Ƿ��дʾ�ƥ����(-1-ȫ��-2-����fid)
        static const int OFFSETMAX = 1000;  //���ƫ��
        static const int FLDFACTOR = 10;         //�ֶγ���(2^FLDFACTOR>OFFSETMAX)
        static const int STRICTDISMAX = 2; //�ϸ�ʾ�
        if (me.vTerms.size() <= 1)
        {
                return true;
        }
	
        static const int PMinNum = PMinNum;
        //map<int, int> &fid2fi = m_rscSharedMod->fid2fi;
        vector<int> lastKeyPos; //ǰ�ʳ���λ��
        vector<bool> idx;                               //�������ʾ�λ��
        int fwdDis, cnt,ivtCnt, klen, fidi,fid, offset, allOffset;
	map<int, int>::const_iterator iter;
        for (size_t k = 0; k < me.vTerms.size(); ++k)
        {       //������title >> series >> T >> B
                fwdDis = pa->pubanalysisdata.objQuery.dis[k];
                klen = me.vTerms[k].len;
                if (0 == k ||true == pa->pubanalysisdata.objQuery.ifSP[k] ||
                                (true == pa->pubanalysisdata.objQuery.keysType[k] || true == pa->pubanalysisdata.objQuery.keysType[k-1]))
                { //�״ʣ�����ǰ�ʸ��ո�,��Ϊ���߻�����磬������ʾ�
                        //�״ʣ�����ǰ�ʸ��ո񲻼���ʾ�
                        lastKeyPos.clear();
                        fidi = (int)me.vFieldsOff[k].field;
			iter = fid2fi.find(fidi);
			if(iter != fid2fi.end())
				fid = iter->second;
                        if( fidlmt == -1 || fid == fidlmt ||
                                        (fidlmt == -2 && fid2fi.find(fid) != fid2fi.end()
                                        && fid2fi.find(fid)->second >= PMinNum))
                        {
                                offset = (int)me.vFieldsOff[k].off;
                                if(offset < OFFSETMAX)
                                {
                                        lastKeyPos.push_back((fid<<FLDFACTOR) + offset + klen);
                                }
                        }
                }
                else if (lastKeyPos.empty())
                {
                        break;
                }
                else
                {
                        idx.clear();
                        idx.resize(lastKeyPos.size(), false);
                        {
                                fidi = (int)me.vFieldsOff[k].field;
				iter = fid2fi.find(fidi);
				if(iter != fid2fi.end())
					fid = iter->second;
                                if( fidlmt == -1 || fid == fidlmt ||
                                        (fidlmt == -2 && fid2fi.find(fid) != fid2fi.end()
                                        && fid2fi.find(fid)->second >= PMinNum))
                                {
                                        offset = (int)me.vFieldsOff[k].off;
                                        if(offset < OFFSETMAX)
                                        {
                                                allOffset = (fid <<FLDFACTOR) + offset;
                                                for(size_t p =0; p < lastKeyPos.size(); ++p)
                                                {
                                                        if( false == idx[p] && allOffset - lastKeyPos[p] >= 0
                                                                && allOffset - lastKeyPos[p] <= fwdDis + STRICTDISMAX)
                                                        {
                                                                lastKeyPos[p] = allOffset + klen;
                                                                idx[p] = true;
                                                        }
                                                }
                                        }
                                }
                        }
                        size_t sz = 0;
                        for (size_t x = 0; x < idx.size(); ++x)
			{
                                if (true == idx[x])
                                {
                                        lastKeyPos[sz] = lastKeyPos[x];
                                        ++sz;
                                }
                        }
                        lastKeyPos.resize(sz);
                        if (lastKeyPos.empty())
                        {
                                break;
                        }
                }
        }
        return lastKeyPos.empty() ? false : true;
}

inline bool CSearchKeyRanking::JudgeDisFilter(
                const SMatchElement& me)
{       //�ʾ����
        if (me.vFieldsOff.size() <= 1)
        {
                return true;
        }
        static const int OFFSETMAX = 1000;  //���ƫ��
        static const int FLDFACTOR = 10;         //�ֶγ���(2^FLDFACTOR>OFFSETMAX)
        vector<pair<int, vector<int> > > vPos;
        int cnt, ivtCnt, fid, offset;
        vector<int> pTmp;
        //map<int, int> &fid2fi = fid2fi;
        size_t valCnt = 0;      //��Ч����
        for (size_t k = 0; k < me.vFieldsOff.size(); ++k)
        {       //������title >> series >> T >> B
                //if (vKeywords[k].keyWeight > 0)
                if(static_cast<int>(log(1/me.vIdf[k]))>0)
                {       //ֻ��¼��Ч��
                        ++valCnt;
                        pTmp.clear();
                        {
                                fid = (int)me.vFieldsOff[k].field;
				if(fid2fi.find(fid) != fid2fi.end())
                                {
                                        offset = (int)me.vFieldsOff[k].off;
                                        if(offset < OFFSETMAX)
                                        {
                                                pTmp.push_back((fid<<FLDFACTOR) + offset);
                                        }
                                }
                        }
                        if (!pTmp.empty())
                        {
                                vPos.push_back(
                                                pair<int, vector<int> >(k, pTmp));
                        }
                }
        }
        size_t N = vPos.size();
        assert(N <= valCnt);
        if (N < 2)
        {
                return false;
        }
        int accDis = 0;         //�ۼƴʾ�
        size_t nextCnt = 0;     //���ڸ���
        {
                int len_i, len_j, idx_i, idx_j, fwdItvlDis;
                int tmpDis, minDis;
                for (size_t i = 0; i < N; ++i)
                {
                        idx_i = vPos[i].first;
                        //len_i = vKeywords[idx_i].keyLength;
                        //len_i = me.vFieldLen[idx_i];
                        len_i = me.vTerms[idx_i].len;
                        vector<int>& pos_i = vPos[i].second;
                        for (size_t j = i+1; j < N; ++j)
                        { //for one pair
                                idx_j = vPos[j].first;
                                //len_j = vKeywords[vPos[j].first].keyLength;
				len_j = me.vTerms[vPos[j].first].len;
                                vector<int>& pos_j = vPos[j].second;
                                assert(idx_i < idx_j);
                                fwdItvlDis = 0;         //������
                                for (int idx = idx_i+1; idx < idx_j; ++idx)
                                {
                                        //fwdItvlDis += vKeywords[idx].keyLength + vKeywords[idx].dis;
                                        //fwdItvlDis += me.vFieldLen[idx] + me.vAllowGap[idx];
                                        fwdItvlDis += me.vTerms[idx].len + me.vAllowGap[idx];
                                }
                                //fwdItvlDis += vKeywords[idx_j].dis;
                                fwdItvlDis += me.vAllowGap[idx_j];
                                //�������ʾ�����̾���
                                minDis = INT_MAX;
                                for (size_t pi = 0; pi < pos_i.size(); ++pi)
                                {
                                        for (size_t pj = 0; pj < pos_j.size(); ++pj)
                                        {
                                                if (pos_i[pi] != pos_j[pj])
                                                {
                                                        tmpDis = abs( (pos_i[pi] < pos_j[pj]) ?
                                                                        (pos_j[pj] - pos_i[pi] - len_i - fwdItvlDis)
                                                                        : (pos_i[pi] - pos_j[pj] - len_j) );
                                                        if (tmpDis < minDis)
                                                        {
                                                                minDis = tmpDis;
                                                        }
                                                }
                                        }
                                }
                                accDis += minDis;
                                if (minDis <= 2)
                                {
                                        ++nextCnt;
                                }
                        }
                }
        }
	if (accDis >= 0 && accDis <= 10)
        {
                return true;
        }
        else
        {
                if (valCnt & 1)
                { //����
                        return nextCnt >= (valCnt > 5 ? (valCnt-1) * (valCnt-1) / 4 : valCnt);
                }
                else
                {       //ż��
                        return nextCnt >= (valCnt > 4 ? valCnt * (valCnt-2) / 4 : valCnt - 1);
                }
        }
}

inline bool CSearchKeyRanking::JudgeTotalMatch(
                const string& queryValidString,
                const SMatchElement& me,
                int fid,
                int pos_title)
{
        if (pos_title <= 2)
        {       //�����ֶ�ƥ��
                int queryLen = (int)queryValidString.size();
		int fieldLen = 0;
		for(size_t t = 0; t < me.vFieldsOff.size();t++)
		{
			if(fid == (int)me.vFieldsOff[t].field)
			{
				fieldLen = me.vFieldLen[t];
				break;
			}
		}
                //int fieldLen = len;
                if ( fieldLen == queryLen ||
                                (fieldLen > 4 && abs(fieldLen - queryLen) <= 2))
                {
                        return true;
                }
        }
        if (fid == PTITLE_FID)
        {       //���������Ӵ�ƥ��
                //hash_map<int, pair<string, string> >::const_iterator it;
		int pid = (int)m_funcFrstInt64(m_isPidProfile, me.id);
                //it = pid2sub.find(pid);
		
		//vector<string> vec = pid2sub[pid];
		HASHVECTOR hvect;
		hvect = pid2sub[pid];
                //if ( it != pid2sub.end()
                               // && (it->second.first == queryValidString
                                       // || it->second.second == queryValidString) )
		//if(vec.size() != 0)
		char strValue[20];
		if(hvect.count != 0)
                {
			for(size_t t = 0; t < hvect.count;t++)
			{
				//if(vec[t] == queryValidString)
				memcpy(&strValue,hvect.data+t*hvect.size,20);
				if( queryValidString == strValue)
				{
					//break;
		                        return true;
				}
			}
                }
        }
        return false;
}

inline void CSearchKeyRanking::RecordCate(
                vector<set<u64> >& vCluster, map<u64, vector<u64> >& highRelSaleCate,
                int id, int cateLevel, int highRelSale, bool ifAutPubSearch)
{
        if (cateLevel <= 0 || cateLevel > (int)vCluster.size())
        {
                return;
        }
	u64 cls = (u64)m_funcFrstInt64(m_clsProfile,id);
	int clsid = (int)cls;
	if(cls != 0)
        { //�������Ϣ(ֻ��¼��һ����)
                vCluster[(int)vCluster.size() - cateLevel].insert(clsid);
                if (highRelSale > 0)
                {       //�����������
                        if (ifAutPubSearch)
                        {       //������������������
                                //������
                                highRelSaleCate[GetClassByLevel(2,clsid)].push_back(highRelSale);
                        }
                        else
                        {       //��������
                                highRelSaleCate[GetClassByLevel(3,clsid)].push_back(highRelSale);
                        }
                }
        }
}

static inline int ComputeCommerce(
                int sale, int evalue, bool newProduct, bool presale,
                int& commerScr, int& saleScr)
{
	commerScr = 0;
	saleScr = 0;
	{
		if (sale)
			//if(stock)
		{       //7����������
			/*commerScr = 8;
			  if(newProduct)
			  ++commerScr;*/
			if (sale < 300)
			{
				if (sale < 10)
				{
					commerScr = 4;
				}
				else if (sale < 80)
				{
					commerScr = sale < 35 ? 5 : 6;
				}
				else
				{
					commerScr = sale < 160 ? 7 : 8;
					saleScr = sale >> 6;
				}
				if (newProduct)
				{
					++commerScr;
				}
			}
			else
			{
				commerScr = 9;
				saleScr = sale >> 6;
			}
		}
		else
		{       //7����������
			if (newProduct)
			{       //��Ʒ
				commerScr = 3;
				if (presale)
				{       //Ԥ��
					++commerScr;
				}
			}
			else if (evalue >= 3)
			{       //��������
				if (evalue >= 100)
				{
					commerScr = 3;
				}
				else
				{
					commerScr = evalue >= 35 ? 2 : 1;
				}
			}
		}
	}
}

//�������ȼ�[1-4]
static inline int ComputeCateLevel(bool feedback, int fieldScr)
{
        if (feedback)
        {       //����4�ȼ�
                return 4;
        }
        else if (fieldScr > 3)
        {       //�ı�3�ȼ���1-3��
                return (fieldScr>>1) - 1;
        }
        else
        {
                return 0;
        }
}

//IAnalysisData* CSearchKeyRanking::QueryAnalyse(SQueryClause& qc)
void CSearchKeyRanking::QueryAnalysePub(CDDAnalysisData* pa, SQueryClause& qc)
{
        vector<QueryItem> queryItems;
        QueryParse(qc.key, qc.vTerms, queryItems);
        //CDDAnalysisData* analysisData = new CDDAnalysisData();
        size_t queryKeyCnt = queryItems.size();
        pa->pubanalysisdata.objQuery.type.resize(queryKeyCnt, 0);
        pa->pubanalysisdata.objQuery.keysRepeat.resize(queryKeyCnt, 0);
        pa->pubanalysisdata.objQuery.keyWeight.resize(queryKeyCnt,0);
        pa->pubanalysisdata.objQuery.dis.resize(queryKeyCnt,0);
        pa->pubanalysisdata.objQuery.keysType.resize(queryKeyCnt,0);
        pa->pubanalysisdata.objQuery.ifSP.resize(queryKeyCnt,0);
	pa->pubanalysisdata.vCluster.resize(4);
        size_t singleCnt = 0;
	
	bool aurpub = false;
	
        for(size_t i = 0; i < queryKeyCnt;i++)
        {
                string key = queryItems[i].word;
                if(queryItems[i].length <= 2)
                { //������
                        ++singleCnt;
                }
		//printf("AurPubKey[%s]=%d\n",key.c_str(),AurPubKey[key]);
                if (AurPubKey[key] != -1)
		{
			aurpub = true;
			pa->pubanalysisdata.objQuery.ifAutPubSearch = pa->pubanalysisdata.objQuery.keysType[i] = AurPubKey[key];	
		}
                if (false == queryItems[i].field.empty())
                { //�߼�����
                        pa->pubanalysisdata.objQuery.ifLimitSearch = true;
                }
                if (i > 0 && queryItems[i].fwdSP)
                { //����ո�
                        pa->pubanalysisdata.objQuery.ifSingleString = false;
                }
                pa->pubanalysisdata.objQuery.dis[i] = queryItems[i].fwdDis;
                pa->pubanalysisdata.objQuery.ifSP[i] = queryItems[i].fwdSP;
                /*if (0 == objQI.keysRepeat[i])
		{       //�ظ��ʼ��鼰��¼
                        int count = 0;
                        for (size_t j = i+1; j < keywordCnt; ++j)
                        {
                                if (key == queryItems[j].words)
                                {
                                        analysisData->pubanalysisdata.objQuery.keysRepeat[j] = ++count + (i << 8);
                                }
                        }
                }*/
        }
	if(aurpub)
	{
		pa->m_queryFieldIds.push_back(FID_AUTHOR_NAME);
		pa->m_queryFieldIds.push_back(FID_TITLE);
		pa->m_queryFieldIds.push_back(FID_TITLEEX);
		pa->m_queryFieldIds.push_back(FID_PUBNAME);
		pa->m_queryFieldIds.push_back(FID_TITLEPRI);
		pa->m_queryFieldIds.push_back(FID_TITLESUB);
		pa->m_queryFieldIds.push_back(FID_SINGER);
		pa->m_queryFieldIds.push_back(FID_DIRECTOR);
		pa->m_queryFieldIds.push_back(FID_ACTOR);
	}
	else
	{
		pa->m_queryFieldIds.push_back(FID_AUTHOR_NAME);
                pa->m_queryFieldIds.push_back(FID_TITLE);
                pa->m_queryFieldIds.push_back(FID_TITLEEX);
                pa->m_queryFieldIds.push_back(FID_PUBNAME);
                pa->m_queryFieldIds.push_back(FID_TITLEPRI);
                pa->m_queryFieldIds.push_back(FID_TITLESUB);
                pa->m_queryFieldIds.push_back(FID_SINGER);
                pa->m_queryFieldIds.push_back(FID_DIRECTOR);
                pa->m_queryFieldIds.push_back(FID_ACTOR);
		pa->m_queryFieldIds.push_back(FID_SERIES);
		pa->m_queryFieldIds.push_back(FID_ABSTRACT);
		pa->m_queryFieldIds.push_back(FID_CONTENT);
//		pa->m_queryFieldIds.push_back(GetFieldId("product_info"));
		pa->m_queryFieldIds.push_back(FID_CATALOG);
		pa->m_queryFieldIds.push_back(FID_ISBNSEARCH);
	}
        pa->pubanalysisdata.objQuery.validString = qc.key;
        if (singleCnt == queryKeyCnt)
        { //ȫ����
                pa->pubanalysisdata.objQuery.ifAllSingleWord = true;
        }
        /*hash_map<string, vector<pair<u64,bool> > >::const_iterator hit =
                Query2Cate.find(pa->pubanalysisdata.objQuery.validString);*/
	//vector<pair<int,int> > vec = Query2Cate[pa->pubanalysisdata.objQuery.validString];
	HASHVECTOR hvect;
	hvect = Query2Cate[pa->pubanalysisdata.objQuery.validString];
        //if (hit != Query2Cate.end())
	//if(vec.size() != 0)
	if(hvect.count != 0)
        { //�з���
                pa->pubanalysisdata.objQuery.ifHasFeedback = true;
		int cid,score;
		for(size_t t = 0; t < hvect.count;t++)
		{
                	//pa->pubanalysisdata.objQuery.fbCate = vec;
			memcpy(&cid,hvect.data+t*hvect.size,4);
			memcpy(&score,hvect.data+t*hvect.size+4,4);
			pa->pubanalysisdata.objQuery.fbCate.push_back(make_pair(cid,score));
		}
        }
        pa->pubanalysisdata.objQuery.hasSet = true;

}

void CSearchKeyRanking::RankingSinglePub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{

        int sale = m_funcFrstInt(m_saleWeekProfile, me.id);
        int date = m_funcFrstInt(m_inputDateProfile, me.id);
        int evalue = m_funcFrstInt(m_TOTAL_REVIEW_COUNTProfile, me.id);
	int saleprice = m_funcFrstInt(m_SalePriceProfile,me.id);
	int pubdate = m_funcFrstInt(m_inputDateProfile,me.id);
        bool presale = m_funcFrstInt(m_pre_saleProfile, me.id) == 1 ? true : false;
        bool stock = m_funcFrstInt(m_stockProfile, me.id) > 0 ? true : false;
        bool imaged = m_funcFrstInt(m_num_imagesProfile, me.id) > 0 ? true : false;
        int pid =(int) m_funcFrstInt64(m_isPidProfile, me.id);
	//fprintf(stderr,"pid=%d\n",pid);
        bool isebook = (pid>=1900000000&&pid<2000000000)?true:false;
        if(isebook)
                stock = true;
	int match = -1;			//(1-����أ�0-һ����أ�-1�����)
        int fieldScr = 0;               //�ֶη���[0,9]
        int fieldFlag = 0;              //(4-���⣬3-ͬ�壬2-���꣬1-����)
        bool disFlag = 1;               //�ʾ����ڱ�ʶ
        bool withAutPubSearch = false;  //�����߳���������
        {
                //�ֶγ����ж�
                bool ifInT = false;             //�Ƿ�������
                bool ifInTitle = false;         //�Ƿ��ڱ���/ͬ�����
                bool ifInISBN = false;          //�Ƿ���isbn
                bool ifInMainTitle = false;     //�Ƿ��ڱ�������
                int titleSearchType = -1;       //������������(TITLE/TITLEEX/-1)
                int pos_title = INT_MAX;        //�ڱ������״γ���Ч��λ��
                int pos_title_ex = INT_MAX;     //ͬ������״γ���Ч��λ��
                {
                        bool ifPrimaryAppear = false;   //���������ֶγ���
			bool ifSubAppear = false;       //���⸱���ֶγ���
                        int cnt = 0;
                        int fid = -1, fieldNum;
			 map<int, int>::const_iterator mit;
                        //int ivtCount = (int)me.vFieldsOff.size();

                        {
                                fid = (int)me.vFieldsOff[cnt].field;
				mit = fid2fi.find(fid);
				if(mit != fid2fi.end())
                                {
					fieldNum = mit->second;
                                        if (fieldNum >= PMinNum)
                                        {       //����
                                                ifInT = true;
                                                if (fieldNum == TITLEID)
                                                {
                                                        ifInTitle = true;
                                                        pos_title = (int)me.vFieldsOff[cnt].off;
                                                }
                                                else if (fieldNum == TITLEEX)
                                                {
                                                        ifInTitle = true;
                                                        pos_title_ex =(int)me.vFieldsOff[cnt].off;
                                                }
                                                else if (fieldNum == AUTHOR || fieldNum == PUBNAME)
                                                {
                                                                withAutPubSearch = true;
                                                }
                                                else if (fieldNum == ISBN)
                                                {
                                                        ifInISBN = true;
                                                }
                                                else if (fieldNum == TITLEPRI)
                                                { //TITLE-Primary
                                                        ifPrimaryAppear = true;
                                                }
                                                else if (fieldNum == TITLESUB)
                                                {       //TITLE-Sub
                                                        ifSubAppear = true;
                                                }
                                        }
				 }
                        }

                        //����
                        if (true == ifInTitle)
                        {
                                titleSearchType = pos_title < INT_MAX ? TITLEID : TITLEEX;
                                if (ifPrimaryAppear || !ifSubAppear)
                                {       //����
                                        ifInMainTitle = true;
                                }
                        }
			//if(fieldNum > TMinNum)
				//fprintf(stderr,"fieldNum=%d\tifInMainTitle=%d\n",fieldNum,ifInMainTitle);
                }
                //�����ֶε÷ּ�����
                if (true == ifInT)
                { //����
                        fieldFlag = 1;
                        if (true == ifInISBN)
                        {       //isbn����
                                match = 1;
                                fieldScr = 9;
                        }
                        else if (pa->pubanalysisdata.objQuery.ifAutPubSearch > 0)
                        { //����/����������
                                if (true == withAutPubSearch)
                                {
                                        match = 1;
                                        fieldScr = 9;
                                        fieldFlag = 2;
                                }
                                else if (true == ifInTitle)
                                {
                                        if (ifInMainTitle)
                                        {       //����
                                                match = 1;
                                                fieldScr = 8;
					 }
                                        else
                                        {       //����
                                                match = 0;
                                                fieldScr = 5;
                                        }
                                        fieldFlag = titleSearchType == TITLEID ? 4 : 3;
                                }
                                else
                                {
                                        match = 0;
                                        fieldScr = 2;
                                }
                        }
                        else
                        {       //��������
                                if (true == ifInTitle)
                                {
                                        match = 1;
                                        fieldScr = ifInMainTitle ? 8 : 6;
                                        fieldFlag = titleSearchType == TITLEID ? 4 : 3;
                                        {       //��ȷƥ��
                                                int matchFieldID = titleSearchType == TITLEID ?
                                                        PTITLE_FID : TITLE_EX_FID;
                                                int posField = titleSearchType == TITLEID ?
                                                        pos_title : pos_title_ex;
                                                if (true == JudgeTotalMatch(pa->pubanalysisdata.objQuery.validString,
                                                                        me, matchFieldID, posField))
                                                {
                                                        fieldScr = 9;
                                                }
                                        }
                                }
                                else
                                {
                                        //if (vKeywords[0].keyWeight >= 7)
                                        if((int)me.vIdf[0] >= 7 && withAutPubSearch)
                                        {       //����Ϊδ��¼������/������(idf����)
						 match = 1;
                                                fieldScr = 8;
                                                fieldFlag = 2;
                                        }
                                        else
                                        {
                                                match = 1;
                                                fieldScr = 2;
                                        }
                                }
                        }
                }
                else
                {       //����
                        match = -1;
                        fieldScr = 1;
                }
                //���ȱ����Ʒ�͵���ض���Ʒ���
		if(!isebook)
		{
			if (match < 0)
			{       //��ضȵͣ���ҵ��ֵ��
				if (!stock || !imaged || (!sale && !evalue))
				//if (!stock || (!sale && !evalue))
				{
					return;
				}
			}
			else
			{       //��һ����ضȣ�����ҵ��ֵ̫��
				if (!stock && date >= 30 && !sale  && !evalue && !presale)
				{
					return;
				}
			}
			//��ͼ���������
			if (!imaged && !sale)
			//if (!sale)
			{
				if (match != 1)
				{ //����
					return;
				}
				else
				{       //��Ȩ
					fieldScr = 5;
					match = 0;
				}
			}
		}
        }
        //��ҵ��ֵ
        int commerScr = 0;      //��ҵ���صȼ�[0-9]
        int saleScr = 0;
        //����ϸ��Ȩ��
        bool newProduct = false;        //��Ʒ��ʶ
        int mixRecord = 0;                              //����Ƿ����������Ƿ���Ԥ�ۣ��Ƿ�Ϊ��Ʒ
        {
                //��Ʒ
                //if ( stock && imaged && date < 30
		if ( stock && imaged  &&date < 30
                                && saleprice > 0)
                {       //�л���ͼ�м۸��ϼܶ�
                        int PubDate = pubdate;
                        int CurDate = time(NULL);       //(since 1970)
                        if ( 0 == PubDate || (CurDate >= PubDate
                                                && (CurDate - PubDate) / 86400 < 240) )
                        {       //�ϼ�ʱ������ʱ�䲻����8����
                                newProduct = true;
                        }
                }
                //����
                ComputeCommerce(sale, evalue, newProduct, presale, commerScr, saleScr);
		//ComputeCommerce(stock,sale, evalue, newProduct, commerScr, saleScr);
                //��¼
                //mixRecord = (sale ? 0x4 : 0) + (presale ? 0x2 : 0) + (newProduct ? 0x1 : 0);
		mixRecord = (sale ? 0x4 : 0) +  (newProduct ? 0x1 : 0);
        }
        //����Ȩ��
        bool fbCateScr = 0;     //�������[0,1]
        int fbPidScr = 0;               //��������[0,2]
        int dropScr = 0;                //������ֶ�ƥ��?
        {
                if ( 0 == pa->pubanalysisdata.objQuery.ifAutPubSearch
			 && true == pa->pubanalysisdata.objQuery.ifHasFeedback)
                {       //�з���query(ֻ�ޱ�������)
                        if (match == 1 && (sale > 1)) // || presale))
                        {       //����أ���������Ԥ��
                                JudgePidHasQueryTag(
                                                me.id, pa, fbCateScr, fbPidScr);
                                if (0 == fbCateScr)
                                { //���ⲹ��
                                        if (newProduct && fieldScr > 7)
                                        {       //��������л���Ʒ
                                                fbPidScr = 1;
                                        }
                                }
                                else if (9 == fieldScr)
                                {               //����������Ʒ����������ƥ�䣬����������Ϊ�?
                                        dropScr = -1;
                                }
                        }
                }
        }
        //������Ϣ(Ϊ���ࡢ��Ʒ����)
        int cateLevel = 0;      //����ȼ�[1-4]
        {
                cateLevel = ComputeCateLevel(fbCateScr, fieldScr);
                int highRelSale = (match == 1) && (sale > 0) ? sale : 0;
                RecordCate(pa->pubanalysisdata.vCluster, pa->pubanalysisdata.highRelSaleCate,
                                me.id, cateLevel, highRelSale, pa->pubanalysisdata.objQuery.ifAutPubSearch);
        }
        //��������Լ���ؼ���
        bool baseRel = match >= 0;
        bool highRel = 1 == match;
        {
                if (highRel)
                {
                        ++pa->pubanalysisdata.RelDocCnt;
                }
                if (fieldScr >= 2)
                {
			++pa->pubanalysisdata.TDocCnt;
                        if (stock)
                        {
                                ++pa->pubanalysisdata.TDocCnt_stock;
                        }
                }
        }
        //querytag/field/commerce/
        rt.nScore = BaseRelFactor * baseRel                 //������ض�
                + Weight_Feedback * fbCateScr
                + Weight_Field * fieldScr
                + Weight_Commerce * commerScr
                + Weight_Feedback_Pid * fbPidScr
                + Weight_Sale * saleScr
                + Weight_Drop * dropScr;
        rt.nWeight = StockFactor * stock                     //���
                + BaseRelFactor * highRel       //����ر�ʶ
                + FieldIndicator * fieldFlag
                + DisIndicator * disFlag
                + NewIndicator * mixRecord              //��Ʒ��ʶ
                + ClusterFactor * cateLevel
                + FeedBackFactor * fbCateScr
                + PubFieldFactor * fieldScr
                + CommerceFactor * commerScr;
	/*if(rt.nScore>10000 && isebook)
	{
		fprintf(stderr,"pid=%d\n",pid);
		fprintf(stderr,"pid=%d\tnScore=%d\tnWeight=%d\n",rt.nDocId,rt.nScore,rt.nWeight);
		fprintf(stderr,"highRel=%d\tfieldFlag=%d\tdisFlag=%d\tmixRecord=%d\tcateLevel=%d\tfbCateScr=%d\tfieldScr=%d\tcommerScr=%d\n",highRel,fieldFlag,disFlag,mixRecord,cateLevel,fbCateScr,fieldScr,commerScr);
	}*/
	//printf("pid=%d\n",pid);
	//printf("pid=%d\tnScore=%d\tnWeight=%d\n",rt.nDocId,rt.nScore,rt.nWeight);
	//printf("highRel=%d\tfieldFlag=%d\tdisFlag=%d\tmixRecord=%d\tcateLevel=%d\tfbCateScr=%d\tfieldScr=%d\tcommerScr=%d\n",highRel,fieldFlag,disFlag,mixRecord,cateLevel,fbCateScr,fieldScr,commerScr);
        return;
}

void CSearchKeyRanking::RankingMultiPub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
        int count_Q = (int)me.vTerms.size();    //�ؼ�����


        int sale = m_funcFrstInt(m_saleWeekProfile, me.id);
        int date = m_funcFrstInt(m_inputDateProfile, me.id);
        int evalue = m_funcFrstInt(m_TOTAL_REVIEW_COUNTProfile, me.id);
	 int saleprice = m_funcFrstInt(m_SalePriceProfile,me.id);
        int pubdate = m_funcFrstInt(m_inputDateProfile,me.id);
        bool presale = m_funcFrstInt(m_pre_saleProfile, me.id) == 1 ? true : false;
        bool stock = m_funcFrstInt(m_stockProfile, me.id) > 0 ? true : false;
        bool imaged = m_funcFrstInt(m_num_imagesProfile, me.id) > 0 ? true : false;
        int pid = m_funcFrstInt64(m_isPidProfile, me.id);
	//fprintf(stderr,"pid=%d\n",pid);
        bool isebook = (pid>=1900000000&&pid<2000000000)?true:false;

        if(isebook)
                stock = true;


        //�����
        int match = -1;                 //(1-����أ�0-һ����أ�-1�����)
        int fieldScr = 0;               //�ֶη���[0,9]
        int fieldFlag = 0;      //(4-���⣬3-ͬ�壬2-���꣬1-����)
        bool disFlag = 0;               //�ʾ����ڱ�ʶ
        int weight_T = 0;               //�ؼ���Ȩ�غ�(����)
	int cnt_TT = 0,count_QQ=0;
        {
                //map<int, int> &fid2fi = fid2fi;

                //�ֶγ����ж�
                bool ifAllInT = false;                          //�Ƿ�ȫ������
                bool ifAllInTitle = false;                      //�Ƿ�ȫ�ڱ���/ͬ�����
		bool ifAllInSubtitle = false;                   //�Ƿ�ȫ�ڸ�����
                bool ifInMainTitle = false;                     //�Ƿ��ڱ�������
                bool withAutPubSearch = false;          //�����߳���������
                int titleSearchType = -1;                       //������������(TITLE/TITLEEX/-1)
                int weight_Q = 0;                               //�ؼ���Ȩ�غ�(query)
                int length_T = 0;                               //��Ч�ؼ����ڱ����ֳ�
                int pos_title = INT_MAX;                        //�ڱ������״γ���Ч��λ��
                int pos_title_ex = INT_MAX;                             //ͬ������״γ���Ч��λ��
                int m1,m2;
                {
                        bool ifPrimaryAppear = false;   //���������ֶγ���
                        bool ifSubAppear = false;       //���⸱���ֶγ���
                        //PosInvert* ivtAddr;
                        int ivtCnt, cnt, fid, fieldNum, weight, length;
                        bool ifAdd_T, ifAddWeight;                      //per keyword
                        int cnt_T = 0, cnt_ti = 0, cnt_ti_ex = 0, cnt_st = 0;
                        map<int, int>::const_iterator mit;

                        for (int k = 0; k < count_Q; ++k)
                        {
                                weight = (int)me.vIdf[k];
                                length = me.vTerms[k].len;
                                weight_Q += weight;
                                ifAdd_T = ifAddWeight = false;
                                fid = -1;

                                {

                                        //new field
                                        fid = (int)me.vFieldsOff[k].field;
                                        mit = fid2fi.find(fid);
                                        if (mit != fid2fi.end())
                                        {
                                                fieldNum = mit->second;
						//fprintf(stderr,"fieldNum=%d, fid = %d,%s\n",fieldNum,fid,m_vFieldInfo[fid].strFieldName.c_str() );
						//fprintf(stderr,"fieldNum=%d\tTMinNum=%d\n",fieldNum,PMinNum);
                                                if (fieldNum >= PMinNum)
                                                {       //����
                                                        if (false == ifAdd_T)
                                                        {
								ifAdd_T = true;
                                                                ++cnt_T;
                                                        }
							//fprintf(stderr,"cnt_T=%d\n",cnt_T);
                                                        if (fieldNum == TITLEID)
                                                        {       //����
                                                                ++cnt_ti;
                                                                if (weight)
                                                                {
                                                                        if (false == ifAddWeight)
                                                                        {
                                                                                weight_T += weight;
                                                                                length_T += length;
                                                                                ifAddWeight = true;
                                                                        }
                                                                        if ((int)me.vFieldsOff[k].off < pos_title)
                                                                        {
                                                                                pos_title = (int)me.vFieldsOff[k].off;
                                                                        }
                                                                }
                                                                m1 = me.vFieldLen[k];
                                                        }
                                                        else if (fieldNum == TITLEEX)
                                                        { //ͬ��
                                                                ++cnt_ti_ex;
                                                                if (weight)
                                                                {
                                                                        if (false == ifAddWeight)
                                                                        {
                                                                                weight_T += weight;
                                                                                length_T += length;
                                                                                ifAddWeight = true;
                                                                        }
                                                                        if ((int)me.vFieldsOff[k].off < pos_title_ex)
                                                                        {
                                                                                pos_title_ex = (int)me.vFieldsOff[k].off;
                                                                        }
                                                                }
                                                                m2 = me.vFieldLen[k];
							}
                                                        else if (fieldNum == AUTHOR || fieldNum == PUBNAME)
                                                        {       //����/������
                                                                ++cnt_st;
                                                                //if (vKeywords[k].type)
                                                                if(pa->pubanalysisdata.objQuery.keysType[k])
                                                                {
                                                                        withAutPubSearch = true;
                                                                        if (weight && false == ifAddWeight)
                                                                        {
                                                                                weight_T += weight;
                                                                                ifAddWeight = true;
                                                                        }
                                                                }
                                                        }
                                                        else if (fieldNum == ISBN)
                                                        {       //ISBN
                                                                ++cnt_st;
                                                                if (weight && false == ifAddWeight)
                                                                {
                                                                        weight_T += weight;
                                                                        ifAddWeight = true;
                                                                }
                                                        }
                                                        else if (fieldNum == TITLEPRI)
                                                        { //TITLE-Primary
                                                                ifPrimaryAppear = true;
                                                        }
                                                        else if (fieldNum == TITLESUB)
                                                        {       //TITLE-Sub
                                                                ifSubAppear = true;
                                                        }
                                                }
                                        }
                                }
                        }
			cnt_TT = cnt_T;
			count_QQ = count_Q;
                        //����
			//fprintf(stderr,"cnt_T=%d\tcnt_ti=%d\tcnt_ti_ex=%d\tcount_Q=%d\tcnt_st=%d\n",cnt_T,cnt_ti,cnt_ti_ex,count_Q,cnt_st);
			//fprintf(stderr,"cnt_TT=%d\tcount_QQ=%d\n",cnt_TT,count_QQ);
			if (cnt_T == count_Q)
                        {       //ȫ������
                                ifAllInT = true;
                                fieldFlag = 1;
                        }
                        if (cnt_ti || cnt_ti_ex)
                        {       //�����д�
                                if (ifPrimaryAppear || !ifSubAppear)
                                {       //����
                                        ifInMainTitle = true;
                                }

                                titleSearchType = cnt_ti >= cnt_ti_ex ?
                                        TITLEID : TITLEEX;

                                if (titleSearchType == TITLEID)
                                {       //����
                                        if (cnt_ti == count_Q)
                                        {       //ȫ�ڱ���
                                                ifAllInTitle = true;
                                                fieldFlag = 4;
                                        }
                                }
                                else
                                {       //ͬ��
                                        if (cnt_ti_ex == count_Q)
                                        { //ȫ�ڱ���
                                                ifAllInTitle = true;
                                                fieldFlag = 3;
                                        }
                                }
                        }
                        if (1 == fieldFlag && cnt_st == count_Q)
                        {       //ȫ�ڸ���
                                ifAllInSubtitle = true;
                                fieldFlag = 2;
                        }
                }
		//printf("�����ֶη���\n");
		//�����ֶη���
                static const size_t LongQueryMinLen = 10;       //��query����
                static const size_t HasSubStrMaxLen = 16;       //������׼ƥ��query�����
                if (cnt_TT == count_QQ)
                {       //ȫ������
                        if (true == ifAllInTitle)
                        {       //ȫ�ڱ���/ͬ��
                                int matchFieldID = titleSearchType == TITLEID ?
                                        PTITLE_FID : TITLE_EX_FID;
                                if ( (pa->pubanalysisdata.objQuery.validString.size() >= LongQueryMinLen
                                                && !pa->pubanalysisdata.objQuery.ifAllSingleWord)
                                                || true == JudgeDis(pa,me, matchFieldID) )
                                { //����ʾ�����
                                        match = 1;
                                        fieldScr = ifInMainTitle ? 8 : 6;
                                        disFlag = 1;
                                        if (ifInMainTitle &&
                                                        pa->pubanalysisdata.objQuery.validString.size() <= HasSubStrMaxLen)
                                        {
                                                int posField = titleSearchType == TITLEID ?
                                                        pos_title : pos_title_ex;
                                                if (true == JudgeTotalMatch(
                                                                        pa->pubanalysisdata.objQuery.validString,
                                                                        me, matchFieldID, posField))
                                                { //���⾫׼ƥ�俪��
                                                        fieldScr = 9;
                                                }
                                        }
                                }
                                else
                                {       //������ʾ�����
                                        if (ifInMainTitle)
                                        {       //������
                                                if (true == pa->pubanalysisdata.objQuery.ifHasFeedback)
                                                {       //�з���query(ֻ�ޱ�������)
                                                        match = 1;
                                                        fieldScr = 7;
						 }
                                                else
                                                {
                                                        int dis = ComputeDis(me, matchFieldID);
                                                        if ( dis <= 10)
                                                        {       //�ʾ��С
                                                                match = 1;
                                                                fieldScr = 7;
                                                        }
                                                        else
                                                        {
                                                                match = 0;
                                                                fieldScr = 5;
                                                        }
                                                }
                                        }
                                        else
                                        {       //�ڴ���
                                                match = 0;
                                                fieldScr = 4;
                                        }
                                }
                        }
                        else if ( true == withAutPubSearch
                                        || (weight_T > 0 && weight_Q == weight_T) )
                        {       //������/������/ISBN���������н������޶���
                                match = 1;
                                if (titleSearchType != -1)
                                {       //�����д�
                                        fieldScr = ifInMainTitle ? 8 : 6;
                                        if (ifInMainTitle &&
                                                        pa->pubanalysisdata.objQuery.validString.size() <= HasSubStrMaxLen)
                                        {
                                                int matchFieldID = titleSearchType == TITLEID ?
                                                        PTITLE_FID : TITLE_EX_FID;
                                                int flen;
                                                if(matchFieldID == PTITLE_FID)
                                                        flen = m1;
						else
                                                        flen = m2;
                                                int posField = titleSearchType == TITLEID ?
                                                        pos_title : pos_title_ex;
                                                if (posField <= 2 && (abs(length_T - flen) <= 2))
                                                {//���⾫׼ƥ�俪��
                                                        fieldScr = 9;
                                                }
                                        }
                                }
                                else
                                { //����/������/ISBN�������
                                        fieldScr = 9;
                                }
                        }
                        else
                        {       //�г��������ߡ���������
                                if (true == ifAllInSubtitle)
                                {       //ȫ������������/������/ISBN��
                                        if ( (pa->pubanalysisdata.objQuery.validString.size() >= LongQueryMinLen
                                                                && !pa->pubanalysisdata.objQuery.ifAllSingleWord)
                                                        || true == JudgeDis(pa,me, -2) )    //������
                                        { //����ʾ�����(����Ϊ������/�������)
                                                match = 1;
                                                fieldScr = 5;
                                                disFlag = 1;
                                        }
                                        else
                                        {       //������
                                                match = 1;
                                                fieldScr = 2;
                                        }
                                }
                                else if (true == ifInMainTitle && weight_T > 0 &&
                                                ((weight_T << 1) >= weight_Q) )
                                {       //��������������ҪȨ�ش�
                                        match = 0;
                                        fieldScr = weight_T == weight_Q ? 5 : 4;
				}
                                else
                                {       //���ⲻ������ҪȨ�ش�
                                        match = 0;
                                        fieldScr = 2;
                                }
                        }
                }
                else if(0< cnt_TT < count_QQ)
                { //��ȫ������
                        if ( pa->pubanalysisdata.objQuery.validString.size() <= 6
                                        || pa->pubanalysisdata.objQuery.ifAllSingleWord)
                        { //��������(���̴�/ȫ���ִ�)
                                if (false == JudgeDis(pa,me))
                                {       //�ϸ�ʾ����
                                        if(!isebook)
                                                return;
                                }
                                if ( true == ifInMainTitle && (stock || evalue)
                                                && weight_T > 0 && ((weight_T << 1) >= weight_Q) )
                                {
                                        match = 0;
                                        fieldScr = weight_T == weight_Q ? 5 : 4;
                                }
                                else
                                {
                                        match = -1;
                                        fieldScr = 1;
                                }
                        }
                        else
                        {       //һ������
                                if ( true == ifInMainTitle
                                                && weight_T > 0 && ((weight_T << 1) >= weight_Q) )
                                {
                                        match = 0;
                                        fieldScr = weight_T == weight_Q ? 5 : 4;
                                }
				else if (true == JudgeDisFilter(me))
                                {
                                        match = -1;
                                        fieldScr = 1;
                                }
                                else
                                {       //�ʾ����
                                        if(!isebook)
                                        {
                                                return;
                                        }
                                        else
                                        {
                                                match = -1;
                                                //fieldScr = weight_T == weight_Q ? 5 : 4;
                                                fieldScr = 1;
                                        }
                                }
                        }
                }
		else
		{
			return;
		}
                //���ȱ����Ʒ�͵���ض���Ʒ���
                if(!isebook)
                {
                        if (match < 0)
                        {       //��ضȵͣ���ҵ��ֵ��
                                if (!stock || !imaged || (!sale && !evalue))
				//if (!stock || (!sale && !evalue))
                                {
                                        return;
                                }
                        }
                        else
                        {       //��һ����ضȣ�����ҵ��ֵ̫��
                                if (!stock && date >= 30 && !sale && !evalue  && !presale)
                                {
                                        return;
                                }
                        }
			//��ͼ���������
                        if (!imaged && !sale)
			//if(!sale)
                        {
                                if (match != 1)
                                { //����
                                        return;
                                }
                                else
                                {       //��Ȩ
                                        fieldScr = 5;
                                        match = 0;
                                }
                        }
                }
        }
	//printf("��ҵ��ֵ\n");
        //��ҵ��ֵ
        int commerScr = 0;                              //��ҵ���صȼ�[0-9]
        int saleScr = 0;                                        //����ϸ��Ȩ��
        bool newProduct = false;        //��Ʒ��ʶ
        int mixRecord = 0;                              //����Ƿ����������Ƿ���Ԥ�ۣ��Ƿ�Ϊ��Ʒ
        {
                //��Ʒ
                //if ( stock && imaged && date < 30
		if ( stock && imaged  && date < 30
                                && saleprice > 0)
                {       //�л���ͼ�м۸��ϼܶ�
                        int PubDate = pubdate;
                        int CurDate = time(NULL);       //(since 1970)
                        if ( 0 == PubDate || (CurDate >= PubDate
                                                && (CurDate - PubDate) / 86400 < 240) )
                        {       //�ϼ�ʱ������ʱ�䲻����8����
                                newProduct = true;
                        }
                }

                //����
                //ComputeCommerce(stock, evalue, newProduct, presale, commerScr, saleScr);
		ComputeCommerce(stock,sale, evalue, newProduct, commerScr, saleScr);
		//��¼
                //mixRecord = (sale ? 0x4 : 0) + (presale ? 0x2 : 0) + (newProduct ? 0x1 : 0);
		mixRecord = (sale ? 0x4 : 0) +  (newProduct ? 0x1 : 0);
        }
	//printf("����Ȩ��\n");
        //����Ȩ��
        bool fbCateScr = 0;     //�������
        int fbPidScr = 0;               //��������
        int dropScr = 0;                //����
        {
                if (true == pa->pubanalysisdata.objQuery.ifHasFeedback)
                {       //�з���query(ֻ�ޱ�������)
                        if (match == 1 && (sale > 1)) // || presale))
                        {       //������أ���������Ԥ��
                                JudgePidHasQueryTag(
                                                me.id, pa, fbCateScr, fbPidScr);

                                if (0 == fbCateScr)
                                { //���ⲹ��
                                        if (newProduct && fieldScr > 7)
                                        {       //��������л���Ʒ
                                                fbPidScr = 1;
                                        }
                                }
                                else if (9 == fieldScr)
                                {       //����������Ʒ���������ƥ�䣬����������Ϊ��
                                        dropScr = -1;
                                }
                        }
                }
        }

        //������Ϣ
	//printf("������Ϣ\n");
        int cateLevel = 0;      //����ȼ�[1-4]
        {
                cateLevel = ComputeCateLevel(fbCateScr, fieldScr);
		//fprintf(stderr,"cateLevel=%d\tfbCateScr=%d\tfieldScr=%d\n",cateLevel,fbCateScr,fieldScr);
                int highRelSale = (match == 1) && (sale > 0) ? sale : 0;
                RecordCate(pa->pubanalysisdata.vCluster, pa->pubanalysisdata.highRelSaleCate,
                                me.id, cateLevel, highRelSale, false);
	}
	//printf("��������Լ���ؼ���\n");
        //��������Լ���ؼ���
        bool baseRel = match >= 0;
        bool highRel = 1 == match;
        {
                if (highRel)
                {
                        ++pa->pubanalysisdata.RelDocCnt;
                }
                if (fieldScr >= 2)
                {
                        ++pa->pubanalysisdata.TDocCnt;
                        if (stock)
                        {
                                ++pa->pubanalysisdata.TDocCnt_stock;
                        }
                }
        }

        //querytag/field/commerce/
        rt.nScore = BaseRelFactor * baseRel          //������ض�
                + Weight_Feedback * fbCateScr
                + Weight_Field * fieldScr
                + Weight_Commerce * commerScr
                + Weight_Feedback_Pid * fbPidScr
                + Weight_T_AREA * weight_T
                + Weight_Sale * saleScr
                + Weight_Drop * dropScr;

       rt.nWeight = StockFactor * stock                     //���
                + BaseRelFactor * highRel       //����ر�ʶ
                + FieldIndicator * fieldFlag
                + DisIndicator * disFlag
                + NewIndicator * mixRecord              //��Ʒ��ʶ
                + ClusterFactor * cateLevel
                + FeedBackFactor * fbCateScr
                + PubFieldFactor * fieldScr
		+ CommerceFactor * commerScr;
	//if(rt.nScore>10000)
       /*{
	       fprintf(stderr,"pid=%d\n",pid);
	       fprintf(stderr,"pid=%d\tnScore=%d\tnWeight=%d\n",rt.nDocId,rt.nScore,rt.nWeight);
	       fprintf(stderr,"highRel=%d\tfieldFlag=%d\tdisFlag=%d\tmixRecord=%d\tcateLevel=%d\tfbCateScr=%d\tfieldScr=%d\tcommerScr=%d\n",highRel,fieldFlag,disFlag,mixRecord,cateLevel,fbCateScr,fieldScr,commerScr);
       }*/
       //printf("pid=%d\n",pid);
       //printf("pid=%d\tnScore=%d\tnWeight=%d\n",rt.nDocId,rt.nScore,rt.nWeight);
       //printf("highRel=%d\tfieldFlag=%d\tdisFlag=%d\tmixRecord=%d\tcateLevel=%d\tfbCateScr=%d\tfieldScr=%d\tcommerScr=%d\n",highRel,fieldFlag,disFlag,mixRecord,cateLevel,fbCateScr,fieldScr,commerScr);
        return;

}
void CSearchKeyRanking::ReRankingPub(vector<SResult>& vRes, CDDAnalysisData* pa)
{
	//����������(δ����δ����)
	if (vRes.empty())
	{
		return;
	};
	static const int CateWeight = Weight_Cate;
	static const int FieldWeight = Weight_Field;
	static const int CommerceWeight = Weight_Commerce;
	static const int FeedbackWeight = Weight_Feedback;
	static const int PerfectMatch = 9;      //��ȷƥ��
	static const int MainHighMatch = 8;     //�������
	static const int AuthorSearch = AUTHOR;
	static const int PubSearch = PUBNAME;
	//Fst, ��������
	/*std::sort(
	  vRes.begin(), vRes.end(),
	  boost::bind(
	  std::greater<int>(),
	  boost::bind(&SResult::nScore, _1),
	  boost::bind(&SResult::nScore, _2)));*/

	//Snd, ����,�Ƽ�,����
	int AutPubSearch = pa->pubanalysisdata.objQuery.ifAutPubSearch > 0
		&& 1 == pa->pubanalysisdata.objQuery.keysType.size() ?
		pa->pubanalysisdata.objQuery.ifAutPubSearch : 0; //������/����������
	bool ifLongQuery = pa->pubanalysisdata.objQuery.validString.size() > 16 ? true : false;
	if ( NULL != pa
			&& false == pa->pubanalysisdata.objQuery.ifLimitSearch)
	{       //�Ǹ߼�����(�߼��������Ƽ����޾��࣬�޾���)
		/*if (pa->pubanalysisdata.TDocCnt <= 0)
		{       //����ؽ��
			vRes.clear();
			return;
		}*/

		//����
		set<u64> mInCluster;              //�������
		map<u64, bool> mExtCluster; //��չ��(��mInCluster�޽���)
		int InClusterLevel = 0; //�ھ����е���ȼ�(4-����/3-�������/2-�ϸ����/1-�����)
		bool ifHasDirOrBig = false;     //�Ƿ���ֱ�������������Ƽ����
		int fairResultCnt = 0;
		{
			//�����ռ�
			if (false == pa->pubanalysisdata.vCluster.empty())
			{
				if (false == pa->pubanalysisdata.vCluster[0].empty())
				{ //��������ȼ�
					mInCluster.insert(pa->pubanalysisdata.vCluster[0].begin(),                                                                                                  pa->pubanalysisdata.vCluster[0].end());
				}
				if (false == pa->pubanalysisdata.objQuery.fbCate.empty())
				{       //query�������(���ٻ�)
					const vector<pair<int, int> >& cate =
						pa->pubanalysisdata.objQuery.fbCate;
					for (size_t c = 0; c < cate.size(); ++c)
					{
						mInCluster.insert(cate[c].first);
					}
				}
				if (false == mInCluster.empty())
				{
					InClusterLevel = 4;     //��������
				}
				else
				{       //�ı��ȼ�(ֻȡ���һ��)
					for (size_t v = 1; v < pa->pubanalysisdata.vCluster.size(); ++v)
					{
						if (false == pa->pubanalysisdata.vCluster[v].empty())
						{
							mInCluster.insert(pa->pubanalysisdata.vCluster[v].begin(),                                                                                          pa->pubanalysisdata.vCluster[v].end());
							InClusterLevel = 4 - v;
							break;
						}
					}
				}
				//��չ������(ֻ������������չ)
				if ( InClusterLevel > 1
						&& cid2cids.size() != 0)
				{       //��չ��
					int catid,catid2;
					for (set<u64>::const_iterator iter = mInCluster.begin();
							iter != mInCluster.end(); ++iter)
					{
						HASHVECTOR ext;
						ext = cid2cids[*iter];	
						if(ext.count != 0)
						{       //������
							for (size_t v = 0; v < ext.count; ++v)
							{
								memcpy(&catid,ext.data+v*ext.size,4);
								if ( mInCluster.find(catid) == mInCluster.end()
										&& (mExtCluster.find(catid) == mExtCluster.end()
											|| 0 == mExtCluster[catid]) )
								{ //��/С����
									memcpy(&catid2,ext.data+v*ext.size+4,4);
									mExtCluster[catid] = catid2;	
								}
							}
						}
					}
				}
			}
			//�������
			if (InClusterLevel && false == mInCluster.empty())
			{
				//�������÷�
				hash_map<int, vector<int> >::const_iterator it;
				int cateLevel, relLevel, cateScr, cateThird;

				for (size_t i = 0; i < vRes.size(); ++i)
				{
					if (0 == vRes[i].nScore)
					{
						continue;
					}
					if (vRes[i].nWeight/TypeIndicator != 0)
					{
						continue;
					}

					int cateScr = 0;
					int cateType = 0;       //4-����/3-ֱ��/2-������/1-С����
					cateLevel = vRes[i].nWeight / ClusterFactor % 10;

					if (cateLevel >= InClusterLevel)
					{       //���������
						cateScr = 1;
						cateType = 4;
						++fairResultCnt;
					}
					else
					{ //�鿴����
						u64 cls = (u64)m_funcFrstInt64(m_clsProfile,vRes[i].nDocId);
						if (cls != 0)
						{ //�����
							bool ifCluster = false;
							//const vector<int> &cate = it->second;

							//ֱ���ж�
							if (mInCluster.find(cls) != mInCluster.end())
							{
								cateScr = 1;
								cateType = 3;
								ifCluster = true;
								ifHasDirOrBig = true;
								++fairResultCnt;
							}

							//��չ���ж�
							if (false == ifCluster)
							{
								if ( false == mExtCluster.empty()
										&& vRes[i].nWeight / StockFactor % 10
										&& mExtCluster.find(cls) != mExtCluster.end())
								{ //�л���Ʒ
									if (1 == mExtCluster[cls])
									{ //������
										cateType = 2;
										ifHasDirOrBig = true;
										++fairResultCnt;
									}

									else
									{ //С����
										cateType = 1;
									}
									cateScr = 1;
									ifCluster = true;
								}

								if ( false == ifCluster
										&& 0 == vRes[i].nWeight / BaseRelFactor % 10)
								{       //���ˣ��Ǹ����)
									//fprintf(stderr,"process pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
									vRes[i].nScore = 0;
									continue;

								}
								/*else if (vRes[i].nWeight / PubCommerceFactor % 10 <= 0)
								{       //����ص�����ҵ��ֵ
									vRes[i].nScore -= 3 * FieldWeight;
								}*/
							}
						}
						else if (0 == vRes[i].nWeight / BaseRelFactor % 10)
						{       //���ˣ��Ǹ����)
							//fprintf(stderr,"process pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
							vRes[i].nScore = 0;
							continue;

						}
						/*else if (vRes[i].nWeight / PubCommerceFactor % 10 <= 0)
						{       //����ص�����ҵ��ֵ
							vRes[i].nScore -= 3 * FieldWeight;
						}*/
					}
					//fprintf(stderr,"cateType=%d\n",cateType);
					if ((1 == cateType) || (4 == cateType &&
								vRes[i].nWeight / FeedBackFactor % 10 > 0))
					{       //����С���ࡢ��������÷�
						vRes[i].nScore += cateScr * CateWeight / 2;
					}
					else
					{
						vRes[i].nScore += cateScr * CateWeight;
					}
					vRes[i].nWeight = vRes[i].nWeight /
						(ClusterFactor * 10) * (ClusterFactor * 10)
						+ cateType * ClusterFactor
						+ vRes[i].nWeight % ClusterFactor;

				}
			}
			else
			{       //�޾�����Ϣ�����з���ؽ����0
				for (size_t i = 0; i < vRes.size(); ++i)
				{
					if (vRes[i].nWeight/TypeIndicator != 0)
                                        {
                                                continue;
                                        }
					if (0 == vRes[i].nScore / BaseRelFactor % 10)
					{
						//fprintf(stderr,"process pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
						vRes[i].nScore = 0;
					}
				}
			}
		}
		/*for(size_t i = 0; i < vRes.size();i++)
		{
			printf("before pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
		}*/
		//������������
		if (ifHasDirOrBig)
		{ //����С�����ҵ���ؽ��
			if (fairResultCnt > HighRelBound)
			{
				vRes.erase(std::remove_if(vRes.begin(), vRes.end(), filter_small_ext_all),vRes.end());
			}
			else
			{
				vRes.erase(std::remove_if(vRes.begin(), vRes.end(), filter_small_ext),vRes.end());
				//С������ҵ����
				for (size_t i = 0; i < vRes.size(); ++i)
				{
					if (vRes[i].nWeight/TypeIndicator != 0)
                                        {
                                                continue;
                                        }
					if (1 == vRes[i].nWeight / ClusterFactor % 10)
					{
						vRes[i].nScore -= 5 * CommerceWeight;
					}
				}
			}
		}
		else
		{ //������weight
			vRes.erase(std::remove_if(vRes.begin(), vRes.end(), filter_zero_weight),vRes.end());
		}
		sort(vRes.begin(), vRes.end());
		/*for(size_t i = 0; i < vRes.size();i++)
		{
			printf("here after process pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
		}*/
	}
	//Thd, ���,��Ʒ����
	if (pa->pubanalysisdata.TDocCnt_stock <= 0 && pa->pubanalysisdata.TDocCnt > 0)
	{       //�����л������ضȵ��Ҵ�����һ����ضȵ�ȱ��������������ȱ���������
		static const int NoStockMax = 2;        //�����޻���Ʒ��top���ֵĸ���
		multimap<int, int, greater<int> > weight2idx;
		for (size_t i = 0; i < vRes.size(); ++i)
		{
			if (vRes[i].nWeight/TypeIndicator != 0)
			{
				continue;
			}
			if (0 == vRes[i].nWeight / StockFactor % 10)
			{       //ȱ��
				if (vRes[i].nWeight / FieldFactor % 10 >= MainHighMatch)
				{
					weight2idx.insert(make_pair(vRes[i].nScore, i));
				}
			}
			else
			{       //�л�
				vRes[i].nScore += 1 * StockFactor;
			}
		}
		multimap<int, int, greater<int> >::const_iterator iter = weight2idx.begin();
		int count = 0;
		for (; count < NoStockMax && iter != weight2idx.end(); ++count, ++iter)
		{
			vRes[iter->second].nScore += 1 * StockFactor; //α����
		}
	}
	else
	{
		static const size_t NoStockMax = 2;     //�����޻���Ʒ��top���ֵĸ���
		static const size_t TopPosMin = 4;      //�޻���Ʒ��߳���λ��
		size_t noStockCount = 0;
		size_t hasStockCount = 0;
		int stockWeight, baseRel, clusterScr, fieldScr, mixScr;
		vector<pair<int, int> > tmp;
		for (size_t i = 0; i < vRes.size(); ++i)
		{

			if (vRes[i].nWeight/TypeIndicator != 0)
			{
				continue;
			}
			
			fieldScr = vRes[i].nWeight / FieldFactor % 10;
			mixScr = vRes[i].nWeight / NewIndicator % 10;
			if (0 == vRes[i].nWeight / StockFactor % 10)
			{       //ȱ��
				if ( (mixScr & 4)
						&& (PerfectMatch == fieldScr || (MainHighMatch == fieldScr && ifLongQuery))
						&& (noStockCount < NoStockMax) )
				{       //����ȱ��λ���о�ȷƥ��������
					if (i <= TopPosMin || !tmp.empty())
					{       //λ��̫��ǰ
						tmp.push_back(pair<int, int>(i, vRes[i].nScore));
					}
					else
					{       //λ������
						vRes[i].nScore += 1 * StockFactor;    //α����
						if (i && vRes[i].nScore == vRes[i-1].nScore)
						{
							--vRes[i].nScore;
						}
					}
					++noStockCount;
				}
			}
			else
			{       //�л�
				++hasStockCount;
				vRes[i].nScore += 1 * StockFactor;

				if (!tmp.empty() && hasStockCount > TopPosMin)
				{       //�о�ȷƥ�������
					int tmpWeight = vRes[i].nScore;
					for (size_t t = 0; t < tmp.size(); ++t)
					{
						vRes[tmp[t].first].nScore = --tmpWeight;
					}
					tmp.clear();
				}

				//��Ʒ
				if ((mixScr & 1) && (fieldScr >= MainHighMatch))
				{ //�������
					//hash_map<int, vector<int> >::const_iterator it =m_joinMap.find(vDocs[i].pid);
					//if (it != m_joinMap.end())
					size_t nCnt = 0;
					u64* ptr = (u64*)m_funcValPtr(m_clsProfile, vRes[i].nDocId, nCnt);
					{ //�����
						//const vector<int> &cate = it->second;
						vector<u64> cate;
						for(size_t t = 0; t < nCnt;t++)
						{
							cate.push_back(ptr[t]);
						}
						int simCateIdx = -1;

						u64 catefmt = AutPubSearch ? (cate[0]>>16) : (cate[0]>>8);
						if ( pa->pubanalysisdata.highRelSaleCate.find(catefmt)
								!= pa->pubanalysisdata.highRelSaleCate.end() &&
								pa->pubanalysisdata.highRelSaleCate[catefmt].size() >= 6)
						{       //���㹻���Ƽ�
							simCateIdx = 0;
						}


						if (simCateIdx >= 0)
						{       //�ҵ��㹻���Ƽ�
							int catefmt = AutPubSearch ?
								(cate[simCateIdx] >> 16) : (cate[simCateIdx] >> 8);
							vector<u64>& highSale =
								pa->pubanalysisdata.highRelSaleCate[catefmt];
							sort(highSale.begin(), highSale.end(), greater<int>());
							highSale.resize(10, 0);

							//����������Ϣ
							bool ifInHotCate = hotNPCate.find(cate[simCateIdx])!= hotNPCate.end() ? true : false;
							bool ifPreSaleOrSale = (mixScr & 2) || (mixScr & 4);
							//bool ifHasSaleDay = refRscSharedModule->m_refDocReader->
							//GetDataFieldValue<int>(vDocs[i].docID, "sale_day") > 0 ? true : false;
							bool ifHasSaleDay = m_funcFrstInt(m_sale_dayProfile,vRes[i].nDocId);
							int PredictSale = 0;
							if (ifPreSaleOrSale)
							{
								size_t idx = 5; //mid
								if (ifInHotCate)
								{
									idx -= 1;
								}
								if (ifHasSaleDay)
								{
									idx -= 1;
								}
								PredictSale = highSale[idx];
							}
							else
							{

								int date = m_funcFrstInt(m_TOTAL_REVIEW_COUNTProfile,vRes[i].nDocId);
								switch (date / 7)
								{
									case 0:
										PredictSale = ifInHotCate ? highSale[4] : highSale[5];
										break;
									case 1 :
										PredictSale = (ifInHotCate ? highSale[4] : highSale[5]) / 2 + 1;
										if (PredictSale < highSale[9])
										{
											PredictSale = highSale[9];
										}
										break;
									case 2:
										if (ifInHotCate)
										{
											PredictSale = highSale[5] / 5 + 1;
											if (PredictSale > highSale[9])
											{
												PredictSale = highSale[9];
											}
										}
										break;
								}
							}

							int newCommerceScr = getSaleScr(PredictSale);
							if (newCommerceScr > 0)
							{       //��ֵ
								if (newCommerceScr == 9)
								{       //����̫��
									newCommerceScr = 8;
								}
								int oldCommerceScr = vRes[i].nWeight / PubCommerceFactor % 10;
								if (newCommerceScr > oldCommerceScr)
								{
									vRes[i].nWeight = vRes[i].nWeight / PubCommerceFactor / 10 * PubCommerceFactor * 10
										+ newCommerceScr * PubCommerceFactor
										+ vRes[i].nScore % PubCommerceFactor;
									vRes[i].nWeight = vRes[i].nWeight / NewIndicator / 10 * NewIndicator * 10
										+ 9 * NewIndicator
										+ vRes[i].nWeight % NewIndicator;
									vRes[i].nScore = vRes[i].nScore - oldCommerceScr * CommerceWeight
										+ newCommerceScr * CommerceWeight
										+ (ifPreSaleOrSale ? oldCommerceScr : 0);
								}
							}
						}
					}
				}
			}
		}
	}
	/*for(size_t i = 0; i < vRes.size();i++)
	{
		printf("after pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
	}*/

}



void CSearchKeyRanking::SortPub(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa)
{
	// do somethine more 
	SortRange(vRes ,from, to);
	int out_size = min((int)vRes.size(), 50);
#ifdef DEBUG
	for (int i=0; i<out_size; i++) {
		int docID = vRes[i].nDocId;
		int date = m_funcFrstInt(m_inputDateProfile, docID);
		int pid = m_funcFrstInt64(m_isPidProfile, docID);
		printf("debug_weight=%d  rank_score=%d  pid=%d  date=%d\n", vRes[i].nWeight, vRes[i].nScore, pid, date);
	}
#endif
}
