/******************************************************************************
 **author:		sunqian
 **
 **date	:		2012.05.28
 **
 **description:	custom filt ,group and price sort	
 *******************************************************************************/

#include "QueryResultPacker.h"

void CQueryResultPack::CustomGroupBy(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& gfNode,
		pair<int, vector<SGroupByInfo> >& prGpInfo)
{
	int iTime = 0;
	#ifdef DEBUG
		TimeUtil t_cust_gp;
	#endif
	int ebook_product_id = 0;
	int iEbookNum = 0;
	int iPapNum = 0;
	char chBuf[52];
	if (gfNode.nId==CAT_CUSTOM_GROUP)
	{
		m_funcGpClassPtr(m_vProfiles[gfNode.nField],vDocIds,1000,prGpInfo.second,false, 5*16+8);
		prGpInfo.first=gfNode.nField;
	}

	if(gfNode.nId==PRICE_CUSTOM_GROUP)
	{
		#ifdef PRICE
			int iTime = 0;
			TimeUtil t_st;
		#endif

		StatisticPriceSpan(vDocIds,pad);

		#ifdef PRICE
		iTime = t_st.getPassedTime();
		cout<<"BeforeGroupBy cost: "<<iTime<<endl;
		#endif
	}
	if(gfNode.nId==EBOOK_CUSTOM_GROUP)
	{
		#ifdef DEBUG 
			TimeUtil t_ebook;
		#endif
		for(int i = 0;i<vDocIds.size();i++)
		{
			ebook_product_id = m_funcFrstInt64(m_ProdProfile,vDocIds[i]);	
			if( ebook_product_id>=1900000000 && ebook_product_id<=2000000000 )
				//1.static ebook number
				iEbookNum++;
			else if(m_funcFrstInt(m_PubProfile,vDocIds[i])==1)
				//2.static paper number
				iPapNum++;
			
		}
		sprintf(chBuf,"%d,%d",iEbookNum,iPapNum);
		if(pad->m_strReserve=="")   
		{
			pad->m_strReserve=EBOOK_NUM_GP;
		}
		else
		{
			pad->m_strReserve+=";";
			pad->m_strReserve+=EBOOK_NUM_GP;
		}
	
		pad->m_strReserve+=":";	
		pad->m_strReserve+=chBuf;

	#ifdef DEBUG 
	iTime = t_ebook.getPassedTime();
	cout<<"ebookGroupBy cost: "<<iTime<<endl;
	#endif
	}
	#ifdef DEBUG 
	iTime = t_cust_gp.getPassedTime();
	cout<<"CustGroupBy cost: "<<iTime<<endl;
	#endif

}

void  CQueryResultPack::SortForCustom(vector<SResult>& vRes, int from, int to, IAnalysisData* pad)
{

	string sCust = pad->m_hmUrlPara[US];
	int iPromoType = 0;

	if (m_PriceProfile && m_PromoProfile)
	{
		if (sCust == "xprice_desc" || sCust == "xprice_asc")
		{
			int dd_price = 0;
			//int	promo_price = 0;
			bool bAsc = ( sCust == "xprice_asc");
			for (int i = 0; i < vRes.size(); ++i)
			{
				GetRealPrice(dd_price,vRes[i].nDocId,m_PriceProfile);	
				vRes[i].nScore = dd_price;
				if (bAsc)
					vRes[i].nScore = -vRes[i].nScore;
			}
			SortRange(vRes ,from, to);
		}
	}
}

