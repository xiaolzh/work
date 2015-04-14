#ifndef LISTRANKING_H
#define LISTRANKING_H
#include "Module.h"
#include <string>
#include <ext/hash_map>
#include <vector>
#include <math.h>
#include "nokey_static_hash.h"
#include "configer.h"
#include "weighter.h"

struct cls_buf
{
	char chCLs[56];
};

using std::string;

class ListAnalysisData : public IAnalysisData {
public:
	enum {PUB_LIST, MALL_LIST, UNKNOWN_LIST};
	ListAnalysisData(){m_list_type = UNKNOWN_LIST;}
	virtual ~ListAnalysisData(){;}

	int m_list_type;//查询类型 
    u64 m_cat_id;

    /// @todo add more list data for analysis
    int cur_date;
    int page_size;
};

struct ScatterInfo {
    int field_id;
    int unit_num;
    vector<int> priority_list;
};

class CListRanking:public CModule {
public:
    struct WeightCoff{
        int stock;
        int commerce;
        int review;
        int sale;
        WeightCoff(): stock(0), commerce(0), review(0), sale(0) {}
    };
    
public:
	virtual ~CListRanking();

	/*
	*初始化函数需要具体模块重载
	*Parameter psdi 搜索数据传递
	*Parameter strConf 模块各自的配置文件地址
	*return true初始化成功，false初始化失败 
	*/
	virtual bool Init(SSearchDataInfo* psdi, const string& strConf);

	/*
	* Method:    QueryRewrite 查询重写
	* Returns:   void
	* Parameter: hash_map<string, string> & hmParams 查询各参数 map形式，
    *            可以修改KEY对应的VALUE
	*/
	virtual void QueryRewrite(hash_map<string,string>& hmParams);

	/*
	* Method:    query分析
	* Returns:   void
	* Parameter: SQueryClause & qc 结构化的查询语句
	* return: IAnalysisData*  返回的query分析数据 
    *         用户动态生成，框架负责销毁，生成失败需返回NULL
	*/
	virtual IAnalysisData* QueryAnalyse(SQueryClause& qc);
	/*
	* Method:    计算非文本搜索权重
	* Returns:   void
	* Parameter: vector<SResult> & vRes 搜索打分结果
	* Parameter: IAnalysisData* Pad, 返回的query分析数据
	*/
	virtual void ComputeWeight(IAnalysisData* pad, vector<SResult>& vRes); 

	/*
	* Method:    重新剪枝计算，常用于非默认排序
	* Returns:   void
	* Parameter: IAnalysisData* Pad, 返回的query分析数据
	* Parameter: vector<SResult> & vRes 搜索打分结果
	*/
	virtual void ReRanking(vector<SResult>& vRes, IAnalysisData* pad);

	/*
	*Method:  填充统计信息，将内部以ID统计的量转换为文字 
    *         例如CLASS PATH--> CLASS NAME
	*Parameter:vGpInfo 传入数组 字段ID对应-->统计信息
	*/
	virtual void FillGroupByData(
        vector<pair<int, vector<SGroupByInfo> > >& vGpInfo);

	/*
	* Method:    搜索默认排序（如按照其他字段如价格排序不走该接口）
	* Returns:   void
	* Parameter: vector<SResult> & vRes 搜索打分结果
	* Parameter: IAnalysisData* Pad, 返回的query分析数据
	* Parameter: from 用户索取的文档在 vRes中的起始位置
	* Parameter: to 用户索取的文档在 vRes中的终止位置（不含）
	*            from to 为用户取第几页，每页多少个的另一种表现形式，
    *            如取第五页，每页50个 则from=200,to=250
	*/
	virtual void SortForDefault(vector<SResult>& vRes, int from, int to, 
        IAnalysisData* pad);

	/*
	* Method:    ShowResult 自定义展现信息
    *            <result><cost></cost>
    *            *****用户自定义填充内容******
    *            </result>"
	* Returns:   void
	* Parameter: IAnalysisData* Pad,    返回的query分析数据
	* Parameter: CControl & ctrl        查询控制参数包括翻页，排序项等
	* Parameter: GROUP_RESULT & vGpRes  分组汇总统计信息，
	* Parameter: vector<SResult> & vRes 搜索打分结果
	* Parameter: vector<string> & vecRed 标红字符串数组
	* Parameter: string & strRes         返回字符串
	*/
	virtual void ShowResult(IAnalysisData* pad, CControl& ctrl, 
        GROUP_RESULT &vGpRes, vector<SResult>& vRes, vector<string>& vecRed, 
        string& strRes);
	
