/*
*模块内存管理原则
*1传入的指针都不需要用户释放
*2用户动态生成的IAnalysisData对象系统自己释放
*/


#ifndef MODULE_H
#define MODULE_H
#include <string>
#include <vector>
#include <map>
#ifdef  NEW_SEG
#include "../segment_new/segmentor.h"
#else
#include "../segment/ta_dic_w.h"
#endif
#include "GlobalDef.h"
using namespace std;
using segment::Term;


//匹配类型
enum {
	EXACT_MATCH,
	GAP_MATCH,
	OVER_FIELD_MATCH
};

//Log 级别
enum {
	SL_DEBUG,
	SL_INFO,
	SL_NOTICE,
	SL_WARN,
	SL_ERROR,
	SL_CRIT,
	SL_FATAL
};

#define COMMON_LOG(level, fmt, ...)  (m_funcWriteLogptr(m_pLogger, true, level, __FILE__, __LINE__,  fmt, ##__VA_ARGS__));
#define SESSION_LOG(level, fmt, ...)  (m_funcWriteLogptr(m_pLogger, false, level, __FILE__, __LINE__,  fmt, ##__VA_ARGS__));

//query分析 基类接口需要写模块者派生
class IAnalysisData
{
public:
	IAnalysisData(){bAdvance = false; bAutoAd = true;}
	virtual ~IAnalysisData(){;}


	bool bAdvance; //是否高级搜索
	bool bAutoAd;//是否高级搜索自动处理


	hash_map<string,string> m_hmUrlPara;//url参数 KEY/VALUE形式，包装展示信息时候可能会有用
	vector<int> m_queryFieldIds; //表示到倒排索引的哪些字段去搜当前query，如果为空，表示到所有字段去搜
	vector<Term> m_vTerms;   //各分词后TERM
	string m_strReserve;    //通用字串
	//do it .
};

// 模块信息 每个模块初始化时候传入模块映射表用于调用其他模块的功能
// 调用map查找到的其他模块指针只是在函数内部有效，不可以保存，
//  否则将会产生无法预料的错误,原因为模块支持重载，重载后原指针无效。
class CModule;
struct SModuleInfo
{
	CModule* pMod;//对象指针
	void   * hDll; //动态库句柄
	int      tm;//时间戳
};
typedef map<string, SModuleInfo> MOD_MAP;

typedef pair<string, string> PSS;


/*
*以下函数指针用于获取数字字段值
*FPTR_GET_FRST_NUM_PTR:  获取单值数字字段值 返回数字指针，
*						 使用中转换为配置文件的类型 1字节-char 2字节-short 4字节-int 8字节-long long
*FPTR_GET_FRST_INT_VAL:  获取int型数字字段值,返回int整数
*FPTR_GET_FRST_64BIT_VAL 获取long long型数字字段值,返回long long整数
*FPTR_GET_VAL_PTR:       获取单值/或多值数字字段值,返回数字指针，
*pProfile:               数字字段属性，不能为NULL
*nDocId:                 文档id
*nCnt:                   字段值个数,可能为0
*/
typedef void* (*FPTR_GET_FRST_NUM_PTR)(void* pProfile, int nDocId);
typedef int   (*FPTR_GET_FRST_INT_VAL)(void* pProfile, int nDocId);
typedef long long  (*FPTR_GET_FRST_INT64_VAL)(void* pProfile, int nDocId);
typedef void* (*FPTR_GET_VAL_PTR)(void* pProfile, int nDocId, size_t& nCnt);


/*展示文档时获取文档内容使用 
*nDocId:                            文档ID
*vector<int>& vShowFieldIds:        需要获取的所有字段ID，需要从小到大排序
*vector<char*>& vFieldDataPtr;获取到每个字段内容均转换成字符串，保存起始地址，无需DELETE
*vector<char>& vDataBuf;            用于获取数据的缓冲区
*void pSearcher                    搜索器指针，模块初始化时候传入
*/
typedef void (*FPTR_GET_DOC_INFO)(int nDocId, vector<int>& vShowFieldIds, 
									vector<char*>& vFieldDataPtr, 
									vector<char>& vDataBuf, void* pSearcher);

/*
*
*FPTR_GROUPBY_CLASS:     按照分类汇总，
*pProfile:               数字字段属性，不能为NULL
*vDocId:                 文档序列id
*nRetCnt:                最多返回个数,可能为0
*vGroupBuf:              汇总信息
*bUseClassTree:          是否使用分类树
*clsId:                  基准分类ID
*/
typedef void (*FPTR_GROUPBY_CLASS)(void* pProfile, vector<int> &vDocId, int nRetCnt, 
								vector<SGroupByInfo> &vGroupBuf, bool bUseClassTree, unsigned long long clsId);

