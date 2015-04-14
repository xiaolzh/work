#ifndef _QUERY_RESULT_PACKER_H_
#define _QUERY_RESULT_PACKER_H_
#include <iostream>
#include "GlobalDef.h"
#include <string>
#include <vector>
#include "nokey_static_hash.h"
#include "MMapFile.h"
#include "PackDefine.h"
#include "price_span.h"
#include "Class.h"
#include "Module.h"
#include "XmlHttp.h"
#include "UtilDef.h"
#include "TaDicW.h"
#include "UtilTemplateFunc.h"
#include <unistd.h>
#include <stdio.h>
#include "AbstractInfoShow.h" 
#include "ResourceUtil.h"
#include "hash_wrap.h"
#include "BitMap.h"
#include <time.h>
#include "TimeUtil.h"
#include "map_number_nolock.h"
using namespace std;
/*class CKeySrchAnalysis:public IAnalysisData
{
	public:
		    CKeySrchAnalysis(){}
			    virtual ~CKeySrchAnalysis(){;}
	//do it .
};*/

class CQueryResultPack:public CModule 
{
	public:
		CQueryResultPack();
		~CQueryResultPack();
		
		//search show main function
		void ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
		                 vector<SResult>& vRes, vector<string>& vecRed, string& strRes);
		
		//load show data
		bool Init(SSearchDataInfo* psdi, const string& strConf);
		
		//load attr data
		bool LoadAttrData();
		//load shop data
        	bool LoadShopData();
		
		//special book price 
		void ShowSpecialPriceSpan(vector<SResult>& vRes,string &strRes);

		//statistic price span
		//void StatisticPriceSpan(vector<int>& vRes,string &str);	
		void StatisticPriceSpan(vector<int>& vRes,IAnalysisData* pad);	
		
		//show attr info
		void ShowAttrInfo(vector<SGroupByInfo> &sg,string& str,IAnalysisData* pad);
		//show shop_data info
        	void ShowShopInfo(string& shop_id,string& str);

		//show brand info
		void ShowBrandInfo(vector<SGroupByInfo> &sg,string& str,IAnalysisData* pad);

		//show shop info
		void ShowShopGroupInfo(vector< SGroupByInfo > &vGpRes,string &str,vector<SResult>& vRes);

		//get attr info	
		void GetAttrData(SGroupByInfo &sg ,vector< KAttrPack >& vPack,IAnalysisData* pad,
				hash_map< int,int > &hCnt);	
		
		//get shop index
        	void GetShopData(string& shop_id ,KShopIndex& kShopIndex);
		//get product index
		void GetProIndex(string& catid,KProIndex& kProIndex);		

		//combine pack bh and pub xml
		void CombineXmlInfo(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
		                 vector<SResult>& vRes, vector<string>& vecRed, string& strRes);
		//fill group info
		void FillGroupData(vector<pair<int, vector<SGroupByInfo> > >& vGpInfo);

		//custom group	
		void CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
				        pair<int, vector<SGroupByInfo> >& prGpInfo);

		//set custom group and filt
		
		void SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vFgn);

		//custom filt
		void CustomFilt(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& fgNode);

		int GetGpIndex(GROUP_RESULT &vGpRes,const char* gp);

		//query rewrite	
		void QueryRewrite(hash_map<string,string>& hmParams);

		//load config file
		void LoadConfig();

		//judge red field
		bool IsRedField(string& fld,int iDoc);
		
		//show full category info
		void ShowFullCatInfo(vector<SGroupByInfo> &sg,string& str);

		//set mall and pub show field bit map
		void SetShowBitMap(CBitMap* pBit,vector< string > &vec); 
		
		//init  global val	
		bool InitGlobalVal();
		
		//error code 
		bool ErrorMessage(IAnalysisData* pad,GROUP_RESULT &vGpRes,string& strRes);
		
		//pack category info
		inline void PackCatInfo(string& s,SGroupByInfo& sg);

		//show pub or mall category info	
		void ShowMallOrPubCatInfo(vector<SGroupByInfo> &sg,string& str,
				        IAnalysisData* pad,string& sPath);
		
		//show dangdang product num and shop product num	
		void ShowDangDangAndShopNum(vector<SGroupByInfo> &vGpRes,
				        string& str,vector<SResult>& vRes);

		//show statistic data 
		void ShowStatisticData(GROUP_RESULT &vGpRes,string& str, IAnalysisData* pad,
				        CControl& ctrl,vector<SResult>& vRes);

		void  SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad);

		void GetClassCode(u64 &clsID,string &code);
		
		void GetCode(string &cat_path,string& code);
		                  
		bool JudgeIfPromo(int promo_type,int iDocId,string name);
		
		void GetRealPrice(int& iPrice,int iDocId,void* profile);

		void BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, 
				vector<PSS>& vGpName, GROUP_RESULT& vGpRes);
		
		//IAnalysisData* QueryAnalyse(SQueryClause& qc);
		
		void SetSequence(int iSrc,vector<SFGNode>& vFgn,int iDes);
		void InitHashSet(map_number_nolock<int>& setPap,vector<int>& vDocIds);
	private:
		
		static_hash_map< u64,u64 >		m_AttrInfoHash;
		static_hash_map< u64,KBrandInfo >  m_BrandInfoHash;
		static_hash_map< u64,KBrandInfo >  m_BrdToName;
		static_hash_map< u64,KShopInfo >  m_ShopInfoHash;
		static_hash_map< u64,cls_buf >   m_CatInfoHash;     
		static_hash_map< string,KCatToPath > m_CatToPathHash;
		static_hash_map< string,KPathToCat > m_PathToCatHash;
		
		//shop info
		static_hash_map< u64,KShopIndex > m_ShopIndexHash;
		//product index
		static_hash_map< u64,KProIndex > m_ProIndexHash;		

		CMMapFile m_idxMPFile;
		CMMapFile m_datMPFile;
		
		char* m_pIdxBeg;
		char* m_pDatBeg;
		
		FILE* m_idxFp;
		FILE* m_datFp;
		vector< char > m_vecIdxData;
		vector< char > m_vecDatData;

		vector <KCatInfo >      m_vecShopData;
        vector <KProductInfo>   m_vecProductData;

		hash_map< string,string > m_mapShowField;
		vector< string > m_vecShowEbook;	
		vector< string > m_vecAddShowEbook;
		vector< string > m_vecAddEbook;

		vector< string > m_vecPubRed;
		vector< string > m_vecBhRed;
		vector< string > m_vecShow;

		vector< string > m_vecPubField;
		vector< string > m_vecMallField;
		vector< void* >  m_vecProfile;

		CBitMap *m_pMallBit;
		CBitMap *m_pPubBit;

		void* m_PubProfile;
		void* m_HasEbookProfile;
		void* m_EbookIdProfile;
		void* m_PriceProfile;
		void* m_PurPriceProfile;
		void* m_DiscountProfile;
		void* m_PapProfile;
		void* m_PromoProfile;	
		void* m_PromoStartProfile;
		void* m_PromoEndProfile;
		void* m_PromoTypeProfile;	
		void* m_CatPromoStartProfile;
		void* m_CatPromoEndProfile;
		void* m_CatPromoTypeProfile;	
		void* m_PriceMaxProfile;
		void* m_ProdProfile;
		void* m_StockProfile;
		void* m_DisplayProfile;
		void* m_ProdFiltProfile;
		void* m_PromoFiltProfile;
        void* m_ProductBit;
		char m_sCityCode[MAX_CITY_NUM];
		u64 m_clsID;
		vector< KSequence > m_vecSeq;

};
#endif