	void SortForCustom(vector<SResult>& vRes, int from, int to, 
        IAnalysisData* pad);

public:
    /**
     *    extra functions
     */	
    int ComputeDefaultWeight(int doc_id, IAnalysisData* pa);

protected:
	//searcher for get doc data

private:
    bool _LoadConfig(const string& conf_file);
    bool _ConfigDefaultWeight(const vector<string>& factors); 
    bool _ConfigCustomWeight(const map<string, string>& cat_configs, 
        const map<string, string>& bak_configs);
    bool _ConfigScatter(const string& scatter); 
    void _LoadPriorityDocid();
    bool _LoadProfile(const char* field, void*& profile);
    u64 _GetWeightCat(u64 cat_id);

    inline bool _IsMall(u64 cls_id) {
        /// 百货以58分类开始
        return (*(char*)&cls_id == 5*16+8);
    }

    inline int _GetStockWeight(int doc_id, int coff) {
        int stock = 0;
        if(NULL != m_stock_profile)
            stock = m_funcFrstInt(m_stock_profile, doc_id);
        int stock_scr = 0;
		if (stock > 0) {
			if ((stock & 0x01) > 0) stock_scr = 3;
			else stock_scr = 2;
		}else{
			stock_scr = 1;
		}
        /// normalize to be [0, 100]
        stock_scr *= 100000;
        return stock_scr*coff;
    }

    inline int _GetSaleWeight(int doc_id, int coff) {
        int sale_week = 0;
        if(NULL != m_sale_week_profile)
            sale_week = m_funcFrstInt(m_sale_week_profile, doc_id); 
        /// normalize to be [0, 100]
        return (int)(log(1 + sale_week)*coff);
    }

    inline int _GetReviewWeight(int doc_id, int coff) {
        int review_count = 0;
        if(NULL != m_review_count_profile)
            review_count = m_funcFrstInt(m_review_count_profile, doc_id);
        /// normalize to be [0, 100]
        return (int)(log(1 + review_count)*coff);
    }

    int _GetCommerceWeight(int doc_id, int coff, IAnalysisData* pa);
	
	virtual void SetGroupByAndFilterSequence(IAnalysisData* pad, 
        vector<SFGNode>& vec);

	virtual void CustomFilt(IAnalysisData* pad, vector<int>& vDocIds, 
        SFGNode& fgNode);

	virtual void CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, 
        SFGNode& gfNode, pair<int, vector<SGroupByInfo> >& prGpInfo);
	virtual void BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, 
			vector<PSS>& vGpName,GROUP_RESULT& vGpRes);
private:
    // --------------------------------------------
    // following are field profiles
    // --------------------------------------------

    /// 库存字段的属性指针，获取每个商品的库存
    void* m_stock_profile;
    /// 周销量字段的属性指针，获取每个商品的周销量
    void* m_sale_week_profile;
    /// 公用品字段的属性指针，获取每个商品的是否是公用品
    void* m_is_share_product_profile;
    /// 浏览数字段的属性指针，获取每个商品的浏览数
    void* m_review_count_profile;
    /// 评分字段的属性指针，获取每个商品的评分
    void* m_score_profile;
    /// 入库时间字段的属性指针，获取每个商品的入库时间
    void* m_first_input_date_profile;
    /// 出版时间字段的属性指针，获取每个商品的出版时间
    void* m_publish_date_profile;

    ///排序靠前
    hash_map<long long, hash_map<long long, int> > m_priority_cat_pid;



    WeightCoff m_default_weight;
    map<u64, Weighter*>  m_custom_weighters;
    vector<void*> m_dl_handles;

    /// the [cat_path, ScatterInfo] map, save the config of field and weight
    /// to a root category path
    map<u64, ScatterInfo> m_scatters;
	
	//static_hash_map<u64, cls_buf> m_staticCatPathName;
    ///----------------------------------------------------------------
    /// follow codes are kept for later ranking using feed back data
    ///----------------------------------------------------------------
    /** 每个分类对应的pid及其反馈数据中点击收藏数量映射的权重
     *  u64: 分类id， vector<u64>: pid|权重(1-9)
     *  @todo 这里仅考虑部分pid，若某分类包含的pid过多则截断考虑
     */
    //hash_map<u64, vector<u64> > m_cat2pid;
    /** 在购买收藏中pid对应的关键词与其数量映射的权重
     *  int：pid； string：关键词,int：权重 1-9
     *  @todo 限于pub，可考虑后期扩展为pub+b2c
     */
    //static_hash_vector<int,vector<pair<string,int> > > m_query2tags;
    /** 过滤的关键词对应的百货类别id与数量
     *  string：品牌；pair<int,int>：类别id,搜索数量
     *  @todo 限于b2c, 可考虑与上面pub的融合
     *  @todo 过滤or未过滤，that's a question
     */
    //static_hash_vector<string,vector<pair<int,int> > > m_query2cate;
    /** 近期点击，购买，暂存表中的关键字对应的pid
     *  string：关键词int：pid
     *  @todo 限于b2c
     */
    //static_hash_vector<string,vector<int> > m_query2pid;
};
#endif	
