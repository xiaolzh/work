#include "SearchKeyRanking.h"
#include <boost/bind.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <math.h>
//#include "QuerySegment.h"

static inline bool filter_small_ext(const SResult& di)	
{
	return (di.nScore <= 0) ||
		(1 == di.nWeight / CSearchKeyRanking::ClusterFactor % 10 //小扩类
		 && 1 != di.nWeight / CSearchKeyRanking::BaseRelFactor % 10 //低相关
		 && di.nWeight / CSearchKeyRanking::PubCommerceFactor < 6);
	//商业价值低	
}

static inline bool filter_small_ext_all(const SResult& di)	
{
	return (di.nScore <= 0) ||
		(1 == di.nWeight / CSearchKeyRanking::ClusterFactor % 10 //小扩类
		 && 1 != di.nWeight / CSearchKeyRanking::BaseRelFactor % 10); //低相关	
}

//过滤0值结果
static inline bool filter_zero_weight(const SResult& di)
{
	return di.nScore <= 0;	
}

static inline int getSaleScr(int sale)	
{
	if (sale)
	{	//7日内有销量
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
		// 当分割标志为空时，则认为全部字符串不必被分割，直接返回即可
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
		// 去掉尾部的“slitWord”
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
			string curItem; // 临时存储当前被分割的子串
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
				// 整个字符串中不含有分割符号
				result.push_back(curItem);
			}
			// 向后移动firstPos
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
{ //[0,1],[0,2](只比较第一分类)
        fbCateScr = fbPidScr = 0;
        //类别反馈

	//size_t nCnt = 0;
	//u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针
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
	//单品反馈
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
{       //计算某一指定字段范围内的出现词词距
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
{ //在指定域内判断是否有词距匹配结果(-1-全域，-2-主域，fid)
        static const int OFFSETMAX = 1000;  //最大偏移
        static const int FLDFACTOR = 10;         //字段乘子(2^FLDFACTOR>OFFSETMAX)
        static const int STRICTDISMAX = 2; //严格词距
        if (me.vTerms.size() <= 1)
        {
                return true;
        }
	
        static const int PMinNum = PMinNum;
        //map<int, int> &fid2fi = m_rscSharedMod->fid2fi;
        vector<int> lastKeyPos; //前词出现位置
        vector<bool> idx;                               //标记满足词距位置
        int fwdDis, cnt,ivtCnt, klen, fidi,fid, offset, allOffset;
	map<int, int>::const_iterator iter;
        for (size_t k = 0; k < me.vTerms.size(); ++k)
        {       //索引：title >> series >> T >> B
                fwdDis = pa->pubanalysisdata.objQuery.dis[k];
                klen = me.vTerms[k].len;
                if (0 == k ||true == pa->pubanalysisdata.objQuery.ifSP[k] ||
                                (true == pa->pubanalysisdata.objQuery.keysType[k] || true == pa->pubanalysisdata.objQuery.keysType[k-1]))
                { //首词，或与前词隔空格,或为作者或出版社，不计算词距
                        //首词，或与前词隔空格不计算词距
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
{       //词距过滤
        if (me.vFieldsOff.size() <= 1)
        {
                return true;
        }
        static const int OFFSETMAX = 1000;  //最大偏移
        static const int FLDFACTOR = 10;         //字段乘子(2^FLDFACTOR>OFFSETMAX)
        vector<pair<int, vector<int> > > vPos;
        int cnt, ivtCnt, fid, offset;
        vector<int> pTmp;
        //map<int, int> &fid2fi = fid2fi;
        size_t valCnt = 0;      //有效词数
        for (size_t k = 0; k < me.vFieldsOff.size(); ++k)
        {       //索引：title >> series >> T >> B
                //if (vKeywords[k].keyWeight > 0)
                if(static_cast<int>(log(1/me.vIdf[k]))>0)
                {       //只记录有效词
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
        int accDis = 0;         //累计词距
        size_t nextCnt = 0;     //近邻个数
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
                                fwdItvlDis = 0;         //正向间隔
                                for (int idx = idx_i+1; idx < idx_j; ++idx)
                                {
                                        //fwdItvlDis += vKeywords[idx].keyLength + vKeywords[idx].dis;
                                        //fwdItvlDis += me.vFieldLen[idx] + me.vAllowGap[idx];
                                        fwdItvlDis += me.vTerms[idx].len + me.vAllowGap[idx];
                                }
                                //fwdItvlDis += vKeywords[idx_j].dis;
                                fwdItvlDis += me.vAllowGap[idx_j];
                                //计算两词绝对最短距离
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
                { //奇数
                        return nextCnt >= (valCnt > 5 ? (valCnt-1) * (valCnt-1) / 4 : valCnt);
                }
                else
                {       //偶数
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
        {       //整个字段匹配
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
        {       //标题特有子串匹配
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
        { //有类别信息(只记录第一分类)
                vCluster[(int)vCluster.size() - cateLevel].insert(clsid);
                if (highRelSale > 0)
                {       //高相关有销量
                        if (ifAutPubSearch)
                        {       //仅人名、出版社搜索
                                //二级类
                                highRelSaleCate[GetClassByLevel(2,clsid)].push_back(highRelSale);
                        }
                        else
                        {       //其他搜索
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
		{       //7日内有销量
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
		{       //7日内无销量
			if (newProduct)
			{       //新品
				commerScr = 3;
				if (presale)
				{       //预售
					++commerScr;
				}
			}
			else if (evalue >= 3)
			{       //有评论数
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

//计算聚类等级[1-4]
static inline int ComputeCateLevel(bool feedback, int fieldScr)
{
        if (feedback)
        {       //反馈4等级
                return 4;
        }
        else if (fieldScr > 3)
        {       //文本3等级【1-3】
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
                { //单短字
                        ++singleCnt;
                }
		//printf("AurPubKey[%s]=%d\n",key.c_str(),AurPubKey[key]);
                if (AurPubKey[key] != -1)
		{
			aurpub = true;
			pa->pubanalysisdata.objQuery.ifAutPubSearch = pa->pubanalysisdata.objQuery.keysType[i] = AurPubKey[key];	
		}
                if (false == queryItems[i].field.empty())
                { //高级搜索
                        pa->pubanalysisdata.objQuery.ifLimitSearch = true;
                }
                if (i > 0 && queryItems[i].fwdSP)
                { //间隔空格
                        pa->pubanalysisdata.objQuery.ifSingleString = false;
                }
                pa->pubanalysisdata.objQuery.dis[i] = queryItems[i].fwdDis;
                pa->pubanalysisdata.objQuery.ifSP[i] = queryItems[i].fwdSP;
                /*if (0 == objQI.keysRepeat[i])
		{       //重复词检验及记录
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
        { //全单字
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
        { //有反馈
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
	int match = -1;			//(1-高相关，0-一般相关，-1不相关)
        int fieldScr = 0;               //字段分数[0,9]
        int fieldFlag = 0;              //(4-标题，3-同义，2-副标，1-主区)
        bool disFlag = 1;               //词距相邻标识
        bool withAutPubSearch = false;  //带作者出版社搜索
        {
                //字段初步判定
                bool ifInT = false;             //是否在主域
                bool ifInTitle = false;         //是否在标题/同义标题
                bool ifInISBN = false;          //是否在isbn
                bool ifInMainTitle = false;     //是否在标题重区
                int titleSearchType = -1;       //标题搜索类型(TITLE/TITLEEX/-1)
                int pos_title = INT_MAX;        //在标题中首次出有效词位置
                int pos_title_ex = INT_MAX;     //同义标题首次出有效词位置
                {
                        bool ifPrimaryAppear = false;   //标题主区字段出词
			bool ifSubAppear = false;       //标题副区字段出词
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
                                        {       //主区
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

                        //整理
                        if (true == ifInTitle)
                        {
                                titleSearchType = pos_title < INT_MAX ? TITLEID : TITLEEX;
                                if (ifPrimaryAppear || !ifSubAppear)
                                {       //重区
                                        ifInMainTitle = true;
                                }
                        }
			//if(fieldNum > TMinNum)
				//fprintf(stderr,"fieldNum=%d\tifInMainTitle=%d\n",fieldNum,ifInMainTitle);
                }
                //计算字段得分及过滤
                if (true == ifInT)
                { //主区
                        fieldFlag = 1;
                        if (true == ifInISBN)
                        {       //isbn搜索
                                match = 1;
                                fieldScr = 9;
                        }
                        else if (pa->pubanalysisdata.objQuery.ifAutPubSearch > 0)
                        { //作者/出版社搜索
                                if (true == withAutPubSearch)
                                {
                                        match = 1;
                                        fieldScr = 9;
                                        fieldFlag = 2;
                                }
                                else if (true == ifInTitle)
                                {
                                        if (ifInMainTitle)
                                        {       //重区
                                                match = 1;
                                                fieldScr = 8;
					 }
                                        else
                                        {       //次区
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
                        {       //标题搜索
                                if (true == ifInTitle)
                                {
                                        match = 1;
                                        fieldScr = ifInMainTitle ? 8 : 6;
                                        fieldFlag = titleSearchType == TITLEID ? 4 : 3;
                                        {       //精确匹配
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
                                        {       //可能为未收录的人名/出版社(idf限制)
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
                {       //副区
                        match = -1;
                        fieldScr = 1;
                }
                //针对缺货商品和低相关度商品情况
		if(!isebook)
		{
			if (match < 0)
			{       //相关度低，商业价值低
				if (!stock || !imaged || (!sale && !evalue))
				//if (!stock || (!sale && !evalue))
				{
					return;
				}
			}
			else
			{       //有一定相关度，但商业价值太低
				if (!stock && date >= 30 && !sale  && !evalue && !presale)
				{
					return;
				}
			}
			//无图无销量情况
			if (!imaged && !sale)
			//if (!sale)
			{
				if (match != 1)
				{ //过滤
					return;
				}
				else
				{       //降权
					fieldScr = 5;
					match = 0;
				}
			}
		}
        }
        //商业价值
        int commerScr = 0;      //商业因素等级[0-9]
        int saleScr = 0;
        //销量细节权重
        bool newProduct = false;        //新品标识
        int mixRecord = 0;                              //标记是否有销量，是否有预售，是否为新品
        {
                //新品
                //if ( stock && imaged && date < 30
		if ( stock && imaged  &&date < 30
                                && saleprice > 0)
                {       //有货有图有价格上架短
                        int PubDate = pubdate;
                        int CurDate = time(NULL);       //(since 1970)
                        if ( 0 == PubDate || (CurDate >= PubDate
                                                && (CurDate - PubDate) / 86400 < 240) )
                        {       //上架时间距出版时间不超过8个月
                                newProduct = true;
                        }
                }
                //销量
                ComputeCommerce(sale, evalue, newProduct, presale, commerScr, saleScr);
		//ComputeCommerce(stock,sale, evalue, newProduct, commerScr, saleScr);
                //记录
                //mixRecord = (sale ? 0x4 : 0) + (presale ? 0x2 : 0) + (newProduct ? 0x1 : 0);
		mixRecord = (sale ? 0x4 : 0) +  (newProduct ? 0x1 : 0);
        }
        //反馈权重
        bool fbCateScr = 0;     //反馈类别[0,1]
        int fbPidScr = 0;               //反馈增量[0,2]
        int dropScr = 0;                //削弱髯侄纹ヅ涠?
        {
                if ( 0 == pa->pubanalysisdata.objQuery.ifAutPubSearch
			 && true == pa->pubanalysisdata.objQuery.ifHasFeedback)
                {       //有反馈query(只限标题搜索)
                        if (match == 1 && (sale > 1)) // || presale))
                        {       //高相关，有销量或预售
                                JudgePidHasQueryTag(
                                                me.id, pa, fbCateScr, fbPidScr);
                                if (0 == fbCateScr)
                                { //例外补充
                                        if (newProduct && fieldScr > 7)
                                        {       //极高相关有货新品
                                                fbPidScr = 1;
                                        }
                                }
                                else if (9 == fieldScr)
                                {               //削弱反馈商品本身的馔昝榔ヅ洌×恳韵课?
                                        dropScr = -1;
                                }
                        }
                }
        }
        //聚类信息(为聚类、新品服务)
        int cateLevel = 0;      //聚类等级[1-4]
        {
                cateLevel = ComputeCateLevel(fbCateScr, fieldScr);
                int highRelSale = (match == 1) && (sale > 0) ? sale : 0;
                RecordCate(pa->pubanalysisdata.vCluster, pa->pubanalysisdata.highRelSaleCate,
                                me.id, cateLevel, highRelSale, pa->pubanalysisdata.objQuery.ifAutPubSearch);
        }
        //基本相关性及相关计数
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
        rt.nScore = BaseRelFactor * baseRel                 //基本相关度
                + Weight_Feedback * fbCateScr
                + Weight_Field * fieldScr
                + Weight_Commerce * commerScr
                + Weight_Feedback_Pid * fbPidScr
                + Weight_Sale * saleScr
                + Weight_Drop * dropScr;
        rt.nWeight = StockFactor * stock                     //库存
                + BaseRelFactor * highRel       //高相关标识
                + FieldIndicator * fieldFlag
                + DisIndicator * disFlag
                + NewIndicator * mixRecord              //新品标识
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
        int count_Q = (int)me.vTerms.size();    //关键词数


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


        //相关性
        int match = -1;                 //(1-高相关，0-一般相关，-1不相关)
        int fieldScr = 0;               //字段分数[0,9]
        int fieldFlag = 0;      //(4-标题，3-同义，2-副标，1-主区)
        bool disFlag = 0;               //词距相邻标识
        int weight_T = 0;               //关键词权重和(主区)
	int cnt_TT = 0,count_QQ=0;
        {
                //map<int, int> &fid2fi = fid2fi;

                //字段初步判定
                bool ifAllInT = false;                          //是否全在主域
                bool ifAllInTitle = false;                      //是否全在标题/同义标题
		bool ifAllInSubtitle = false;                   //是否全在副标题
                bool ifInMainTitle = false;                     //是否在标题重区
                bool withAutPubSearch = false;          //带作者出版社搜索
                int titleSearchType = -1;                       //标题搜索类型(TITLE/TITLEEX/-1)
                int weight_Q = 0;                               //关键词权重和(query)
                int length_T = 0;                               //有效关键词在标题字长
                int pos_title = INT_MAX;                        //在标题中首次出有效词位置
                int pos_title_ex = INT_MAX;                             //同义标题首次出有效词位置
                int m1,m2;
                {
                        bool ifPrimaryAppear = false;   //标题主区字段出词
                        bool ifSubAppear = false;       //标题副区字段出词
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
                                                {       //主区
                                                        if (false == ifAdd_T)
                                                        {
								ifAdd_T = true;
                                                                ++cnt_T;
                                                        }
							//fprintf(stderr,"cnt_T=%d\n",cnt_T);
                                                        if (fieldNum == TITLEID)
                                                        {       //标题
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
                                                        { //同义
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
                                                        {       //作者/出版社
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
                        //整理
			//fprintf(stderr,"cnt_T=%d\tcnt_ti=%d\tcnt_ti_ex=%d\tcount_Q=%d\tcnt_st=%d\n",cnt_T,cnt_ti,cnt_ti_ex,count_Q,cnt_st);
			//fprintf(stderr,"cnt_TT=%d\tcount_QQ=%d\n",cnt_TT,count_QQ);
			if (cnt_T == count_Q)
                        {       //全在主区
                                ifAllInT = true;
                                fieldFlag = 1;
                        }
                        if (cnt_ti || cnt_ti_ex)
                        {       //标题有词
                                if (ifPrimaryAppear || !ifSubAppear)
                                {       //重区
                                        ifInMainTitle = true;
                                }

                                titleSearchType = cnt_ti >= cnt_ti_ex ?
                                        TITLEID : TITLEEX;

                                if (titleSearchType == TITLEID)
                                {       //标题
                                        if (cnt_ti == count_Q)
                                        {       //全在标题
                                                ifAllInTitle = true;
                                                fieldFlag = 4;
                                        }
                                }
                                else
                                {       //同义
                                        if (cnt_ti_ex == count_Q)
                                        { //全在标题
                                                ifAllInTitle = true;
                                                fieldFlag = 3;
                                        }
                                }
                        }
                        if (1 == fieldFlag && cnt_st == count_Q)
                        {       //全在副标
                                ifAllInSubtitle = true;
                                fieldFlag = 2;
                        }
                }
		//printf("计算字段分数\n");
		//计算字段分数
                static const size_t LongQueryMinLen = 10;       //长query门限
                static const size_t HasSubStrMaxLen = 16;       //开启精准匹配query最长上限
                if (cnt_TT == count_QQ)
                {       //全在主区
                        if (true == ifAllInTitle)
                        {       //全在标题/同义
                                int matchFieldID = titleSearchType == TITLEID ?
                                        PTITLE_FID : TITLE_EX_FID;
                                if ( (pa->pubanalysisdata.objQuery.validString.size() >= LongQueryMinLen
                                                && !pa->pubanalysisdata.objQuery.ifAllSingleWord)
                                                || true == JudgeDis(pa,me, matchFieldID) )
                                { //满足词距条件
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
                                                { //标题精准匹配开启
                                                        fieldScr = 9;
                                                }
                                        }
                                }
                                else
                                {       //不满足词距条件
                                        if (ifInMainTitle)
                                        {       //在重区
                                                if (true == pa->pubanalysisdata.objQuery.ifHasFeedback)
                                                {       //有反馈query(只限标题搜索)
                                                        match = 1;
                                                        fieldScr = 7;
						 }
                                                else
                                                {
                                                        int dis = ComputeDis(me, matchFieldID);
                                                        if ( dis <= 10)
                                                        {       //词距较小
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
                                        {       //在次区
                                                match = 0;
                                                fieldScr = 4;
                                        }
                                }
                        }
                        else if ( true == withAutPubSearch
                                        || (weight_T > 0 && weight_Q == weight_T) )
                        {       //带作者/出版社/ISBN搜索（具有较明显限定）
                                match = 1;
                                if (titleSearchType != -1)
                                {       //标题有词
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
                                                {//标题精准匹配开启
                                                        fieldScr = 9;
                                                }
                                        }
                                }
                                else
                                { //作者/出版社/ISBN混合搜索
                                        fieldScr = 9;
                                }
                        }
                        else
                        {       //有出现在作者、出版社中
                                if (true == ifAllInSubtitle)
                                {       //全部出现在作者/出版社/ISBN中
                                        if ( (pa->pubanalysisdata.objQuery.validString.size() >= LongQueryMinLen
                                                                && !pa->pubanalysisdata.objQuery.ifAllSingleWord)
                                                        || true == JudgeDis(pa,me, -2) )    //在主域
                                        { //满足词距条件(可能为新作者/出版社等)
                                                match = 1;
                                                fieldScr = 5;
                                                disFlag = 1;
                                        }
                                        else
                                        {       //不满足
                                                match = 1;
                                                fieldScr = 2;
                                        }
                                }
                                else if (true == ifInMainTitle && weight_T > 0 &&
                                                ((weight_T << 1) >= weight_Q) )
                                {       //标题重区出现重要权重词
                                        match = 0;
                                        fieldScr = weight_T == weight_Q ? 5 : 4;
				}
                                else
                                {       //标题不满足重要权重词
                                        match = 0;
                                        fieldScr = 2;
                                }
                        }
                }
                else if(0< cnt_TT < count_QQ)
                { //不全在主区
                        if ( pa->pubanalysisdata.objQuery.validString.size() <= 6
                                        || pa->pubanalysisdata.objQuery.ifAllSingleWord)
                        { //特殊搜索(单短串/全单字串)
                                if (false == JudgeDis(pa,me))
                                {       //严格词距过滤
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
                        {       //一般搜索
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
                                {       //词距过滤
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
                //针对缺货商品和低相关度商品情况
                if(!isebook)
                {
                        if (match < 0)
                        {       //相关度低，商业价值低
                                if (!stock || !imaged || (!sale && !evalue))
				//if (!stock || (!sale && !evalue))
                                {
                                        return;
                                }
                        }
                        else
                        {       //有一定相关度，但商业价值太低
                                if (!stock && date >= 30 && !sale && !evalue  && !presale)
                                {
                                        return;
                                }
                        }
			//无图无销量情况
                        if (!imaged && !sale)
			//if(!sale)
                        {
                                if (match != 1)
                                { //过滤
                                        return;
                                }
                                else
                                {       //降权
                                        fieldScr = 5;
                                        match = 0;
                                }
                        }
                }
        }
	//printf("商业价值\n");
        //商业价值
        int commerScr = 0;                              //商业因素等级[0-9]
        int saleScr = 0;                                        //销量细节权重
        bool newProduct = false;        //新品标识
        int mixRecord = 0;                              //标记是否有销量，是否有预售，是否为新品
        {
                //新品
                //if ( stock && imaged && date < 30
		if ( stock && imaged  && date < 30
                                && saleprice > 0)
                {       //有货有图有价格上架短
                        int PubDate = pubdate;
                        int CurDate = time(NULL);       //(since 1970)
                        if ( 0 == PubDate || (CurDate >= PubDate
                                                && (CurDate - PubDate) / 86400 < 240) )
                        {       //上架时间距出版时间不超过8个月
                                newProduct = true;
                        }
                }

                //销量
                //ComputeCommerce(stock, evalue, newProduct, presale, commerScr, saleScr);
		ComputeCommerce(stock,sale, evalue, newProduct, commerScr, saleScr);
		//记录
                //mixRecord = (sale ? 0x4 : 0) + (presale ? 0x2 : 0) + (newProduct ? 0x1 : 0);
		mixRecord = (sale ? 0x4 : 0) +  (newProduct ? 0x1 : 0);
        }
	//printf("反馈权重\n");
        //反馈权重
        bool fbCateScr = 0;     //反馈类别
        int fbPidScr = 0;               //反馈增量
        int dropScr = 0;                //削弱
        {
                if (true == pa->pubanalysisdata.objQuery.ifHasFeedback)
                {       //有反馈query(只限标题搜索)
                        if (match == 1 && (sale > 1)) // || presale))
                        {       //基本相关，有销量或预售
                                JudgePidHasQueryTag(
                                                me.id, pa, fbCateScr, fbPidScr);

                                if (0 == fbCateScr)
                                { //例外补充
                                        if (newProduct && fieldScr > 7)
                                        {       //极高相关有货新品
                                                fbPidScr = 1;
                                        }
                                }
                                else if (9 == fieldScr)
                                {       //削弱反馈商品本身的完美匹配，尽量以销量为主
                                        dropScr = -1;
                                }
                        }
                }
        }

        //聚类信息
	//printf("聚类信息\n");
        int cateLevel = 0;      //聚类等级[1-4]
        {
                cateLevel = ComputeCateLevel(fbCateScr, fieldScr);
		//fprintf(stderr,"cateLevel=%d\tfbCateScr=%d\tfieldScr=%d\n",cateLevel,fbCateScr,fieldScr);
                int highRelSale = (match == 1) && (sale > 0) ? sale : 0;
                RecordCate(pa->pubanalysisdata.vCluster, pa->pubanalysisdata.highRelSaleCate,
                                me.id, cateLevel, highRelSale, false);
	}
	//printf("基本相关性及相关计数\n");
        //基本相关性及相关计数
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
        rt.nScore = BaseRelFactor * baseRel          //基本相关度
                + Weight_Feedback * fbCateScr
                + Weight_Field * fieldScr
                + Weight_Commerce * commerScr
                + Weight_Feedback_Pid * fbPidScr
                + Weight_T_AREA * weight_T
                + Weight_Sale * saleScr
                + Weight_Drop * dropScr;

       rt.nWeight = StockFactor * stock                     //库存
                + BaseRelFactor * highRel       //高相关标识
                + FieldIndicator * fieldFlag
                + DisIndicator * disFlag
                + NewIndicator * mixRecord              //新品标识
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
	//后期排序处理(未排序未过滤)
	if (vRes.empty())
	{
		return;
	};
	static const int CateWeight = Weight_Cate;
	static const int FieldWeight = Weight_Field;
	static const int CommerceWeight = Weight_Commerce;
	static const int FeedbackWeight = Weight_Feedback;
	static const int PerfectMatch = 9;      //精确匹配
	static const int MainHighMatch = 8;     //重区相关
	static const int AuthorSearch = AUTHOR;
	static const int PubSearch = PUBNAME;
	//Fst, 初步排序
	/*std::sort(
	  vRes.begin(), vRes.end(),
	  boost::bind(
	  std::greater<int>(),
	  boost::bind(&SResult::nScore, _1),
	  boost::bind(&SResult::nScore, _2)));*/

	//Snd, 聚类,推荐,过滤
	int AutPubSearch = pa->pubanalysisdata.objQuery.ifAutPubSearch > 0
		&& 1 == pa->pubanalysisdata.objQuery.keysType.size() ?
		pa->pubanalysisdata.objQuery.ifAutPubSearch : 0; //仅作者/出版社搜索
	bool ifLongQuery = pa->pubanalysisdata.objQuery.validString.size() > 16 ? true : false;
	if ( NULL != pa
			&& false == pa->pubanalysisdata.objQuery.ifLimitSearch)
	{       //非高级搜索(高级搜索无推荐，无聚类，无精简)
		/*if (pa->pubanalysisdata.TDocCnt <= 0)
		{       //无相关结果
			vRes.clear();
			return;
		}*/

		//聚类
		set<u64> mInCluster;              //聚类基类
		map<u64, bool> mExtCluster; //扩展类(与mInCluster无交集)
		int InClusterLevel = 0; //在聚类中的类等级(4-反馈/3-极高相关/2-较高相关/1-低相关)
		bool ifHasDirOrBig = false;     //是否有直类或大括类结果或推荐结果
		int fairResultCnt = 0;
		{
			//聚类收集
			if (false == pa->pubanalysisdata.vCluster.empty())
			{
				if (false == pa->pubanalysisdata.vCluster[0].empty())
				{ //反馈聚类等级
					mInCluster.insert(pa->pubanalysisdata.vCluster[0].begin(),                                                                                                  pa->pubanalysisdata.vCluster[0].end());
				}
				if (false == pa->pubanalysisdata.objQuery.fbCate.empty())
				{       //query反馈类别(含百货)
					const vector<pair<int, int> >& cate =
						pa->pubanalysisdata.objQuery.fbCate;
					for (size_t c = 0; c < cate.size(); ++c)
					{
						mInCluster.insert(cate[c].first);
					}
				}
				if (false == mInCluster.empty())
				{
					InClusterLevel = 4;     //反馈聚类
				}
				else
				{       //文本等级(只取最高一级)
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
				//扩展类生成(只允许高相关类扩展)
				if ( InClusterLevel > 1
						&& cid2cids.size() != 0)
				{       //扩展类
					int catid,catid2;
					for (set<u64>::const_iterator iter = mInCluster.begin();
							iter != mInCluster.end(); ++iter)
					{
						HASHVECTOR ext;
						ext = cid2cids[*iter];	
						if(ext.count != 0)
						{       //有括类
							for (size_t v = 0; v < ext.count; ++v)
							{
								memcpy(&catid,ext.data+v*ext.size,4);
								if ( mInCluster.find(catid) == mInCluster.end()
										&& (mExtCluster.find(catid) == mExtCluster.end()
											|| 0 == mExtCluster[catid]) )
								{ //大/小括类
									memcpy(&catid2,ext.data+v*ext.size+4,4);
									mExtCluster[catid] = catid2;	
								}
							}
						}
					}
				}
			}
			//聚类过滤
			if (InClusterLevel && false == mInCluster.empty())
			{
				//计算聚类得分
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
					int cateType = 0;       //4-基类/3-直类/2-大括类/1-小括类
					cateLevel = vRes[i].nWeight / ClusterFactor % 10;

					if (cateLevel >= InClusterLevel)
					{       //聚类基类中
						cateScr = 1;
						cateType = 4;
						++fairResultCnt;
					}
					else
					{ //查看聚类
						u64 cls = (u64)m_funcFrstInt64(m_clsProfile,vRes[i].nDocId);
						if (cls != 0)
						{ //有类别
							bool ifCluster = false;
							//const vector<int> &cate = it->second;

							//直类判断
							if (mInCluster.find(cls) != mInCluster.end())
							{
								cateScr = 1;
								cateType = 3;
								ifCluster = true;
								ifHasDirOrBig = true;
								++fairResultCnt;
							}

							//扩展类判断
							if (false == ifCluster)
							{
								if ( false == mExtCluster.empty()
										&& vRes[i].nWeight / StockFactor % 10
										&& mExtCluster.find(cls) != mExtCluster.end())
								{ //有货商品
									if (1 == mExtCluster[cls])
									{ //大括类
										cateType = 2;
										ifHasDirOrBig = true;
										++fairResultCnt;
									}

									else
									{ //小括类
										cateType = 1;
									}
									cateScr = 1;
									ifCluster = true;
								}

								if ( false == ifCluster
										&& 0 == vRes[i].nWeight / BaseRelFactor % 10)
								{       //过滤（非高相关)
									//fprintf(stderr,"process pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
									vRes[i].nScore = 0;
									continue;

								}
								/*else if (vRes[i].nWeight / PubCommerceFactor % 10 <= 0)
								{       //高相关但无商业价值
									vRes[i].nScore -= 3 * FieldWeight;
								}*/
							}
						}
						else if (0 == vRes[i].nWeight / BaseRelFactor % 10)
						{       //过滤（非高相关)
							//fprintf(stderr,"process pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
							vRes[i].nScore = 0;
							continue;

						}
						/*else if (vRes[i].nWeight / PubCommerceFactor % 10 <= 0)
						{       //高相关但无商业价值
							vRes[i].nScore -= 3 * FieldWeight;
						}*/
					}
					//fprintf(stderr,"cateType=%d\n",cateType);
					if ((1 == cateType) || (4 == cateType &&
								vRes[i].nWeight / FeedBackFactor % 10 > 0))
					{       //削减小扩类、反馈级类得分
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
			{       //无聚类信息，所有非相关结果置0
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
		//过滤清理及排序
		if (ifHasDirOrBig)
		{ //过滤小括类且低相关结果
			if (fairResultCnt > HighRelBound)
			{
				vRes.erase(std::remove_if(vRes.begin(), vRes.end(), filter_small_ext_all),vRes.end());
			}
			else
			{
				vRes.erase(std::remove_if(vRes.begin(), vRes.end(), filter_small_ext),vRes.end());
				//小扩类商业削减
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
		{ //过滤零weight
			vRes.erase(std::remove_if(vRes.begin(), vRes.end(), filter_zero_weight),vRes.end());
		}
		sort(vRes.begin(), vRes.end());
		/*for(size_t i = 0; i < vRes.size();i++)
		{
			printf("here after process pid=%d\tnScore=%d\tnWeight=%d\n",vRes[i].nDocId,vRes[i].nScore,vRes[i].nWeight);
		}*/
	}
	//Thd, 库存,新品调序
	if (pa->pubanalysisdata.TDocCnt_stock <= 0 && pa->pubanalysisdata.TDocCnt > 0)
	{       //所有有货结果相关度低且存在有一定相关度的缺货结果，将高相关缺货结果上提
		static const int NoStockMax = 2;        //允许无货商品在top出现的个数
		multimap<int, int, greater<int> > weight2idx;
		for (size_t i = 0; i < vRes.size(); ++i)
		{
			if (vRes[i].nWeight/TypeIndicator != 0)
			{
				continue;
			}
			if (0 == vRes[i].nWeight / StockFactor % 10)
			{       //缺货
				if (vRes[i].nWeight / FieldFactor % 10 >= MainHighMatch)
				{
					weight2idx.insert(make_pair(vRes[i].nScore, i));
				}
			}
			else
			{       //有货
				vRes[i].nScore += 1 * StockFactor;
			}
		}
		multimap<int, int, greater<int> >::const_iterator iter = weight2idx.begin();
		int count = 0;
		for (; count < NoStockMax && iter != weight2idx.end(); ++count, ++iter)
		{
			vRes[iter->second].nScore += 1 * StockFactor; //伪库存分
		}
	}
	else
	{
		static const size_t NoStockMax = 2;     //允许无货商品在top出现的个数
		static const size_t TopPosMin = 4;      //无货商品最高出现位置
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
			{       //缺货
				if ( (mixScr & 4)
						&& (PerfectMatch == fieldScr || (MainHighMatch == fieldScr && ifLongQuery))
						&& (noStockCount < NoStockMax) )
				{       //还有缺货位且有精确匹配有销量
					if (i <= TopPosMin || !tmp.empty())
					{       //位置太靠前
						tmp.push_back(pair<int, int>(i, vRes[i].nScore));
					}
					else
					{       //位置满足
						vRes[i].nScore += 1 * StockFactor;    //伪库存分
						if (i && vRes[i].nScore == vRes[i-1].nScore)
						{
							--vRes[i].nScore;
						}
					}
					++noStockCount;
				}
			}
			else
			{       //有货
				++hasStockCount;
				vRes[i].nScore += 1 * StockFactor;

				if (!tmp.empty() && hasStockCount > TopPosMin)
				{       //有精确匹配待处理
					int tmpWeight = vRes[i].nScore;
					for (size_t t = 0; t < tmp.size(); ++t)
					{
						vRes[tmp[t].first].nScore = --tmpWeight;
					}
					tmp.clear();
				}

				//新品
				if ((mixScr & 1) && (fieldScr >= MainHighMatch))
				{ //极高相关
					//hash_map<int, vector<int> >::const_iterator it =m_joinMap.find(vDocs[i].pid);
					//if (it != m_joinMap.end())
					size_t nCnt = 0;
					u64* ptr = (u64*)m_funcValPtr(m_clsProfile, vRes[i].nDocId, nCnt);
					{ //有类别
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
						{       //有足够近似集
							simCateIdx = 0;
						}


						if (simCateIdx >= 0)
						{       //找到足够近似集
							int catefmt = AutPubSearch ?
								(cate[simCateIdx] >> 16) : (cate[simCateIdx] >> 8);
							vector<u64>& highSale =
								pa->pubanalysisdata.highRelSaleCate[catefmt];
							sort(highSale.begin(), highSale.end(), greater<int>());
							highSale.resize(10, 0);

							//其他辅助信息
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
							{       //赋值
								if (newCommerceScr == 9)
								{       //避免太过
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
