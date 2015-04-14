#ifndef GlobalDef_H
#define GlobalDef_H
#include <string>
#include <vector>
#include <algorithm>
using std::string;
using std::vector;
using std::pair;
using std::sort;
#define TQ   "q"
#define PG   "pg"
#define PS   "ps"
#define NS   "ns" //不排序
#define FS   "fs"
#define FA   "fa"
#define SS   "ss"
#define US   "us"
#define SA   "sa"
#define SF   "sf"  //show fields
#define SC   "sc"  //scatter field
#define SU   "su"  //scatter unit
#define GP   "gp"
#define UM   "um" //use module
#define ST   "st" // search Type
#define FT   "ft" // show format xml html json
#define DB_HOME "./"
#define INDEX "index/"
#define DATA  "data/"
#define EX    "ex"

#define DB_IDX_PAGESIZE 1024*64
#define DB_DTL_PAGESIZE 1024*8
#define DEFAULT_GET_CNT 20
#define DEFAULT_GP_CNT  1000



#define  FIELD_INDEX 'F'
#define  TEXT_INDEX 'T'
#define  OVER_FILED_INDEX 'O'
#define  STRING     'S'
#define  NUMBER     'N'


#define GROUP_SEARCH  1
#define DETAIL_SEARCH 0

#define SIGN_LINE_ENCODE 1

#define DOT_TF      ".tf"
#define DOT_IDF     ".idf"
#define DOT_LEN     ".len"
#define INDEX_DIC   ".idx.dic"
#define INDEX_IDX   ".idx.idx"
#define INDEX_IVT   ".idx.ivt"
#define PROFILE_IDX ".pfl.idx"
#define PROFILE_DAT ".pfl.dat"
#define PROFILE_DAT_STR ".pfl.dat.str"
#define DETAIL_IDX  "dtl.idx"
#define DETAIL_DAT  "dtl.dat"
#define SEGMENT_RPATH "segment"
#define DOC_MARK     "_doc_mark"
#define FULL_TM      "_full_timestamp"
#define DATE_FORMAT   "%Y-%m-%d %H:%M:%S"





#define BUF_BUILD_INVERT_CNT  1024*1024*64
//#define BUF_BUILD_INVERT_CNT  512
#define BUF_BUILD_DETAIL_SIZE  1024*1024*16
#define BUF_MAX_DOC_LEN       1024*31



#define TOP_FOR_OTHER_SORT        5000 // 有文本搜索情况下加其他排序条件取的TOP个数
#define MAX_OPIMIZE_REVIEW_CNT    5000
#define MAX_EXACT_SCORE_DOCS      1000000
#define MAX_FUZZY_SCORE_DOCS      10000
#define MAX_UNION_SCORE_DOCS      50000
#define MAX_OVER_FIELD_MATCH       1000
#define MIN_EXACT_MATCH     40
#define MIN_FIELD_MATCH     100
#define MIN_FULL_MATCH     0

#define MAX_QUERY_KEY_LEN  16000 
#define MAX_SEG_KEY_LEN    64
#define IMPORTANT_FIELD_LIMIT 50



const char* const SEG_PROFILE_SPLITER[] ={",", 
                                    ",:-|",
                                    ",:-|",
				    ",:-|",
				    ",",
				    ",:-|"
                                   };

enum {SEARCH_PAGE=0,PRODUCT_PAGE,LIST_PAGE};
enum {COMMON=0,CAT_FIELD,BIT_FILED,PARA_FILED,DATE_TIME_FIELD,INVALID_STYPE};
enum {XML_TYPE = 0, HTML_TYPE, JSON_TYPE};


typedef unsigned short ushort;
typedef unsigned char  uchar;
typedef unsigned int   uint;


#ifdef _WIN32
#define atoll _atoi64
#endif

struct SGPFilter 
{
	int nPflId;
	short nNot;
	short nCatlimit;
	long long nMin;
	long long nMax;
};

struct SFGNode //filte groupby node
{
	enum {FILT_TYPE, GROUPBY_TYPE};
	short nCid;//  filt or group by container index id, -1 means custom
	short nField;//field id;
	short nType;// filt or group by
	short nId;//   sequence id
	SGPFilter sgpf; //group by filter 
};

