#ifndef __PACK_DEFINE_H__
#define __PACK_DEFINE_H__
#include <string>
#include "GlobalDef.h"
#include <vector>
using namespace std;

// attr file name
const char* const ATTR_DATA_IDX  = "attr_data.idx";
const char* const ATTR_DATA_DAT  = "attr_data.dat";
const char* const ATTR_DATA_RDX  = "attr_data.rdx";
const char* const BRAND_DATA_RDX = "brand_data.rdx" ;
const char* const SHOP_DATA_RDX  = "shop_data.rdx";
const char* const CATEGORY_PATH  = "cls_path_to_name.rdx";
const char* const CONFIG_FILE    = "show_result.conf";
const char* const CITY_CODE_FILE = "city_code";

const char* const SHOP_INDEX_FILE_RDX     = "shop_index_file.rdx";
const char* const SHOP_DATA_FILE_DAT      = "shop_data_file.dat";
const char* const PRODUCT_SUBCAT_DATA_DAT = "product_subcat_data.dat";
const char* const PRODUCT_INDEX_FILE_RDX  = "product_index_file.rdx";
const char* const CATID_TO_PATH			  = "category_to_path.rdx";
const char* const PATH_TO_CATID			  = "path_to_category.rdx"; 
const char* const BRAND_TO_NAME			  ="brdIdToName.rdx";

//attr name max length limit
const int MAX_SUB_ATTR_NAME_LEG			= 100;
const int MAX_ATTR_NAME_LEN				= 100;
const int MAX_BRAND_NAME_LEN			= 100;
const int MAX_SHOP_NAME_LEN				= 100;
const int MAX_CAT_NAME_LEN				= 100;
const int BUCKET_NUM					= 14;
const int MAX_CATE_PATH_LEN				= 24;
const int SHOP_INFO_LIMIT				= 5;  
const int ALL_TAG_SIZE					= 2;
const int HALF_TAG_SIZE					= 1;
const int MAX_TEXT_LENGTH				= 190;
const int MIN_PRODUCT_LIMIT				= 5;
const char* const PRICE_SPAN			= "dd_sale_price";
const char* const DISCOUNT				= "discount";
const char* const CATE_PATH				= "cat_paths";
const char* const EBOOK_ID				= "ebook_product_id";
const char* const PRODUCT_ID			= "product_id";
const char* const IS_PUBLICATION		="is_publication";  
const char* const HAS_EBOOK				="is_has_ebook";
const char* const PAPER_PRODUCT_ID		="paper_book_product_id";
const char* const FILT_CATE_PATH		="-cat_paths";
const char* const DD_SELL				="is_dd_sell";
const char* const PROMO_PRICE			="promo_saleprice";
const char* const FILT_DD_SELL			="-is_dd_sell";
const char* const DD_PRICE_FILT			="-dd_sale_price";
const int MAX_CITY_NUM					= 1000;
const int MAX_CATE_LEVEL				= 6;
const int MALL_CAT_MAX_LEN              = sizeof("01.01.01.01.01")-1;
const int PUB_CAT_MAX_LEN               = sizeof("01.01.01.01")-1;
const char* const PROMO_TYPE			="promotion_type";
const char* const FILT_PROMO_TYPE			="-promotion_type";
const char*	const PROMO_START_DATE		="promo_start_date";
const char* const PROMO_END_DATE		="promo_end_date";
const char*	const CAT_PROMO_START_DATE	="cat_promo_start_date";
const char* const CAT_PROMO_END_DATE	="cat_promo_end_date";
const char* const CAT_PROMO_TYPE		="cat_promotion_type";
const char* const SPLIT					="|"; 
const char* const PRICE_SPAN_MAX		="dd_sale_price_max";
const char* const PRODUCT_NAME			= "product_name";
const char* const IS_HAS_EBOOK			="is_has_ebook";
const char* const EBOOK_PRODUCT_ID		="ebook_product_id";
const char* const STOCK_STATUS			="stock_status";
const char* const DISPLAY_STATUS_FILT	="_display_status";
const char* const DISPLAY_STATUS		="display_status";
const int EBOOK_CUSTOM_GROUP			= 8;
const int STOCK_STATUS_FILT				= 7;
const int PRICE_CUSTOM_GROUP			= 6;
const int SALE_PRICE_FILT				= 5;
const int    PROMO_FILT					= 4;		
const int INVALID_GPORFILT_TYPE			= 10;
const int INNERCAT_CUSTOM_FILT			= 3;
const int EBOOK_CUSTOM_FILT				= 2;  
const int EBOOK_CUSTOM_FILT_BEF_GP		= 9;  
const int CAT_CUSTOM_GROUP				= 1;
const int ADD_GPORFILT_TYPE				= 999;  
const int PROD_CUSTOM_RES				= 11;
const char* const ATTRIB				= "attrib";
const char* const SEQUENCE				= "sequence";
const char* const EBOOK_NUM_GP			= "ebook_gp";
const char* const EBOOK_NUM_FILT		= "ebook_filt";
const char* const WEB					="WEB";
const char* const BRAND_ID				= "brand_id";
const char* const PROMOTION_FILT		= "promotion_filt";
const char* const PROD_PROMOTION_FILT	="product_promotion_filt";