typedef void (*FPTR_GET_DOCS_BY_PK)(void* pSearcher, int nFieldId, vector<long long>& vPk, vector<int>& vDocIds);
typedef void (*FPTR_WRITE_LOG)(void* pLogger, bool bSingle, int level, const char* file, int line, const char *fmt, ...);
typedef void (*FPTR_SCATTER_RESULT)(void* pSearcher, vector<SResult>& res, vector<int>& pri, int fid, int unit,int sort_upper);

//函数指针结构体
struct SGetValFuncs
{
	FPTR_GET_FRST_NUM_PTR   funcFrstPtr;
	FPTR_GET_FRST_INT_VAL   funcFrstInt;
	FPTR_GET_FRST_INT64_VAL funcFrstInt64;
	FPTR_GET_VAL_PTR        funcValPtr;
	FPTR_GET_DOC_INFO       funcDocInfoPtr;
};

struct SOtherFuncs
{
	FPTR_GROUPBY_CLASS funcGpCLsPtr;
	FPTR_WRITE_LOG     funcWriteLogPtr;
	FPTR_GET_DOCS_BY_PK funcGetDocsByPkPtr;
	FPTR_SCATTER_RESULT funcScatterResult;

};


//模块初始化时候传入的搜索数据及获取方法
struct SSearchDataInfo
{
	vector<SFieldInfo>     vFieldInfo;//搜索各字段信息
	vector<void*>          vProfiles;//搜索各字段属性使用
	hash_map<string, int>  hmFieldNameId; //字段名到字段ID的映射
	void*                  pSearcher;//搜索器指针
	void*                  pLogger;//日志指针
	MOD_MAP*               mapMod;//模块表
	SGetValFuncs           sgvf;      //获取搜索字段值函数
	SOtherFuncs            sof; //其他内核功能函数
	
};



//每个KEY在某字段上偏移
struct SFieldOff 
{
	ushort  off;
	ushort  field;
};


//查询语句结构
struct SQueryClause
{
	vector<Term>         vTerms;//非高级搜索query切分后结构
	vector<vector<int> > vTermInFields; //切分后的TERM出现的字段ID列表；
	string             key;   //搜索关键词
	int                firstSortId;//是否含其他非搜索排序字段第一排序字段（如按照价格排序） -1 为无效
	int                secondSortId;//是否含第二非搜索排序字段 -1为无效
	unsigned long long cat;      //是否指定分类 无效时该值为0 编码方式8级，8个CHAR 第0个CHAR为第一级
	bool               bAdvance; //是否是高级搜索 

	vector<int>   vFields4Advance; //高级搜索的字段列表
	vector<string>  vKeys4Advance;//高级搜索各个字段的查询串
	vector<vector<Term> > vvTerms;//高级搜索切分后的TERM

	hash_map<string,string> hmUrlPara;//所有 url参数 

	SQueryClause()
	{
		cat = 0;
		bAdvance = false;
		firstSortId = -1;
		secondSortId = -1;
	}
};


//筛选出的文档匹配信息
struct SMatchElement 
{
	int   id;       //文档id
	short matchType;//匹配类型  enum {EXACT_MATCH, GAP_MATCH, OVER_FIELD_MATCH};

	vector<SFieldOff> vFieldsOff; //每个字段出现的FIELD及偏移量，EXACT_MATCH 和 GAP_MATCH 情況下field id一样；
	vector<Term>      vTerms;     //query分词后的TERM序列

	vector<int>       vFieldLen; //每个TERM对应字段的长度
	vector<int>       vTf;       //每个TERM对应字段的tf信息
	vector<float>     vIdf;      //每个TERM对应字段的IDF信息

	//以下量为默认模块使用，模块实现者可不必关心
	vector<int>       vAllowGap;      //源分词各个TERM允许与前面一个TERM的间隔,如果词间有空格则很大

};

class CModule
{
public:
	virtual ~CModule(){;}//请在派生类中重载清理动态资源，模块可能多次载入

