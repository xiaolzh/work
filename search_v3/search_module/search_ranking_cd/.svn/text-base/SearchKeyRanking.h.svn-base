#ifndef EG_KEY_RANKING_H
#define EG_KEY_RANKING_H
//#define DEBUG
#include "Module.h"
#include "UtilTemplateFunc.h"
#include "QueryParse.h"
#include "Class.h"
#include "static_hash_vector.h"
#include "ddip.h"
#include <string>
#include <vector>

using namespace std;

const int SCATTER_UPPER_LIMIT = 500;
const int FISRT_CATE_LIMIT    = 3;
const int SCATTER_CAT_LIMIT   = 6;

struct SReferCatInfo
{
	u64 cid;
	int rate;
	int iLev;
};

struct SScatterCategory
{
	u64 cid;
	int cnt;
	int scnt;
	int sunit;
	int iLev;
};

struct QueryAnalysisPart
{
	//基本词特征
	QueryAnalysisPart():ifHasFeedback(false),
	        ifAllSingleWord(false), ifLimitSearch(false)
	{
	}
	vector<int> type;						//词性(0-一般 1-产品 2-品牌)
	vector<int> pubtype;					//出版物词性(0-出版社 1-作者)
	vector<int> dis;						//每个TERM对应字段的偏移
	string queryStr;						//去标点有效词连串
	string key;								//原始查询串
	bool needJudgeDis;						//需要计算词距

	//query语义分析
	string pdtWord;							//产品中心词
	//string pdtNg;							//产品搜索语素
	bool ifPdtQuery;						//产品搜索
	bool ifBrdQuery;						//品牌搜索

	//反馈特征
	//vector<int>	vPid;						//单品反馈
	vector<pair<u64, int> > vCate;			//分类反馈
	//vector<pair<u64, int> > vBrdCate;		//品牌词对应的分类反馈
	

	//结果统计
	//int relCnt_fb;							//相关结果数（当有反馈时使用）
	//int relCnt_prd;							//相关结果数（当特殊类型搜索时使用）
	//set<u64> topCate_fb;					//相关顶级类（当有反馈时使用）
	//set<u64> topCate_prd;					//相关顶级类（当特殊类型搜索时使用）
	//int relCnt_brd;							//相关结果数（品牌搜索时在品牌字段）
	//int relCnt_ti;							//相关结果数（品牌搜索时在标题字段）
	//set<u64> topCate_brd;					//与品牌词相关的顶级类
	//set<u64> topCate_ti;					//与标题相关的顶级类

	//出版物专用字段
	bool ifHasFeedback;           			//是否为有反馈query
	bool ifAllSingleWord;            		//全单字
	bool ifLimitSearch;                		//高级搜索
	int ifAutPubSearch;               		//作者出版社搜索
	vector<bool> ifSP;						//是否间隔空格
	vector<set<u64> > vCluster;             //聚类信息
	map<u64, vector<u64> > highRelSaleCate;	//高相关有销量二级类存储
	size_t relDocCnt;
	size_t tDocCnt;                        	//主区较相关文档数统计
	size_t tDocCnt_stock;					//主区较相关文档且有库存数统计
};

struct FieldToId
{
	char* field;
	int id;
};

inline bool isMall(unsigned long long cls_id)
{
    //百货以58分类开始
    return *(char*)&cls_id == 0x58;
}

/*
手机通讯：58.80.00.00.00.00
数码影音：58.59.00.00.00.00
电脑办公：58.63.00.00.00.00
大家电：  58.82.00.00.00.00
家用电器：58.01.00.00.00.00
*/

inline bool is3C(unsigned long long cls_id)
{
	char* tmpId = (char*)&cls_id;
	int secCate = tmpId[1] & 0xff;
	
	if(secCate == 0x01 || secCate == 0x63 || secCate == 0x82 || secCate == 0x59 || secCate == 0x80)
	{
		return true;
	}
    return false;
}

inline bool isCloth(unsigned long long cls_id)
{
    char* tmpId = (char*)&cls_id;
    int secCate = tmpId[1] & 0xff;
	if(secCate == 0x64 || secCate == 0x65)
	{
		return true;
	}
    return false;
}

class CDDAnalysisData:public IAnalysisData
{
	public:
		//说明：{图书搜索，服装/鞋靴搜索，3C搜索，其它品类搜索, 全站搜索}
		enum {PUB_SEARCH, CLOTH_SEARCH, C3_SEARCH, OTHER_SEARCH, FULL_SITE_SEARCH};	//不能随便变动顺序，需要与混排数据类型的顺序一致
		CDDAnalysisData(){m_searchType = 0;}