void CQueryResultPack::QueryRewrite(hash_map<string,string>& hmParams)
{
	
	hash_map<string,string>::iterator it = hmParams.find(SF);

	string rs;
	if(it==hmParams.end())
	{
		//get config file data  
		if((it = hmParams.find(ST))!=hmParams.end())
		{
			rs = m_mapShowField[it->second];

			hmParams.insert(make_pair(SF,rs));
		}
	}

	//query rewrite city stock

	hash_map<string,string>::iterator itStock = hmParams.find(CITY);
	char city[10];	
	if(itStock!= hmParams.end())
	{
		sprintf(city,"%d",m_sCityCode[atoi(itStock->second.c_str())]);	
		itStock->second = city;
	}
	
	//query rewrite display_status
	/*hash_map<string,string>::iterator itDP = hmParams.find(DISPLAY_STATUS_FILT);	
	sprintf(city,"%d",3);	
	if(itDP !=hmParams.end())
	{
		itDP->second = city;	
	}
	else
	{
		rs = city;
		hmParams.insert(make_pair(DISPLAY_STATUS_FILT,rs));
	}*/
	//query rewrite category_path (st = mall)
	hash_map<string,string>::iterator itSt = hmParams.find(ST);
	hash_map<string,string>::iterator itCat;

	if(itSt!= hmParams.end() && strcmp(itSt->second.c_str(),MALL_SEARCH)==0)
	{
		if((itCat=hmParams.find(CATE_PATH))!=hmParams.end()||
				(itCat=hmParams.find(FILT_CATE_PATH))!=hmParams.end())
		{
			rs = itCat->second;
			if((itCat->second=m_CatToPathHash[rs].path)=="")
			{
				itCat->second="99.99.99.99";	
				SESSION_LOG(SL_WARN,"query rewrite wrong!!");
			}
		}
	}

	if((itCat=hmParams.find(CATE_PATH))!=hmParams.end()||
			(itCat=hmParams.find(FILT_CATE_PATH))!=hmParams.end())
	{
		rs = itCat->second;
		if(itSt!= hmParams.end() && strcmp(itSt->second.c_str(),MALL_SEARCH)==0)	
		{
			if(rs.length() > MALL_CAT_MAX_LEN)
				rs.resize(MALL_CAT_MAX_LEN);
		}
		else if(itSt!= hmParams.end() && strcmp(itSt->second.c_str(),PUB_SEARCH)==0)
		{
			if(rs.length() > PUB_CAT_MAX_LEN)
				rs.resize(PUB_CAT_MAX_LEN);
		}

		itCat->second = rs;
	}

	//query rewrite is_publication
	rs = "";
	hash_map<string,string>::iterator itPub = hmParams.find(GP);
	if(itSt!= hmParams.end() && strcmp(itSt->second.c_str(),FULL_SEARCH)==0)
	{
		if(itPub!=hmParams.end())
		{
			rs+=",";
			rs+=IS_PUBLICATION;
			itPub->second +=rs;
		}
		else
			hmParams.insert(make_pair(GP,IS_PUBLICATION));
	}
	
	//must init	
	//memset(&m_kStatNum,0x0,sizeof(KStatNum));

	// query rewrite dd_sale_price
	rs = "";
	if(itSt!= hmParams.end() && strcmp(itSt->second.c_str(),MALL_SEARCH)==0)
	{
		if(itPub!=hmParams.end())
		{
			rs+=",";
			rs+=PRICE_SPAN;
			itPub->second +=rs;
		}
		else
			hmParams.insert(make_pair(GP,PRICE_SPAN));
	}
	
}