	/*
	*Method:   初始化函数需要具体模块重载
	*Parameter: psdi 搜索数据传递
	*Parameter: strConf 模块各自的配置文件地址
	*return true初始化成功，false初始化失败 
	*/
	virtual bool Init(SSearchDataInfo* psdi, const string& strConf)
	{
		m_vFieldInfo = psdi->vFieldInfo; 
		m_vProfiles = psdi->vProfiles; 
		m_hmFieldNameId = psdi->hmFieldNameId;
		m_mapMods = psdi->mapMod;

		m_funcFrstPtr = psdi->sgvf.funcFrstPtr;
		m_funcFrstInt = psdi->sgvf.funcFrstInt;
		m_funcFrstInt64 = psdi->sgvf.funcFrstInt64;
		m_funcValPtr = psdi->sgvf.funcValPtr;
		m_funcDocInfoPtr = psdi->sgvf.funcDocInfoPtr;
		m_funcGpClassPtr = psdi->sof.funcGpCLsPtr;
		m_funcWriteLogptr = psdi->sof.funcWriteLogPtr;
		m_funcGetDocsByPkPtr = psdi->sof.funcGetDocsByPkPtr;
		m_funcScatterResult = psdi->sof.funcScatterResult;
		m_pSearcher = psdi->pSearcher;
		m_pLogger = psdi->pLogger;

		m_strConf = strConf; 
		size_t s = strConf.find_last_of('/');
		if(s != string::npos)
			m_strModulePath.assign(strConf.c_str(), s+1);
		return true;
	}

	/*
	* Method:    QueryRewrite 查询重写
	* Returns:   void
	* Parameter: hash_map<string, string> & hmParams查询各参数 map形式，可以修改KEY对应的VALUE
	*/
	virtual void QueryRewrite(hash_map<string,string>& hmParams)
	{
		;
	}

	/*
	* Method:    query分类转换到特定处理模块，每个业务有且仅有一个入口模块，可以有多个辅助模块
	* Returns:   void
	* Parameter: SQueryClause & qc 结构化的查询语句
	* return: CModule*  返回的转换到的模块指针
	*/
	virtual CModule* QueryClassify(SQueryClause& qc)
	{
		//通过查询词进行QUERY分类重选模块--例如切换到百货或者图书RANKING，或者百货图书列表
		//一个例子转向默认排序
		/*MOD_MAP::iterator i;
		for(i = m_mapMods->begin();i != m_mapMods->end(); ++i)
		{
			if(i->first == "default_ranking")//调用其他模块的FILL
				i->second.pMod->FillGroupByData(vGpInfo);
		}
		if(i == m_mapMods->end())*/
		return this;
	}

	/*
	* Method:    query分析
	* Returns:   void
	* Parameter: SQueryClause & qc 结构化的查询语句
	* return: IAnalysisData*  返回的query分析数据 用户动态生成，框架负责销毁，生成失败需返回NULL
	*/
	virtual IAnalysisData* QueryAnalyse(SQueryClause& qc){return new IAnalysisData;}


	/*
	* Method:    计算文本搜索权重
	* Returns:   void
	* Parameter: IAnalysisData * pad 返回的query分析数据
	* Parameter: SMatchElement & me每个文档的匹配信息
	* Parameter: SResult & rt 打分结果
	*/
	virtual void ComputeWeight(IAnalysisData* Pad, SMatchElement& me, SResult& rt)
				{rt.nDocId = me.id, rt.nScore = 0;rt.nWeight = 0;}


	/*
	* Method:    计算非文本搜索权重
	* Returns:   void
	* Parameter: vector<SResult> & vRes 搜索打分结果
	* Parameter: IAnalysisData* Pad, 返回的query分析数据
	*/
	virtual void ComputeWeight(IAnalysisData* pad, vector<SResult>& vRes) 
	{
		/*
		for (int i = 0; i < vRes.size(); ++i)
		{
			vRes[i].nWeight = 0;//中间权重
			vRes[i].nScore = 0;//最终分数
		}
		*/
	}


	
	/*
	* Method:    重新剪枝计算，常用于非默认排序
	* Returns:   void
	* Parameter: IAnalysisData* Pad, 返回的query分析数据
	* Parameter: vector<SResult> & vRes 搜索打分结果
	*/
	virtual void ReRanking(vector<SResult>& vRes, IAnalysisData* pad)
	{

	}

	/*
	* Method:    搜索默认排序（如按照其他字段如价格排序不走该接口）
	* Returns:   void
	* Parameter: vector<SResult> & vRes 搜索打分结果
	* Parameter: IAnalysisData* Pad, 返回的query分析数据
	* Parameter: from 用户索取的文档在 vRes中的起始位置
	* Parameter: to 用户索取的文档在 vRes中的终止位置（不含）
	* from to 为用户取第几页，每页多少个的另一种表现形式，如取第五页，每页50个 则from=200,to=250
	*/
	virtual void SortForDefault(vector<SResult>& vRes, int from, int to, IAnalysisData* pad)
	{
		// 给出from to 则可进行部分排序,提高效率

	}