		int m_searchType;					//查询类型 
		int m_otherSortField;				//查询是否有其他排序字段
		QueryAnalysisPart m_AnalysisPart;
		//=========================whj=========================//
        int m_bit_city_location;
		vector<SReferCatInfo> vReferCat;
};

class CSearchKeyRanking:public CModule
{
	public:
		virtual ~CSearchKeyRanking();

		/*
		 *初始化函数需要具体模块重载
		 *Parameter psdi 搜索数据传递
		 *Parameter strConf 模块各自的配置文件地址
		 *return true初始化成功，false初始化失败 
		 */
		virtual bool Init(SSearchDataInfo* psdi, const string& strConf);

		bool InitCommon(SSearchDataInfo* psdi, const string& strConf);
		bool InitData();


		/*
		 * Method:    query分析
		 * Returns:   void
		 * Parameter: SQueryClause & qc 结构化的查询语句
		 * Parameter: CDDAnalysisData& pa IAnalysisData 派生类 
		 * return: IAnalysisData*  返回的query分析数据 用户动态生成，框架负责销毁，生成失败需返回NULL
		 */
		virtual IAnalysisData* QueryAnalyse(SQueryClause& qc);


		/*
		 * Method:    计算文本搜索权重
		 * Returns:   void
		 * Parameter: IAnalysisData * pa 返回的query分析数据
		 * Parameter: SMatchElement & me每个文档的匹配信息
		 * Parameter: SResult & rt 打分结果
		 */
		virtual void ComputeWeight(IAnalysisData* pa, SMatchElement& me, SResult& rt);

		void ComputeWeightOther(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void BrandQRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void SingleRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void MultiRank(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void ComputeCommerceWeight(const int, int&, int&, int&, int&, int&, int&);
		void ComputeSpecialWeight(const int, CDDAnalysisData*, int&, int&, int&, int&);
		void ChangeWeight(SResult& rt, CDDAnalysisData* pa);
		//void ChangeWeight(SResult& rt);

		void ComputeWeight3C(CDDAnalysisData*, SMatchElement&, SResult&);
		void BrandQRank3C(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
        void SingleRank3C(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
        void MultiRank3C(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void ComputeCommerceWeight3C(const int, int&, int&, int&, int&, int&, int&);
		void ComputeSpecialWeight3C(const int, CDDAnalysisData*, int&, int&, int&, int&);
		void ChangeWeight3C(SResult& rt, CDDAnalysisData* pa);
		//void ChangeWeight3C(SResult& rt);

		void ComputeWeightCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void BrandQRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void SingleRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void MultiRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		void ComputeSpecialWeightCloth(const int, CDDAnalysisData*, int&, int&, int&, int&);
		void ComputeCommerceWeightCloth(const int, int&, int&, int&, int&, int&, int&);
		void ChangeWeightCloth(SResult& rt, CDDAnalysisData* pa);
		//void ChangeWeightCloth(SResult& rt);
		void MoveLeftBit(int& score, int value, int moveNum);

		void ComputeWeightPub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		//出版物单个词计算权重
		void RankingSinglePub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		//出版物多个词计算权重
		void RankingMultiPub(CDDAnalysisData* pa, SMatchElement& me, SResult& rt);
		bool LoadFile();
		vector<string> GetStrings(const string& stringList, const string& splitWord);

		void QueryRewrite(hash_map<string,string>& hmParams);
		/*
		 * 用于混排的各个函数
		 */
		float GetAlpha(const string& query, const int cat);
		void ReComputeWeightFullSite(vector<SResult>& vRes, float alpha,int &bhcount);


		/*
		 * Method:    精简结果，二次打分等操作，可用于非默认排序
		 * Returns:   void
		 * Parameter: IAnalysisData* Pad, 返回的query分析数据
		 * Parameter: vector<SResult> & vRes 搜索打分结果
		 */
		virtual void ReRanking(vector<SResult>& vRes, IAnalysisData* pad);

		void ReRankingFullSite(vector<SResult>& vRes, CDDAnalysisData* pa);
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

		//强制单品关联
		bool RemoveGoodsByForceAndFB(const string& query,vector<SResult>& vRes,vector<SResult>& force_vect);
		bool ForceMoveGoods(vector<SResult>& vRes,vector<SResult>& force_vect);
		/*
		 * Method:    ShowResult 自定义展现信息<result><cost></cost>*****用户自定义填充内容******</result>"
		 * Returns:   void
		 * Parameter: IAnalysisData* Pad,    返回的query分析数据
		 * Parameter: CControl & ctrl        查询控制参数包括翻页，排序项等
		 * Parameter: GROUP_RESULT & vGpRes  分组汇总统计信息，
		 * Parameter: vector<SResult> & vRes 搜索打分结果
		 * Parameter: vector<string> & vecRed 标红字符串数组
		 * Parameter: string & strRes         返刈址
		 */
		virtual void ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
				vector<SResult>& vRes, vector<string>& vecRed, string& strRes);

		virtual void SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vec);
		
		virtual void CustomFilt(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& fgNode);

		 virtual void CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
				                             pair<int, vector<SGroupByInfo> >& prGpInfo);
		/*
		 *Method:  填充统计信息，将内部以ID统计的量转换为文字 例如CLASS PATH--> CLASS NAME
		 *Parameter:vGpInfo 传入数组 字段ID对应-->统计信息
		 */
		virtual void FillGroupByData(GROUP_RESULT&  vGpInfo);

	private:
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

		void ScatterCategory(vector<SResult>& vRes,int scatter_upper,CDDAnalysisData* pa);
		
		void SimplePartialSortDesc(SResult* beg, SResult* mid, SResult* end);

		inline void GetFrstMostElem(SResult& sr,vector<SResult> &vRes,int& frst_cnt,SScatterCategory& sc);

		void SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad);

		void BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, vector<PSS>& vGpName, GROUP_RESULT& vGpRes);
	protected:
		//searcher for get doc data