//category info
#define  ATTR_CATEGORY   "cat_paths"
#define  CATE_PATH_LEN   23
#define  ATTR_SHOW_LEVEL 3
#define  ATT_INFO        "attrib"
#define  SHOP_INFO       "shop_id"
#define  BRAND_INFO      "brand_id"
#define  SPECIALSALE     "specialsale"
#define  FILT_EBOOK      "ebook_id"  
#define  INNER_CAT	 "innercat"
#define  SHOP_ID	 "-shop_id"

//search type
const char*  const PUB_SEARCH		= "pub";
const char*  const MALL_SEARCH		= "mall";
const char*  const FULL_SEARCH		= "full";
const char*  const EBOOK_SEARCH		= "ebook";
const char*  const EBOOK_SHOW		= "ebook_show";
const char*  const PUB_RED_FIELD	= "pub_red_field"; 
const char*  const MALL_RED_FIELD	= "mall_red_field";
const char*  const SHOP_SEARCH 		= "shop";
const char*  const CITY				= "-city_stock_status";

//special price info
#define  SRICESPLEN  4
#define  DISCOUNTLEN 3
#define  MARKLENGTH  4

const int g_PriceSpan[SRICESPLEN][2]={{0,5},{5,10},{10,15},{15, 0}};//价格区间 
const int g_Discount[DISCOUNTLEN][2]={{0,30},{30,50},{50, 0}};//折扣 

//attr info
struct KAttrInfo
{
	int			attrib_id;	
	int			attrib_priority;
	long long   catid;
	char		attr_name[MAX_ATTR_NAME_LEN];
};

//sub attr info
struct KSubAttrInfo
{
	int   sub_attrib_id;
	int   sub_attrib_priority;
	char  sub_attrib_name[MAX_SUB_ATTR_NAME_LEG];
};

//attr index info
struct KAttrIndex
{
	long long offset;
	int		  cnt;
};

//file head info
struct KAttrHead
{
	int iAttrSize;
	int iSubAttrSize;
	int iIndexSize;
};

//brand info 
struct KBrandInfo
{
	char brand_name[MAX_BRAND_NAME_LEN];
	int  priority;
};

//brand key
struct KBrandKey
{
	int brand_id;
	int cat_id;
};

// pack brand info
struct KResBrand
{
	char brand_name[MAX_BRAND_NAME_LEN];
	int  priority;
	int  brand_id;
	int  nCnt;
	
	bool operator <(const KResBrand& a)const
	{
		return this->priority > a.priority||
				this->priority == a.priority && this->nCnt>a.nCnt ;
	}
};

//shop info
struct KShopInfo
{
	char shop_name[MAX_SHOP_NAME_LEN];
};

//category info
struct KCategoryInfo
{
	char cat_name[MAX_CAT_NAME_LEN];
};

struct cls_buf
{
	char chCLs[56];
};

struct KNameToCnt
{
	int nCnt;
	int Id;
};

//attr pack xml info
struct KAttrPack
{
	KAttrInfo    kAttr;
	KSubAttrInfo kSubAttr;
	int		  cnt;
	int		  nTotalCnt;
	bool operator < (const KAttrPack &a) const 
	{

		if(this->kAttr.attrib_priority!=a.kAttr.attrib_priority)
		{
			return (this->kAttr.attrib_priority<a.kAttr.attrib_priority);
		}
		/*if(this->kAttr.attrib_id!=a.kAttr.attrib_id)
		{
			return (this->kAttr.attrib_id<a.kAttr.attrib_id);	
		}*/
		else if(this->nTotalCnt!=a.nTotalCnt)
		{
			return (this->nTotalCnt>a.nTotalCnt);	
		}
		else if(this->kAttr.attrib_id!=a.kAttr.attrib_id)
		{
			return (this->kAttr.attrib_id<a.kAttr.attrib_id);	
		}
		else if(this->kSubAttr.sub_attrib_priority!=a.kSubAttr.sub_attrib_priority)
		{
			return (this->kSubAttr.sub_attrib_priority<a.kSubAttr.sub_attrib_priority); 
		}
		else if(this->cnt!=a.cnt)
		{
			return (this->cnt>a.cnt);
		}
		else if(this->kSubAttr.sub_attrib_id!=a.kSubAttr.sub_attrib_id)
		{
			return (this->kSubAttr.sub_attrib_id<a.kSubAttr.sub_attrib_id);
		}
		else
		{
			return false;
		}

	}

};

static inline bool FILT_EBOOK_ID(const int & di)
{
	return di == -1;	
}