struct SResult 
{
	int nDocId;
	int nWeight;
	int nScore;

	bool operator<(const SResult& r)const 
	{
		return this->nScore<r.nScore||
			this->nScore==r.nScore&&this->nDocId<r.nDocId;;
	}

	bool operator>(const SResult& r)const 
	{
		return this->nScore>r.nScore||
	this->nScore==r.nScore&&this->nDocId>r.nDocId;
	}
#ifdef RANKING_DEBUG
	string strRankInfo;
#endif
};

template<class T>
struct PARA_KV
{
	T name_id;
	T value_id;
	bool operator<(const PARA_KV& r)const
	{
		return this->name_id<r.name_id||
			   this->name_id==r.name_id&&this->value_id<r.value_id;
	}
	bool operator>(const PARA_KV& r)const
	{
		return this->name_id>r.name_id||
			   this->name_id==r.name_id&&this->value_id>r.value_id;
	}
};

template<class T>
inline void SortPara(vector<T>& vec)
{
	vec.resize(vec.size()/2*2);
	int prCnt=vec.size()/2;
	if (prCnt>0)
	{
		PARA_KV<T>* p1 = (PARA_KV<T>*)&(vec[0]);
		PARA_KV<T>* p2 = p1+prCnt;
		sort(p1,p2);
	}
}


struct SIvtNode
{
	int    nDocId;
	ushort nOffset;
#ifdef EX_VERSION
	uchar  isCoreWord;
	uchar  align;
#else
	uchar  uField;
	uchar  uLevel;
#endif

	bool operator<(const SIvtNode& r)const
	{
		return this->nDocId<r.nDocId||
			   this->nDocId==r.nDocId&&this->nOffset<r.nOffset;
	}
};

struct CControl
{
	CControl()
	{
		usRetCnt=5;
		usRetOff=1;
		nFSortId=-1;
		nFSAsc=0;
		nSSortId=-1;
		nSSAsc=0;
		nSortKind=0;
		nF=0;
		nT=0;
		nSearchType=0;
		noSort=0;
		nPageType=SEARCH_PAGE;
		nScatterUnit = 1;
	}

	ushort usRetCnt; //每页返回数量
	ushort usRetOff; //第几页
	int nFSortId:16;  //第一排序字段ID
	int nSSortId:16;  //第二排序字段ID
	int nFSAsc:8;    //第一排序是否升序
	int nSSAsc:8;    //第二排序是否升序
	int nSortKind:8; //无用暂时
	int nSearchType:8;//无用暂时
	int nF;          //用户取数据文档数组开始下标 和翻页对应
	int nT;          //用户取数据文档数组终止下标
	int noSort;      //是否不排序 0,1
	int nPageType;   //无用暂时
	int nScatterUnit; //打散单位
	string strUseModule;//直接调用模块名称
	string strCustomSort;//自定义排序字串
	string strScatterField;//自定义排序字串
	vector<int> vecShowFieldIds; //需要展示的字段ID序列 升序
};


struct SGroupByInfo 
{
	char               bufName[52];//名称
	char               bufId2str[28];//数字ID的字符串存储 除了基本数字类型，分类存路径
	int                nCnt;//数量
	unsigned long long nGid;//统计中使用的名称到ID的映射
};

typedef vector<pair<int, vector<SGroupByInfo> > > GROUP_RESULT;

struct SFieldInfo
{
	//enum{NUMBER=0,STRING};
	//enum{NON=0,TEXT,FIELD, };
	SFieldInfo()
	{
		chIndexType='0';
		bProfile=false;
		bShow=false;
		bPk=false;
		nSpecialType=0;
		bUseHash=true;
		chDataType=NUMBER;
		nGpOptimize = 0;
		nKeyPow=-1;
		nKeyCnt=0;
		nBaseWeight=0;
	};


	bool  bProfile;
	bool  bShow;
	bool  bUseHash;
	bool  bPk;//primary key
	char  chIndexType;//
	char  chDataType;
	short nGpOptimize;
	short nKeyPow;
	short nKeyCnt;
	short nBaseWeight;
	short nFieldId;
	short nSpecialType;
	std::string strFieldName;
};
#endif
