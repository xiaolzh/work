#ifndef EG_KEY_RANKING_H
#define EG_KEY_RANKING_H
#include "Module.h"
#include "UtilTemplateFunc.h"
#include "QueryParse.h"
#include "Class.h"
#include "static_hash_vector.h"
#include <string>

using namespace std;

//#define DEBUG

struct MallAnalysisPart {
	//基本词特征
	vector<int> type;							//词性(0-一般，1-产品，2-品牌)
	vector<int> keysRepeat;				//重复词统计
	vector<int> vDis;      //@todo: 每个TERM对应字段的偏移， 对应原有数据结构KeyItem中的dis
	string querystr;							//去标点有效词连串
	bool needJudgeDis;						//需要计算词距

	//query语义分析
	string pdtWord;								//产品搜索词
	string pdtNg;									//产品搜索语素
	bool ifPdtQuery;							//产品搜索
	bool ifBrdQuery;							//品牌搜索

	//反馈特征
	vector<int>	vPid;							//单品反馈
	vector<pair<u64, int> > vCate;	//类别定位

	//结果统计
	int relCnt_fb;								//相关结果数（当有反馈时使用）
	int relCnt_prd;								//相关结果数（当特殊类型搜索时使用）
	set<u64> topCate_fb;					//相关顶级类（当有反馈时使用）
	set<u64> topCate_prd;					//相关顶级类（当特殊类型搜索时使用）
	int relCnt_brd;								//相关结果数（品牌搜索时在品牌字段）
	int relCnt_ti;								//相关结果数（品牌搜索时在标题字段）
	set<u64> topCate_brd;
	set<u64> topCate_ti;

	vector<CateStat> cateStat;		//多类展示query类别信息统计
	map<u64, int>	cid2cnum;				//多类展示cateId-cate编号(cateStat下标)映射
};

struct PubQueryAnalysisPart
{
	//Query信息结构
	struct QueryInfo
	{
		QueryInfo():
			hasSet(false),
			ifHasFeedback(false),ifSingleString(true),
			ifAllSingleWord(false),ifLimitSearch(false),
			ifAutPubSearch(0),keywordCnt(0){}

		bool hasSet;                                            //已经设定完毕
		bool ifHasFeedback;             //是否为有反馈query
		bool ifSingleString;            //单串（无空格）
		bool ifAllSingleWord;           //全单字
		bool ifLimitSearch;                     //高级搜索
		int ifAutPubSearch;               //人名出版社搜索
		size_t keywordCnt;                      //关键词数
		string validString;                     //有效部分连串
		vector<int> keysType;           //关键词类别
		vector<int> keysRepeat; //重复词计数
		vector<int> keyWeight;
		vector<int> dis;
		vector<int> type;
		vector<bool> ifSP;
		vector<pair<int, int> > fbCate;        //反馈类别
	};

	QueryInfo objQuery;                             //Query信息
	vector<set<u64> > vCluster;                     //聚类信息
	map<u64, vector<u64> > highRelSaleCate;         //高相关有销量二级类存储
	size_t RelDocCnt;                               //高相关文档数统计
	size_t TDocCnt;                                 //主区较相关文档数统计
	size_t TDocCnt_stock;

};


class CDDAnalysisData:public IAnalysisData
{
	public:
		enum {FULL_SITE_SEARCH, PUB_SEARCH, MALL_SEARCH};
		CDDAnalysisData(){m_searchType = 0;}
		virtual ~CDDAnalysisData(){;}

		//common member
		int m_searchType;//查询类型 
		int m_otherSortField;//查询是否有其他排序字段
		//pub member
		PubQueryAnalysisPart pubanalysisdata;
		//mall member
		MallAnalysisPart m_mallAnalysisPart;

};

//class path 21.32.41.00 ->id   = (2*16+1) + (3*16+2)<<8 + (4*16+1)<<16
//是否是百货
inline bool isMall(unsigned long long cls_id)
{
	//百货以58分类开始
	return *(char*)&cls_id == 5*16+8;
}