static inline bool SORT_BY_CNT(const SGroupByInfo &l,const SGroupByInfo &r)
{
	return l.nCnt>r.nCnt;	
}

struct KShopIndex
{
        int  strOff;
        int  endOff;
};

//shop_catagory data                                                          
struct KCatInfo
{
        int  shop_id;
        char catid[8];
        char name[MAX_CAT_NAME_LEN];
};

//product_subcat data                                                                
struct KProductInfo
{
        int     product_id;
        char    subcat[8];
        int     priority;
};

//product index
struct KProIndex
{
        int strOff;
        int endOff;
};

//punctuation struct
struct KPunc
{
	int    interSize;
	int    pos;

	bool operator < (const KPunc &m)const 
	{
		return pos< m.pos;
	}

	bool operator == (const KPunc &m)const 
	{
		if(pos == m.pos)
			return true;
		else
			return false;
	}

};

//window score
struct KWind
{
	int start;
	int end;
	int score;
	int wordKind;
};

//category id to path
struct KCatToPath
{
	char path[MAX_CATE_PATH_LEN];
};

//path to category
struct KPathToCat
{
	char catid[52];
	int  guan_id;
};

//statistic data struct
struct KStatNum
{
	int iPapNum;	
	int	iEbookNum;
	int iEbookFlt;
};

// sequence data struct
struct KSequence
{
	//char cat_name[32];
	int	 nFld; 
	int	 nType;
	int  nId;
	int  nCid;
};

static bool IsHalf(unsigned char c)
{
	if (c < 0x80)
	{
		return true;
	}
	else
	{
		return false;
	}
}


static void SplitToVecEx(const char* pSrc, vector<string> &vec,const char* pchSplit)
{

	vector<char> vTmp(strlen(pSrc)+1);
	memcpy(&vTmp[0],pSrc,vTmp.size());

	//pSrc=&vTmp[0];
	char* pTmp = &vTmp[0];
	char* pLast = NULL;
	//char* pchCurWord=strtok(pTmp,pchSplit);
	char* pchCurWord=strtok_r(pTmp,pchSplit,&pLast);
	while(pchCurWord&&*pchCurWord)
	{
		vec.push_back(pchCurWord);
		//pchCurWord = strtok(NULL, pchSplit);
		pchCurWord = strtok_r(NULL, pchSplit,&pLast);
	}
}


enum {GBKWORD,GBKSYMBOL,GBKUNCOMMON,HALFGBK,NONGBK};

static int IsGBKWord(unsigned char ch1,unsigned char ch2)
{
	if ((ch1 >= 129)&&(ch2 <= 254))
	{
		if (((ch2 >= 64)&&(ch2 < 127)) ||((ch2 > 127)&&(ch2 <= 254)))
		{
			int posit = (ch1 - 129) * 190 + (ch2 - 64) - (ch2/128);
			
			// gbk/3
			if(ch1>=0x81&&ch1<=0xA0&&ch2>=0x40&&ch2<=0xFE)
				return GBKUNCOMMON;
			
			// gbk/4
			if(ch1>=0xAA&&ch1<=0xFE&&ch2>=0x40&&ch2<=0xA0)
				return GBKUNCOMMON;
			
			if (posit>=0 && posit<=6079)
				return GBKWORD;
			
			else if(posit>=7790 && posit<=23845)
			{
				return GBKWORD;
			}
			else
				return GBKSYMBOL;
		}
		else
			return HALFGBK;
	}
	else	
		return NONGBK;
}


class CCustPriceFilt
{
	public:
		enum {MIN_VAL=0,MAX_VAL=2147483647};	
	
	public:	
		
		// parse price interval	
		void SignalParse(string &strKey)
		{
	
			if (strKey[strKey.length()-1]!=',')
			{   
				strKey.push_back(',');
			}   
	
			vector<char> vec(&strKey[0],&strKey[strKey.length()-1]+2);
			char* p=&vec[0];
			
			m_iMin = MIN_VAL;
			m_iMax = MAX_VAL;
			int min = 0;
			int max = 0;
			
			char* pVal = p;
			char chFlag = '\0';
			char* pMin = p;
			char* pMax = NULL;
			
			while(*p)
			{
				if(*p==',')
				{
					*p=0;	
					if(chFlag =='\0')
					{
						m_vSnglVal.push_back(atoi(pVal));	
					}
					else
					{
						if(pMax==p)
							max = m_iMax;
						else
							max = atoi(pMax);
						
						m_vPrVal.push_back(make_pair(min,max));		
						chFlag ='\0';	
					}
					pVal = p+1;
					pMin = pVal;
				}
				else if(*p=='~')
				{
					*p=0;	
					chFlag ='~';
					if(pMin==p)
						min = m_iMin;
					else
						min = atoi(pMin);
					pMax=p+1;
				}
				p++;
			}
		}
	public:
		int m_iMin;
		int m_iMax;
		vector< pair<int,int> > m_vPrVal;
		vector< int > m_vSnglVal;
};
#endif