	private:
		set<string> m_pdtWords;			//产品库
		set<string> m_brdWords;			//品牌库

		/***********************混排数据**************************/
		static_hash_vector<string, vector<float> > m_percentPub2Mall;	//0-出版物 1-服装 2-3C 3-其它
		static const int catenum = 4;		//按照分类进行排序数
		float alpha[catenum];				//各分类商品占总商品数的比重,0-出版物 1-服装 2-3C 3-其它
		//static const int m_base_weight = 1000000000;
		//static const int m_max_weight_pub = 110100000;
		//static const int m_max_weight_bh = 1111000000;

	public:
		/*************************百货数据 mall member*******************************/
		//调试权重因子
		//static const int ProdCentreFactor = 1;		//产品中心匹配度指示位
		//static const int FeedbackPidFactor = 10;		//单品反馈指示位
		//static const int FeedbackCateFactor = 100;	//分类反馈指示位
		//static const int FieldFactor = 1000;			//字段文本匹配指示位
		//static const int RelIndicator = 10000;			//分类反馈匹配、中心词匹配指示位
		//static const int CommerceFactor = 100000;		//商业因素指示位
		static const int TypeIndicator = 1000000000;	//表示出版物或者百货，用户混排
		//排序权重因子
		//static const int FakeScore =   1101000000;	//虚假部分（支持混排）
		//static const int BaseRelFactor = 10000000;	//相关指示位

		//static const int Weight_FBCate = 80;			//反馈类[0-9]
		//static const int Weight_FBPid = 10;			//反馈单品[0-1]
		//static const int Weight_Field = 60;			//基本字段[0-4]
		//static const int Weight_PdtCore = 25;			//产品中心[0-2]
		//static const int Weight_Dis = 30;				//词距[0-1]
		//static const int Weight_Brand = 30;			//品牌字段支持[0-1]
		//static const int Weight_Commerce = 25;		//商业因素[0-16]
		//static const int Weight_Public = 10;			//公用品[0-1]
		//static const int Weight_Sale = 1;				//销量细节[0-42]

