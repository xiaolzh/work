#ifndef KEYRANKING_H
#define KEYRANKING_H
#include "Module.h"
#include "GlobalDef.h"

//query分析产生的子类
class CKeySrchAnalysis:public IAnalysisData
{
public:
	CKeySrchAnalysis(){}
	virtual ~CKeySrchAnalysis(){;}
	//do it .
};



class CKeyRanking:public CModule
{
public:
	virtual ~CKeyRanking(){fprintf(stderr, "key ranking leave\n");}

	/*
	*初始化函数需要具体模块重载
	*Parameter psdi 搜索数据传递
	*Parameter strConf 模块各自的配置文件地址
	*return true初始化成功，false初始化失败 
	*/
	virtual bool Init(SSearchDataInfo* psdi, const string& strConf);

	/*
	* Method:    query分类转换到特定处理模块，每个业务有且仅有一个入口模块，可以有多个辅助模块
	* Returns:   void
	* Parameter: SQueryClause & qc 结构化的查询语句
	* return: CModule*  返回的转换到的模块指针
	*/
	virtual CModule* QueryClassify(SQueryClause& qc);

	
	/*
	* Method:    query分析
	* Returns:   void
	* Parameter: SQueryClause & qc 结构化的查询语句
	* return: IAnalysisData*  返回的query分析数据 用户动态生成，框架负责销毁，生成失败需返回NULL
	*/
	virtual IAnalysisData* QueryAnalyse(SQueryClause& qc);


	/*
	*Method:  填充统计信息，将内部以ID统计的量转换为文字 例如CLASS PATH--> CLASS NAME
	*Parameter:vGpInfo 传入数组 字段ID对应-->统计信息
	*/
	virtual void FillGroupByData(GROUP_RESULT&  vGpInfo);

	virtual void SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad);
	
	/*
	* Method:    计算权重分析
	* Returns:   void
	* Parameter: IAnalysisData * pad 返回的query分析数据
	* Parameter: SMatchElement & me每个文档的匹配信息
	* Parameter: SResult & rt 打分结果
	*/
	virtual void ComputeWeight(IAnalysisData* pad, SMatchElement& me, SResult& rt);

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
	*Method:  运行于GROUP_BY之前;
	*Parameter: IAnalysisData* Pad,    返回的query分析数据
	*Parameter: vector<SResult> & vRes 搜索打分结果
	*Parameter: vector<PSS> & vGpName  分组汇总的字段名和信息值
	*Parameter: GROUP_RESULT    vGpRes 传入数组 字段ID对应-->统计信息
	*/
	virtual void BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, vector<PSS>& vGpName, GROUP_RESULT& vGpRes);

	virtual void SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vec);
	virtual void CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
                                    pair<int, vector<SGroupByInfo> >& prGpInfo);

protected:
	void* m_pflSalePrice ;
	void* m_pflPromoPrice ;

};
#endif	