void CQueryResultPack::SetGroupByAndFilterSequence(IAnalysisData* pad, vector<SFGNode>& vFgn)
{	
	hash_map<string,string>::iterator itPromo = pad->m_hmUrlPara.find(FILT_PROMO_TYPE);
	hash_map<string,string>::iterator i = pad->m_hmUrlPara.find(ST);
	int fieldId;
	
	SFGNode sfg;
	vector< SFGNode > vSg;
	vSg.clear();
	
	for(int j = 0; j < vFgn.size(); ++j)	
	{
		fieldId = vFgn[j].nField;	
		if(itPromo!=pad->m_hmUrlPara.end())
		{
			if(fieldId != -1 && strcmp(m_vFieldInfo[fieldId].strFieldName.c_str(),PROMO_TYPE)==0
					&& vFgn[j].nType==SFGNode::FILT_TYPE)
			{
				vFgn[j].nCid		= -1;
				vFgn[j].nType		= SFGNode::FILT_TYPE; 
				vFgn[j].nId			= PROMO_FILT;
			}
		}		
		
		if ((m_vFieldInfo[fieldId].nSpecialType == PARA_FILED ||
	strcmp(m_vFieldInfo[fieldId].strFieldName.c_str(),BRAND_ID)==0)
				&& vFgn[j].nType==SFGNode::GROUPBY_TYPE)
		{
		
			unsigned long long cls = 0;
			string path;

			hash_map<string,string>::iterator itCatPath;
			if(( itCatPath= pad->m_hmUrlPara.find(CATE_PATH))!=pad->m_hmUrlPara.end())
			{
				path = itCatPath->second;

				cls = TranseClsPath2ID(path.c_str(), path.length());
				int nPflId = GetFieldId(CATE_PATH);
				if (cls != 0 && nPflId != -1)
				{
					vFgn[j].sgpf.nMax=cls;
					vFgn[j].sgpf.nMin=cls;
					vFgn[j].sgpf.nNot=0;
					vFgn[j].sgpf.nPflId=nPflId;
				}
			}
		}
		
		// mall and pub category level	
		if (fieldId != -1 && m_vFieldInfo[fieldId].nSpecialType == CAT_FIELD 
				&& vFgn[j].nType == SFGNode::GROUPBY_TYPE)
		{
			if(i != pad->m_hmUrlPara.end() && strcmp(i->second.c_str(),PUB_SEARCH)==0)
				vFgn[j].sgpf.nCatlimit = 3;
			else if(i != pad->m_hmUrlPara.end() && strcmp(i->second.c_str(),MALL_SEARCH)==0)
				vFgn[j].sgpf.nCatlimit = 4;

		}
		
		// custom price filt
		if(fieldId != -1 && strcmp(m_vFieldInfo[fieldId].strFieldName.c_str(),PRICE_SPAN) ==0 
				&& vFgn[j].nType == SFGNode::FILT_TYPE)
		{
			vFgn[j].nCid		= -1;
			vFgn[j].nType		= SFGNode::FILT_TYPE; 
			vFgn[j].nId			= SALE_PRICE_FILT;
		}

		//custom stock filt
		if(fieldId != -1 && strcmp(m_vFieldInfo[fieldId].strFieldName.c_str(),STOCK_STATUS) ==0
				&& vFgn[j].nType == SFGNode::FILT_TYPE)
		{
			vFgn[j].nCid        = -1;
			vFgn[j].nType       = SFGNode::FILT_TYPE;
			vFgn[j].nId         = STOCK_STATUS_FILT;
		}
		
		
		if(i != pad->m_hmUrlPara.end() && strcmp(i->second.c_str(),FULL_SEARCH)==0)
		{
			if (fieldId != -1 && m_vFieldInfo[fieldId].nSpecialType == CAT_FIELD
					&& vFgn[j].nType==SFGNode::GROUPBY_TYPE)
			{
				vFgn[j].nCid = -1;
				vFgn[j].nId = CAT_CUSTOM_GROUP;
			}
		}
		
		if(i != pad->m_hmUrlPara.end() && strcmp(i->second.c_str(),MALL_SEARCH)==0)
		{
			if(fieldId != -1 && strcmp(m_vFieldInfo[fieldId].strFieldName.c_str(),PRICE_SPAN)==0
						&& vFgn[j].nType==SFGNode::GROUPBY_TYPE)
			{
				vFgn[j].nCid        = -1;
				vFgn[j].nType       = SFGNode::GROUPBY_TYPE;
				vFgn[j].nId         = PRICE_CUSTOM_GROUP;
			}
		}
	}
	//if(strcmp(i->second.c_str(),MALL_SEARCH)!=0)
	if(i != pad->m_hmUrlPara.end() && strcmp(i->second.c_str(),FULL_SEARCH)==0)
	{
		//ebook custom group	
		memset(&sfg,0x0,sizeof(SFGNode));
		sfg.nCid    = -1;
		sfg.nId     = EBOOK_CUSTOM_GROUP;
		sfg.nType   = SFGNode::GROUPBY_TYPE;
		vFgn.push_back(sfg);
	
		//full search need to filt ebook product which have paper book
		memset(&sfg,0x0,sizeof(SFGNode));
		sfg.nCid    = -1;
		sfg.nId     = EBOOK_CUSTOM_FILT;
		sfg.nType   = SFGNode::FILT_TYPE;
		vFgn.push_back(sfg);
	
		memset(&sfg,0x0,sizeof(SFGNode));
		sfg.nCid    = -1;
		sfg.nId     = EBOOK_CUSTOM_FILT_BEF_GP;
		sfg.nType   = SFGNode::FILT_TYPE;
		vFgn.push_back(sfg);
		
		/*memset(&sfg,0x0,sizeof(SFGNode));
		sfg.nCid    = -1;
		sfg.nId     = PROD_CUSTOM_RES;
		sfg.nType   = SFGNode::FILT_TYPE;
		vFgn.push_back(sfg);
		*/
	}
	//adjust group and filt sequence 
	for(int se = 0;se<m_vecSeq.size();se++)
	{
		for(int fg = 0;fg<vFgn.size();fg++)
		{
			if(vFgn[fg].nCid!=-1)
			{
				if(m_vecSeq[se].nFld == vFgn[fg].nField && m_vecSeq[se].nType == vFgn[fg].nType)	
				{
					vSg.push_back(vFgn[fg]);	
					break;
				}
			}
			else 
			{
				if(m_vecSeq[se].nId == vFgn[fg].nId && m_vecSeq[se].nCid == vFgn[fg].nCid)	
				{	
					vSg.push_back(vFgn[fg]);
					break;
				}

			}
		}
	}

	/*for(int f = 0; f<vFgn.size();f++)
	{
			fieldId = vFgn[f].nField;	
			if(fieldId!=-1)		
			cout<<"name: "<<m_vFieldInfo[fieldId].strFieldName
				<<" type: "<<vFgn[f].nType<<endl;
	}
	cout<<endl;*/
	//swap vector
	vFgn = vSg;
	/*for(int v = 0; v<vSg.size();v++)
	{
			fieldId = vSg[v].nField;	
			if(fieldId!=-1)		
			cout<<"name: "<<m_vFieldInfo[fieldId].strFieldName
				<<" type: "<<vSg[v].nType<<endl;
	}*/

}