		/***********************出版物数据*************************/
		/*调试权重因子
		static const int PubCommerceFactor = 1;		//商业乘子
		static const int FieldFactor = 10;			//字段文本匹配指示位
		static const int FeedBackFactor = 100;		//分类反馈指示位
		static const int ClusterFactor = 1000;		//基类、直类、大扩类、小扩类指示位
		static const int NewIndicator = 10000;		//新品指示位
		static const int CommerceFactor = 100000;	//该项与DisIndicator重复，商业因素指示位
		static const int DisIndicator = 100000;		//词距指示位
		static const int FieldIndicator = 1000000;	//出现字段指示
		static const int StockFactor = 100000000;	//库存指示位

		static const int ClusterLevels = 4;			//聚类等级数(1个反馈级别，3个文本级别)
		static const size_t ResultMaxCnt = 500;		//聚类结果集最大数
		static const size_t HighRelBound = 80;		//高相关结果数界限
		//排序权重因子
		static const int BaseRelFactor = 10000000;	//相关指示位
		static const int Weight_Cate = 80;			//用于重排增加类别权重
		static const int Weight_Feedback = 70;		//反馈类
		static const int Weight_Field = 60;			//基本字段
		static const int Weight_Commerce = 40;		//商业因素[0-6]
		static const int Weight_Feedback_Pid = 40;	//反馈单品
		static const int Weight_T_AREA = 30;		//重区权重和
		static const int Weight_Drop = 25;			//削弱反馈[-1-0]
		static const int Weight_Sale = 1;			//销量细节
		*/
		/***********************出版物数据*************************/
		//调试权重因子
		static const int PubCommerceFactor = 1;		//商业乘子
		static const int FieldFactor = 10;			//字段文本匹配指示位
		static const int FeedBackFactor = 100;		//分类反馈指示位
		static const int ClusterFactor = 1000;		//基类、直类、大扩类、小扩类指示位
		static const int NewIndicator = 10000;		//新品指示位
		static const int CommerceFactor = 100000;	//该项与DisIndicator重复，商业因素指示位
		static const int DisIndicator = 100000;		//词距指示位
		static const int FieldIndicator = 1000000;	//出现字段指示
		static const int StockFactor = 100000000;	//库存指示位
		static const int BaseRelFactor = 10000000;	//相关指示位

		static const int ClusterLevels = 4;			//聚类等级数(1个反馈级别，3个文本级别)
		static const size_t ResultMaxCnt = 500;		//聚类结果集最大数
		static const size_t HighRelBound = 80;		//高相关结果数界限
		//排序权重因子
		static const int Weight_Baserel = 134217728;	//相关指示位
		static const int Weight_Cate = 4;			//用于重排增加类别权重
		static const int Weight_Feedback = 1;		//反馈类
		static const int Weight_Field = 16777216;	//基本字段
		static const int Weight_Dis = 12582912;		//词距匹配
		static const int Weight_Pdtcore = 2097152;	//中心词
		static const int Weight_Autpub = 1048576;	//作者出版社
		static const int Weight_Stock = 32768;		//库存
		static const int Weight_Image = 16384;		//图片
		static const int Weight_Commerce = 24;		//商业因素[0-6]
		static const int Weight_Feedback_Pid = 4;	//反馈单品
		static const int Weight_T_AREA = 4;		//重区权重和
		static const int Weight_Drop = 0;		//削弱文本匹配权重[-1-0]
		static const int Weight_Sale = 1;			//销量细节

		/*服装/鞋靴
		static const int CloWeight_Field = 150;			//基本字段[0-4]
		static const int CloWeight_PdtCore = 60;		//产品中心[0-1]
		//static const int Weight_Dis = 30;				//词距[0-1]
		//static const int Weight_Brand = 30;			//品牌字段支持[0-1]

		static const int CloWeight_FBCate = 40;			//反馈类[0-9]
		//static const int Weight_FBPid = 10;			//反馈单品[0-1]

		static const int CloWeight_Commerce = 10;		//商业因素[0-12]
		//static const int Weight_Public = 10;			//公用品[0-1]
		static const int CloWeight_Sale = 4;			//销量细节(0-30)
		static const int CloWeight_Prediction = 25;		//销售预测[0-2]
		static const int CloWeight_Picture = 10;     	//商品的图片[0-1]
		static const int CloWeight_Comment = 3;      	//商品的评论数[0-10]
		*/