class CEgKeyRanking:public CModule
{
	public:
		virtual ~CEgKeyRanking();

		/*
		 *初始化函数需要具体模块重载
		 *Parameter psdi 搜索数据传递
		 *Parameter strConf 模块各自的配置文件地址
		 *return true初始化成功，false初始化失败 
		 */
		virtual bool Init(SSearchDataInfo* psdi, const string& strConf);

		bool InitCommon(SSearchDataInfo* psdi, const string& strConf);
		bool InitPub(SSearchDataInfo* psdi, const string& strConf);
		bool InitMall(SSearchDataInfo* psdi, const string& strConf);


		/*
		 * Method:    query分析
		 * Returns:   void
		 * Parameter: SQueryClause & qc 结构化的查询语句
		 * Parameter: CDDAnalysisData& pa IAnalysisData 派生类 
		 * return: IAnalysisData*  返回的query分析数据 用户动态生成，框架负责销毁，生成失败需返回NULL
		 */
		virtual IAnalysisData* QueryAnalyse(SQueryClause& qc);

		void QueryAnalyseCommon(CDDAnalysisData* pa, SQueryClause& qc);
		void QueryAnalyseMall(CDDAnalysisData* pa, SQueryClause& qc);
		void QueryAnalysePub(CDDAnalysisData* pa, SQueryClause& qc);


		/*
		 * Method:    计算文本搜索权重
		 * Returns:   void
		 * Parameter: IAnalysisData * pa 返回的query分析数据
		 * Parameter: SMatchElement & me每个文档的匹配信息
		 * Parameter: SResult & rt 打分结果
		 */
		virtual void ComputeWeight(IAnalysisData* pa, SMatchElement& me, SResult& rt);