	/*
	* Method:    搜索自定义排序（如按照其他字段如价格排序不走该接口）
	* Returns:   void
	* Parameter: vector<SResult> & vRes 搜索打分结果
	* Parameter: IAnalysisData* Pad, 返回的query分析数据
	* Parameter: from 用户索取的文档在 vRes中的起始位置
	* Parameter: to 用户索取的文档在 vRes中的终止位置（不含）
	* from to 为用户取第几页，每页多少个的另一种表现形式，如取第五页，每页50个 则from=200,to=250
	*/
	virtual void SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad)
	{
		// 给出from to 则可进行部分排序,提高效率

	}

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
							vector<SResult>& vRes, vector<string>& vecRed, string& strRes)
	{

	}





	/*
	*Method:  填充统计信息，将内部以ID统计的量转换为文字 例如CLASS PATH--> CLASS NAME
	*Parameter:vGpInfo 传入数组 字段ID对应-->统计信息
	*/
	virtual void FillGroupByData(GROUP_RESULT& vGpInfo){;}



	/*
	*Method:  设置过滤和汇总顺序
	*Parameter:IAnalysisData* Pad,    返回的query分析数据
	*Parameter:vec 传入默认汇总和过滤顺序，可调整顺序或者增加自定义过滤汇总方法
	*/
	virtual void SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vec)
	{
		;
	}

	/*
	*Method:  自定义过滤函数 不能对vDocIds重新排序，只能删除已有的ID
	*Parameter:IAnalysisData* Pad,    返回的query分析数据
	*Parameter:vDocIds 文档ID序列，过滤后把不相干的就地删除
	*Parameter:fgNode  过滤节点信息
	*/
	virtual void CustomFilt(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& fgNode)
	{

	}

	/*
	*Method:  自定义汇总函数 不能对vDocIds重新排序，只能删除已有的ID
	*Parameter:IAnalysisData* Pad,    返回的query分析数据
	*Parameter:vDocIds 文档ID序列
	*Parameter:fgNode  汇总标识信息
	*Parameter:prGpInfo  pair first 用户可自定义各种负整数，second为汇总信息
	*/
	virtual void CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
		                        pair<int, vector<SGroupByInfo> >& prGpInfo)
	{

	}


    /*
	*Method:  运行于GROUP_BY之前;
	*Parameter: IAnalysisData* Pad,    返回的query分析数据
	*Parameter: vector<SResult> & vRes 搜索打分结果
	*Parameter: vector<PSS> & vGpName  分组汇总的字段名和信息值
	*Parameter: GROUP_RESULT    vGpRes 传入数组 字段ID对应-->统计信息
	*/
	virtual void BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, vector<PSS>& vGpName, GROUP_RESULT& vGpRes){}





protected:

	inline void* FindProfileByFieldId(int i)
	{
		if (i >= 0 && i < (int)m_vProfiles.size())
			return m_vProfiles[i];
		return NULL;
	}

	inline void* FindProfileByName(const char* name)
	{
		hash_map<string,int>::iterator i = m_hmFieldNameId.find(name);
		if (i != m_hmFieldNameId.end())
			return m_vProfiles[i->second];
		return NULL;
	}

	inline int GetFieldId(const char* name)
	{
		hash_map<string,int>::iterator i = m_hmFieldNameId.find(name);
		if (i != m_hmFieldNameId.end())
			return i->second;
		return -1;
	}
	
protected:
	//searcher for get doc data
	string m_strConf;                 //配置文件
	string m_strModulePath;           //模块所在路径

	vector<SFieldInfo> m_vFieldInfo;  //搜索各字段信息    和m_vProfiles 一对一
	vector<void*>      m_vProfiles;   //属性用于字段值获取
	hash_map<string, int> m_hmFieldNameId; //字段名到字段ID的映射

	MOD_MAP*  m_mapMods;//模块映射表 用于调用其他模块使用
	void*     m_pSearcher;//搜索器指针
	void*     m_pLogger; //日志指针


	FPTR_GET_FRST_NUM_PTR   m_funcFrstPtr; //取字段第一个值指针
	FPTR_GET_FRST_INT_VAL   m_funcFrstInt; //取字段第一个整数值(如果是单值4字节)
	FPTR_GET_FRST_INT64_VAL m_funcFrstInt64; //取字段第一个整数值64bit
	FPTR_GET_VAL_PTR        m_funcValPtr;  //取字段值指针，最通用方法取单值多值都可用。

	FPTR_GROUPBY_CLASS      m_funcGpClassPtr; //统计分类函数指针
	FPTR_GET_DOC_INFO       m_funcDocInfoPtr;//在展示结果时候取文档信息
	FPTR_WRITE_LOG          m_funcWriteLogptr;//日志函数指针
	FPTR_GET_DOCS_BY_PK     m_funcGetDocsByPkPtr;//通过主键获取文档ID
	FPTR_SCATTER_RESULT     m_funcScatterResult; //打散函数



};
#endif	