	private:
		//枚举需要与field_id中的元素一一对应
		enum{
			CATALOG = 0,
			CONTENT = 1,
			ABSTRACT = 2,
			SERIES = 3,
			TMINNUM = 4,					//区分重要字段与次要字段
			SUBNAME = 4,
			ISBN = 5,
			BRAND = 6,
			PUBNAME = 7,
			BOTTOMCATE = 8,
			ACTOR = 9,
			DIRECTOR = 10,
			SINGER = 11,
			AUTHOR = 12,
			TITLESYN = 13,
			TITLENAME = 14,
			TITLEPRI = 15,
			TITLESUB = 16
		};
		enum{
			FBPIDBIT = 28,
			BASERELBIT 	= 27,
			TEXTRELBIT = 24,
			TERMDISBIT = 22,
			PDTCOREBIT = 21,
			INBRDBIT = 20,
			FBCATEBIT = 16,
			STOCKBIT = 15,
			IMAGEBIT = 14,
			SALEPREBIT = 6,
			COMMERCEBIT = 1,
			ISPUBBIT = 0
		};
		enum{
			FBPIDBITNUM = 1,
			BASERELBITNUM  = 1,
			TEXTRELBITNUM = 3,
            TERMDISBITNUM = 2,
            PDTCOREBITNUM = 1,
            INBRDBITNUM = 1,
            FBCATEBITNUM = 1,
            STOCKBITNUM = 1,
            IMAGEBITNUM = 1,
            SALEPREBITNUM = 2,
            COMMERCEBITNUM = 5,
            ISPUBBITNUM = 1
		};
		static const int ScoreBitMap[9];	//获取排序因素对应的位的值
		static const FieldToId field_id[17];//字段到编号的映射
		vector<int> m_vFieldIndex;			//字段对应的索引
		vector<pair<string, void**> > m_vProfile;	//存储各个函数指针
		static const char* field[17];		//存储某些字段名
		vector<int> m_fid2fi;				//字段索引到编号映射
		vector<short> m_salAmtScr;			//存储销售额的得分
		vector<short> m_salNumScr;			//存储销量的得分
		vector<short> m_commentScr;			//存储评论数的得分

		//pub member
		void* m_totalReviewCountProfile;	//总浏览数字段的属性指针，袢∶扛錾唐纷苣浏览次数
		void* m_preSaleProfile;				//是否预售商品字段的属性指针，获取每个商品是否是预售商品
		void* m_numImagesProfile;			//图片字段的属性指针，获取每个商品的图片
		void* m_salePriceProfile;			//当当卖价字段的属性指针，获取每个商品的销售价格
		void* m_pubDateProfile;				//出版时间字段的属性指针，获取每个出版物的出版时间
		//common member
		void* m_clsProfile;					//分类字段的属性指针，用于获取每个商品对应的分类
		void* m_stockProfile;				//库存字段的属性指针，获取每个商品的库存
		void* m_stockStatusProfile;         //城市库存信息u64 whj========================
		void* m_saleDayProfile;				//日销量字段的属性指针，获取每个商品一周的销量
		void* m_saleWeekProfile;			//周销量字段的属性指针，获取每个商品一周的销量
		void* m_saleMonthProfile;			//周销量字段的属性指针，获取每个商品一周的销量
		void* m_saleDayAmtProfile;			//日销售额字段的属性指针，获取每个商品昨天的销售额
		void* m_saleWeekAmtProfile;			//周销售额字段的属性指针，获取每个商品一周的销售额
		void* m_saleMonthAmtProfile;		//月销售额字段的属性指针，获取每个商品一月的销售额
		void* m_inputDateProfile;			//上架时间字段的属性指针，获取每个商品的上架时间
		void* m_modifyTime;					//最新上架时间字段的属性指针，获取每个商品的最新上架时间
		void* m_isShareProductProfile;		//是否公用品字段的属性指针，获取每个商品是否是公用品
		void* m_isPidProfile;				//商品ID字段的属性指针，获取每个商品的商品ID
		void* m_isPublicationProfile;

		static_hash_map<string, int> m_aurpub;									//作者和出版社
		static_hash_vector<int, vector<pair<int, int> > >  m_cid2Cids;			//出版物聚类扩展用
		static_hash_vector<int, vector<string> > m_pid2Sub;						//出版物标题精确匹配用
		static_hash_vector<string, vector<pair<u64, int> > > m_key2Cate; 		//分类反馈
		static_hash_vector<string, vector<pair<u64, int> > > m_brdkey2Cate;		//品牌词对应的分类反馈
		static_hash_vector<string, vector<pair<int, int> > > m_key2Pid;			//单品反馈
		static_hash_vector<int, vector<string> > m_pid2Core;					//商品对应的中心词
        //whj
		ddip_t* m_ip_location;	//ip对应城市查询函数的指针
		hash_map<string,int> m_city_blocation;	//城市对应的位位置
public:
		bool InitPersonalityLocationStockDict();	//初始化个性化地域库存字典
		bool LoadIp2LocationDict(const string& ip_file);
		bool LoadCity2BitLocationDict(const string& city_file);
		bool GetUserIp2Location(CDDAnalysisData* pa);	//获取地域信息
		bool JudgeLocationStock(int doc_id, int bit_city_location, int& out) const;
		//模板选择
		u64 JudgeResultMAinCate(vector<SResult>& vRes, int end);
		bool SelectShowTemplate(CDDAnalysisData* pa,vector<SResult>& vRes, int end);
		void FindAndReplaceShowTempalte(string& str_reserve,string str_template);

};
#endif	