		void BrandQRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt) const;
		void SingleRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void MultiRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void ComputeCommerceWeight(const int docID, int& commerceScr, int& saleDetail) const;
		void ComputeSpecialWeight(const int docID, CDDAnalysisData* pa, int& fbCateWeight,
				int& fbPidWeight, int& pdtCoreWeight, int& relScr);
		void ComputeWeightMall(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void CombineComputeWeight_ComQ(CDDAnalysisData* pa, vector<SResult>& vRt);
		void CombineComputeWeight_PrdQ(CDDAnalysisData* pa, vector<SResult>& vRt);
		void CombineComputeWeight_BrdQ(CDDAnalysisData* pa, vector<SResult>& vRt);

		void ComputeWeightPub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);

		//出版物单个词计算权重
		void RankingSinglePub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		//出版物多个词计算权重
		void RankingMultiPub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		bool LoadFile(SSearchDataInfo* psdi, const string& strConf);
		vector<string> GetStrings(const string& stringList, const string& splitWord);


		/*
		 * 用于混排的各个函数
		 */
		float GetAlpha(const string& query);
		void ReComputeWeightFullSite(vector<SResult>& vRes, float alpha);


		/*
		 * Method:    精简结果，二次打分等操作，可用于非默认排序
		 * Returns:   void
		 * Parameter: IAnalysisData* Pad, 返回的query分析数据
		 * Parameter: vector<SResult> & vRes 搜索打分结果
		 */
		virtual void ReRanking(vector<SResult>& vRes, IAnalysisData* pad);

		void ReRankingFullSite(vector<SResult>& vRes, CDDAnalysisData* pa);
		void FilterMallResults(vector<SResult>& vRt);
		void ReRankingMall(vector<SResult>& vRes, CDDAnalysisData* pa);
		void ReRankingPub(vector<SResult>& vRes, CDDAnalysisData* pa);


		/*
		 * Method:    搜索默认排序（即不存在其他排序条件如价格），分类多样性的结果穿插重排可放这里
		 * Returns:   void
		 * Parameter: vector<SResult> & vRes 搜索打分结果
		 * Parameter: IAnalysisData* Pad, 返回的query分析数据
		 * Parameter: from 用户索取的文档在 vRes中的起始位置
		 * Parameter: to 用户索取的文档在 vRes中的终止位置（不含）
		 * from to 为用户取第几页，每页多少个的另一种表现形式，如取第五页，每页50个 则from=200,to=250
		 */
		virtual void SortForDefault(vector<SResult>& vRes, int from, int to, IAnalysisData* pa);

		void SortFullSite(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa);
		void SortPub(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa);
		void SortMall(vector<SResult>& vRes, int from, int to, CDDAnalysisData* pa);





		/*
		 * Method:    ShowResult 自定义展现信息<result><cost></cost>*****用户自定义填充内容******</result>"
		 * Returns:   void
		 * Parameter: IAnalysisData* Pad,    返回的query分析数据
		 * Parameter: CControl & ctrl        查询控制参数包括翻页，排序项等
		 * Parameter: GROUP_RESULT & vGpRes  分组汇总统计信息，
		 * Parameter: vector<SResult> & vRes 搜索打分结果
		 * Parameter: vector<string> & vecRed 标红字符串数组
		 * Parameter: string & strRes         返回字符串
		 */
		virtual void ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
				vector<SResult>& vRes, vector<string>& vecRed, string& strRes);




		/*
		 *Method:  填充统计信息，将内部以ID统计的量转换为文字 例如CLASS PATH--> CLASS NAME
		 *Parameter:vGpInfo 传入数组 字段ID对应-->统计信息
		 */
		virtual void FillGroupByData(GROUP_RESULT&  vGpInfo);

	private:
	private:
		/*//出版物单个词计算权重
		  void RankingSinglePub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		//出版物多个词计算权重
		void RankingMultiPub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);*/
		/** @function
		 *********************************
		 <PRE>
		 函数名：JudgePidHasQueryTag
		 功能    ：判断文档是否有反馈
		 参数    ：
		 * @param  [in] pID      文档对应的产品ID
		 * @param  [in] queryValidString 关键词有效连串
		 * 返回值        反馈权重[0,9]
		 </PRE>
		 **********************************/
		inline void JudgePidHasQueryTag(
				const int pid, const CDDAnalysisData *analysisDat,
				bool& fbCateScr, int& fbPidScr);

		/** @function
		 *********************************
		 <PRE>
		 函数名：JudgeDis
		 功能    ：判断关键词在该文档内是否词距匹配
		 参数    ：
		 * @param  [in] keywords 关键词信息
		 * @param  [in] fidlmt   域限定(-1为全域)
		 </PRE>
		 **********************************/
		inline bool JudgeDis(
				const CDDAnalysisData *pa,
				const SMatchElement& me,
				int fidlmt = -1);
		/** @function
		 *********************************
		 <PRE>
		 函数名：JudgeDisFilter
		 功能    ：词距过滤器
		 参数    ：
		 * @param  [in] vKeywords        关键词信息
		 </PRE>
		 **********************************/
		inline bool JudgeDisFilter(
				const SMatchElement& me);

		inline int ComputeDis(
				const SMatchElement& me,
				size_t fidlmt);

		/** @function
		 *********************************
		 <PRE>
		 函数名：JudgeTotalMatch
		 功能    ：判断关键词是否与标题精确匹配
		 参数    ：
		 * @param  [in] objQuery 关键词信息
		 * @param  [in] objDoc   文档信息
		 * @param  [in] fid      字段ID
		 * @param  [in] pos_title        关键词在标题首次出现位置
		 </PRE>
		 **********************************/
		inline bool JudgeTotalMatch(
				const string& queryValidString,
				const SMatchElement& me,
				int fid,
				int pos_title);
		/** @function
		 *********************************
		 <PRE>
		 函数名：RecordCate
		 功能    ：记录文档所属类别信息
		 参数    ：
		 * @param  [out] vCluster        聚类信息存储
		 * @param  [in] pid      文档产品id
		 * @param  [in] level    文档类别等级
		 * @param  [in] cateScore        类别权重
		 * @param  [in] match    文档基本相关性
		 </PRE>
		 **********************************/
		inline void RecordCate(
				vector<set<u64> >& vCluster,
				map<u64, vector<u64> >& highRelSaleCate,
				int pid, int cateLevel, int highRelSale,
				bool ifAutPubSearch);


	protected:
		//searcher for get doc data

	private:
		//common member
		void* m_clsProfile;//分类字段的属性指针，用于获取每个文档的分类
		void* m_stockProfile;//库存字段的属性指针，获取每个商品的库存
		void* m_saleWeekProfile;
		void* m_inputDateProfile;
		void* m_isShareProductProfile;
		void* m_isPidProfile;
		;
		//pub member
		//int FID_AUTHOR_NAME;//作者字段ID
		;
		/*************************百货数据 mall member*******************************/
		//调试权重因子
		static const int FeedbackCateFactor = 100;
		static const int FeedbackPidFactor = 10;
		static const int ProdCentreFactor = 1;
		static const int FieldFactor = 1000;
		static const int RelIndicator = 10000;
		//static const int CommerceFactor = 100000; // 去除商业因素
		static const int TypeIndicator = 1000000000; //表示出版物或者百货，用户混排
		//排序权重因子
		//static const int BaseRelFactor = 10000000;	//相关指示位(与出版物一致)
		static const int FakeScore =   1101000000;	//虚假部分（支持混排）
		static const int CommerceFactor = 0;
		static const int Weight_FBCate = 100;	//反馈类[0-9]
		static const int Weight_FBPid = 10;		//反馈单品[0-1]
		static const int Weight_Field = 60;		//基本字段[0-4]
		static const int Weight_PdtCore = 25;	//产品中心[0-2]
		static const int Weight_Dis = 30;			//词距[0-1]
		static const int Weight_Brand = 30;		//品牌字段支持[0-1]
		// 去除商业因素
		//static const int Weight_Commerce = 20;//商业因素[0-6]
		static const int Weight_Commerce = 0;
		static const int Weight_Public = 10;	//公用品[0-1]
		// 去除商业因素
		//static const int Weight_Sale = 1;		  //销量细节(0-30)
		static const int Weight_Sale = 0;

		//int FID_BRAND_NAME;//品牌字段ID
		int TITLENAME;			//标题
		int TITLESYN;		//同义扩展
		int BOTTOMCATE;	//底级类
		int OTHERCATE;	//其他级类
		int BRAND;			//品牌
		int SUBTITLE;		//副标题
		int TMinNum;		//主区字段最小编号
		int TITLE_FID;
		int TITLESYN_FID;
		int BRAND_FID;
		int BOTTOMCATE_FID;
		int OTHERCATE_FID;
		int SUBNAME_FID;
		int PRODUCTID_FID;
		int SHARE_PID_FID;
		int SALE_WEEK_FID;
		int INPUT_DATE_FID;
		vector<int> m_fid2fi;		//字段Id到编号Index映射
		static_hash_vector<string, vector<pair<u64,int> > > m_key2Cate; //@todo: 需确认m_mallAnalysisPart.querystr
		//是否与反馈文件中的保持一致
		static_hash_vector<string, vector<pair<u64,int> > > m_key2CateBrd;
		static_hash_vector<string, vector<int> > m_key2Pid;
		static_hash_vector<int, vector<string> > m_pid2Core;
		set<string> m_pdtWords;			//产品库
		set<string> m_brdWords;			//品牌库

		/***********************混排数据**************************/
		static_hash_map<string, pair<int,int> > m_percentPub2Mall;
		static const int m_base_weight = 1000000000;
		static const int m_max_weight_pub = 110100000;
		static const int m_max_weight_bh = 1111000000;

		/***********************出版物数据*************************/
	public:
		//static const int RecoIndicator = 1000000000;//推荐指示位
		static const int StockFactor =  100000000;      //库存指示位
		static const int BaseRelFactor = 10000000;      //相关指示位
		static const int RelevantScoreKeyMin = StockFactor + BaseRelFactor;
		static const int FieldIndicator = 1000000;      //出现字段指示
		static const int DisIndicator = 100000;                 //词距指示
		static const int NewIndicator = 10000;                  //新品指示位
		static const int ClusterFactor = 1000;                  //聚类乘子
		static const int FeedBackFactor = 100;                  //反馈乘子
		static const int PubFieldFactor = 10;                      //字段乘子
		static const int PubCommerceFactor = 1;                    //商业乘子
		static const int ClusterLevels = 4;                     //聚类等级数(1个反馈级别，3个文本级别)
		static const size_t ResultMaxCnt = 500;                 //聚类结果集最大数
		static const size_t HighRelBound = 80;                  //高相关结果数界限
		//static const int Weight_Field = 60;
		static const int PubWeight_Commerce = 40;
		static const int Weight_Feedback = 70;
		static const int Weight_Feedback_Pid = 40;
		static const int Weight_Cate = 80;
		static const int Weight_T_AREA = 30;
		//static const int Weight_Sale = 1;
		static const int Weight_Drop = 25;
	private:
		int FID_AUTHOR_NAME;//作者字段ID
		int FID_TITLE;
		int FID_TITLEEX;
		int FID_PUBNAME;
		int FID_ISBN;
		int FID_TITLEPRI;
		int FID_TITLESUB;
		int FID_SERIES;
		int FID_ABSTRACT;
		int FID_CONTENT;
		int FID_COMMENT;
		int FID_CATALOG;
		int FID_ISBNSEARCH;
		int FID_SINGER;
		int FID_CONDUCTOR;
		int FID_DIRECTOR;
		int FID_ACTOR;

		int TITLEID;
		int TITLEPRI;
		int TITLESUB;
		int TITLEEX;
		int AUTHOR;
		int PUBNAME;
		int ISBN;
		int SERIES;
		int ABSTRACT;
		int CONTENT;
		int COMMENT;
		int CATALOG;
		//int TMinNum;    //主区字段最小编号
		//int TITLE_FID;                  //记录title的fieldID
		int TITLE_PRI_FID;      //记录标题重要区字段fieldID
		int TITLE_SUB_FID;      //记录标题次要区字段fieldID
		int TITLE_EX_FID;               //记录标题同义字段fieldID

		//pub member
		void* m_TOTAL_REVIEW_COUNTProfile;//查看字段的属性指针，获取每个商品的view
		//void* m_pre_saleProfile;//销量字段的属性指针，获取每个商品的销量
		//void* m_num_imagesProfile;//图片字段的属性指针，获取每个商品的图片
		void* m_SalePriceProfile;
		//void* m_LastInputDateProfile;
		//void* m_PubDateProfile;
		void* m_sale_dayProfile;

		//hash_map<string, vector<pair<u64, bool> > > Query2Cate;                 //有反馈query及反馈类别
		static_hash_vector<string,vector<pair<int,int> > > Query2Cate;
		//hash_map<string, int> AurPubKey;                                        //作者和出版社
		static_hash_map<string,int> AurPubKey;
		//multimap<int, string> pid2tags;                                         //单品反馈词对
		static_hash_vector<int,vector<pair<string,int> > >  pid2tags;
		//map<u64, vector<pair<u64, bool> > > cid2cids;                           //聚类扩展用
		static_hash_vector<int,vector<pair<int,int> > >  cid2cids;
		//hash_map<int, pair<string, string> > pid2sub;                           //标题精确匹配用
		static_hash_vector<int,vector<string> > pid2sub;
		set<u64> hotNPCate;
		map<int, int> fid2fi;
};
#endif	