void CQueryResultPack::CustomFilt(IAnalysisData* pad, vector<int>& vDocIds, SFGNode& fgNode)
{
#ifdef DEBUG 
	int iTime = 0;
	TimeUtil t_cust_filt;
#endif
	long long ebook_product_id = 0; 
	vector< long long > vecEbookId;
	int iDocSize = vDocIds.size();
	int subcat_product_id = 0; 
	void* pProfile;
	int iPapProd = 0;
	map_number_nolock<int> mapSet;
	mapSet.init(2000000);

	if(fgNode.nId == INNERCAT_CUSTOM_FILT)  
	{    
		set <int>       setInnerProductId;
		vector<int>     vecDocId;
		map <int,int>   mapShopProductId;
		vector <string> str_catid;
		hash_map<string,string>::iterator it_cat = pad->m_hmUrlPara.find(INNER_CAT);
		hash_map<string,string>::iterator it_shop = pad->m_hmUrlPara.find(SHOP_ID);
		if(it_shop == pad->m_hmUrlPara.end())
		{    
			it_shop = pad->m_hmUrlPara.find(SHOP_INFO);
		}    

		int k,j; 
		KShopIndex kShopIndex;
		memset(&kShopIndex,0x0,sizeof(KShopIndex));
		GetShopData(it_shop->second,kShopIndex);    
		//判断innercat的最后一位是否为‘0’，若为‘0’则是主分类，否则为子分类
		int len = it_cat->second.length() - 1;
		bool flag = false;
		if('0' == it_cat->second.c_str()[len])
			flag = true;

		for(k = kShopIndex.strOff; k < kShopIndex.endOff; ++k)
		{
			if(flag)
			{
				if(0 == strncmp(it_cat->second.c_str(),m_vecShopData[k].catid,3))
				{
					str_catid.push_back(m_vecShopData[k].catid);
				}
			}
			else
			{
				if(0 == strcmp(it_cat->second.c_str(),m_vecShopData[k].catid))
				{
					str_catid.push_back(m_vecShopData[k].catid);
				}
			}
		}

		KProIndex kProIndex;
		int cat_size = str_catid.size();
		for(k = 0;k < cat_size;++k)
		{
			memset(&kProIndex,0x0,sizeof(KProIndex));
			GetProIndex(str_catid[k],kProIndex);
			for(j = kProIndex.strOff;j <= kProIndex.endOff;++j)
			{
				setInnerProductId.insert(m_vecProductData[j].product_id);
			}
		}

		pProfile = FindProfileByName(PRODUCT_ID);
		for(int i = 0;i < iDocSize;i++)
		{
			subcat_product_id = m_funcFrstInt64(pProfile,vDocIds[i]);
			mapShopProductId.insert(make_pair(subcat_product_id,vDocIds[i]));
		}

		set <int >::iterator itInner = setInnerProductId.begin();
		map <int,int >::iterator itShop  = mapShopProductId.begin();
		for(;itInner != setInnerProductId.end();++itInner)
		{
			for(;itShop != mapShopProductId.end();++itShop)
			{
				if(itShop->first > *itInner)
					break;
				if(*itInner == itShop->first)
				{
					vecDocId.push_back(itShop->second);
					break;
				}
			}
		}

		vector <int >::iterator itPro;
		for(k = 0;k < iDocSize;++k)
		{
			itPro = vecDocId.begin();
			for(;itPro != vecDocId.end();++itPro)
			{
				if(*itPro > vDocIds[k])
				{
					vDocIds[k] = -1;
					break;
				}
				if(vDocIds[k] == *itPro)
					break;
			}
		}

	}

	string path="";
	string st = "";
	int iEbookNum = 0;
	char chBuf[20];
	hash_map<string,string>::iterator itCatPath;
	if((( itCatPath= pad->m_hmUrlPara.find(CATE_PATH))!=pad->m_hmUrlPara.end())
		||((itCatPath= pad->m_hmUrlPara.find(FILT_CATE_PATH))!=pad->m_hmUrlPara.end()))
				path = itCatPath->second;
	hash_map<string,string>::iterator itST = pad->m_hmUrlPara.find(ST);
	//hash_set<int> setPap;
	
	if(itST!=pad->m_hmUrlPara.end())
		st = itST->second;	
	//if(fgNode.nId==PROD_CUSTOM_RES)
	/*if(strcmp(st.c_str(),FULL_SEARCH)==0)	
	{
		for(int set=0; set<iDocSize;set++)
		{
			if(vDocIds[set]==-1)
				continue;
			iPapProd = m_funcFrstInt64(m_ProdProfile,vDocIds[set]);
			if( iPapProd>=1900000000 && iPapProd<=2000000000)
				continue;	
			//cout<<"paper product: "<<iPapProd<<endl;
			setPap.insert(iPapProd);	
		}
	}*/
	if (fgNode.nId==EBOOK_CUSTOM_FILT)
	{
		//1.filt ebook which has paper book
		InitHashSet(mapSet,vDocIds);
		for(int i = 0;i<iDocSize;i++)
		{
			if(vDocIds[i]==-1)
				continue;
			ebook_product_id = m_funcFrstInt64(m_ProdProfile,vDocIds[i]);
			if( ebook_product_id>=1900000000 && ebook_product_id<=2000000000)
			{	
				if((itCatPath!=pad->m_hmUrlPara.end() && strcmp(st.c_str(),PUB_SEARCH)==0 
						&& strncmp(path.c_str(),"98",2)!=0) || (itCatPath==pad->m_hmUrlPara.end()))
				{
					iPapProd = m_funcFrstInt(m_PapProfile,vDocIds[i]);	
					//cout<<"before: "<<ebook_product_id<<"::"<<iPapProd<<endl;
					//cout<<"ebook: "<<ebook_product_id<<endl;
					//if( iPapProd >0 && setPap.find(iPapProd)!=setPap.end())
					if( iPapProd >0 && mapSet.get(iPapProd)!=-1)
					{
						vDocIds[i] = -1;
						//cout<<"after: "<<ebook_product_id<<"::"<<iPapProd<<endl;
						iEbookNum ++;
					}
				}
			}
		}
		sprintf(chBuf,"%d",iEbookNum);	
		//cout<<iEbookNum<<endl;	
		if(pad->m_strReserve=="")   
		{
			pad->m_strReserve=EBOOK_NUM_FILT;
		}
		else
		{
			pad->m_strReserve+=";";
			pad->m_strReserve+=EBOOK_NUM_FILT;
		}
		pad->m_strReserve+=":";
		pad->m_strReserve+=chBuf;
		//cout<<"iEbookNum: "<<pad->m_strReserve<<endl;

	}

	if(fgNode.nId==EBOOK_CUSTOM_FILT_BEF_GP)
	{
		InitHashSet(mapSet,vDocIds);
		for(int i = 0;i<iDocSize;i++)
		{
			if(vDocIds[i]==-1)
				continue;
			ebook_product_id = m_funcFrstInt64(m_ProdProfile,vDocIds[i]);
			if( ebook_product_id>=1900000000 && ebook_product_id<=2000000000)
			{	
				if((itCatPath!=pad->m_hmUrlPara.end() && strcmp(st.c_str(),PUB_SEARCH)==0 
						&& strncmp(path.c_str(),"98",2)!=0) || (itCatPath==pad->m_hmUrlPara.end()))
				{
					iPapProd = m_funcFrstInt(m_PapProfile,vDocIds[i]);	
					if(iPapProd>0 && mapSet.get(iPapProd)!=-1 &&
							m_funcFrstInt(m_DisplayProfile,vDocIds[i])==3)
					{
						vDocIds[i]=-1;
					}
				}
			}	
		}
		
		vDocIds.erase(remove(vDocIds.begin(),vDocIds.end(),-1),vDocIds.end());
	}

	int iPromoType = 0;	
	//promo process	
	if(fgNode.nId == PROMO_FILT)
	{
		//if(m_CatPromoTypeProfile && m_PromoTypeProfile)
		{
			for(int p = 0;p<iDocSize;p++)
			{
				if(vDocIds[p]==-1)
					continue;
				//1.test product promo
				iPromoType = m_funcFrstInt(m_PromoTypeProfile,vDocIds[p]);
				if(JudgeIfPromo(iPromoType,vDocIds[p],PROMO_TYPE))
					continue;

				//2.test cat promo
				iPromoType = m_funcFrstInt(m_CatPromoTypeProfile,vDocIds[p]);
				if(JudgeIfPromo(iPromoType,vDocIds[p],CAT_PROMO_TYPE))
					continue;
		
				vDocIds[p]=-1;

			}
		}
	}

	int iPrice = 0;	
#ifdef PRICE
	TimeUtil t_pri;
#endif
	//dd_sale_price filt
	if(fgNode.nId == SALE_PRICE_FILT)
	{
		//parse price param	
		hash_map<string,string>::iterator p = pad->m_hmUrlPara.find(DD_PRICE_FILT);	
		string szPrice;	
		CCustPriceFilt CustFilt;
		if(p != pad->m_hmUrlPara.end())
		{
			//1.get price param	
			szPrice = p->second; 
			CustFilt.SignalParse(szPrice);
			
			//2.filt docid 	
			for(int k = 0;k<iDocSize;k++)
			{
				if(vDocIds[k]==-1)
					continue;
				GetRealPrice(iPrice,vDocIds[k],m_PriceProfile);		
				for(int cVal = 0;cVal<CustFilt.m_vSnglVal.size();cVal++)	
				{
					if(iPrice != CustFilt.m_vSnglVal[cVal])
					{
						vDocIds[k]=-1;
						break;
					}
				}
				
				for(int cRange = 0; cRange <CustFilt.m_vPrVal.size();cRange++)
				{
					if(iPrice<CustFilt.m_vPrVal[cRange].first 
							|| iPrice>CustFilt.m_vPrVal[cRange].second)
					{
						vDocIds[k]=-1;
						break;
					}
				}

			}
		}
	}
	
	//stock custom filt
	int iStock = 0;
	if(fgNode.nId == STOCK_STATUS_FILT)
	{
		for(int s = 0;s<iDocSize;s++)
		{
			ebook_product_id = m_funcFrstInt64(m_ProdProfile,vDocIds[s]);
			if(vDocIds[s]==-1)
				continue;
			if( ebook_product_id>=1900000000 && ebook_product_id<=2000000000 )
				continue;		
			iStock = m_funcFrstInt(m_StockProfile,vDocIds[s]);	
			if(iStock==0)
				vDocIds[s]=-1;
			
		}
	}

#ifdef PRICE
	iTime = t_pri.getPassedTime();
	cout<<"Price cost: "<<iTime<<endl;
#endif
	vDocIds.erase(remove(vDocIds.begin(),vDocIds.end(),-1),vDocIds.end());
#ifdef DEBUG 
	iTime = t_cust_filt.getPassedTime();
	cout<<"cust filt cost: "<<iTime<<endl;
#endif
	
}

void CQueryResultPack::InitHashSet(map_number_nolock<int>& setPap,vector<int>& vDocIds)
{
	int iPapProd = 0;
	for(int set=0; set<vDocIds.size();set++)
	{
		if(vDocIds[set]==-1)
			continue;
		iPapProd = m_funcFrstInt64(m_ProdProfile,vDocIds[set]);
		if( iPapProd>=1900000000 && iPapProd<=2000000000)
			continue;	
		//setPap.insert(iPapProd);	
		setPap.put(iPapProd);
	}
	
}

void CQueryResultPack::SetSequence(int iSrc,vector<SFGNode>& vFgn,int iDes)
{
	SFGNode sfg;	
	if(iSrc<iDes)
	{
		sfg = vFgn[iSrc];	
		for(int i = iSrc+1;i<iDes;i++)
			vFgn[i-1]=vFgn[i];
		vFgn[iDes] = sfg;
	}
	else if(iSrc>iDes)
	{
		sfg = vFgn[iSrc];	
		for(int j = iSrc-1;j>=iDes;j--)
			vFgn[j+1] = vFgn[j];	
		vFgn[iDes] = sfg;
	}
}
