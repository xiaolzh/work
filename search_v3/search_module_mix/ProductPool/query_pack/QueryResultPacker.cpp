#include "QueryResultPacker.h"

CQueryResultPack::CQueryResultPack()
{
}

CQueryResultPack::~CQueryResultPack()
{
	if(m_pMallBit != NULL)	
	{
		delete m_pMallBit;
		m_pMallBit=NULL;
	}
	if(m_pPubBit != NULL)
	{
		delete m_pPubBit;
		m_pPubBit = NULL;
	}
}


void CQueryResultPack::ShowResult(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
		vector<SResult>& vRes, vector<string>& vecRed, string& strRes)
{
		
	//change term to string vector
	if(ErrorMessage(pad,vGpRes,strRes))
	{
		return;
	}
	string key="";	
	vector< string > vecStr;
	vecStr.clear();
	string strTmp;
	string code = "";
	int iPrice = 0;
	int iTime = 0;


#ifdef 	DEBUG
	TimeUtil t_sh;
#endif	
	hash_map<string,string>::iterator itTQ = pad->m_hmUrlPara.find(TQ);

	if(itTQ!=pad->m_hmUrlPara.end())
	{
		key =itTQ->second;
		for(int a=0;a<pad->m_vTerms.size();a++)
		{
			strTmp.assign(key,pad->m_vTerms[a].pos,pad->m_vTerms[a].len);	
			vecStr.push_back(strTmp);
		}
	}

	char chBuf[256];
	CXmlHttp xh;
#ifdef 	DEBUG
	TimeUtil t_sd;
#endif	
	strRes+="<Header>";
	strRes+="<Term>";
	bool dot = false;
	for(int vt = 0;vt<vecStr.size();vt++)
	{
		if(!dot)
			dot = true;
		else
			strRes+=",";	
		strRes+=vecStr[vt];

	}
	strRes+="</Term>";
		
	ShowStatisticData(vGpRes,strRes, pad,ctrl,vRes);
	strRes+="</Header>";
#ifdef DEBUG
	iTime = t_sd.getPassedTime();
	cout<<"ShowStatisticData cost: "<<iTime<<endl;	
#endif
	HSII hsii;
	void*pProfile;
	string str;
	string sPath;
	string sTmp;

	strRes+="<StatInfo>";

	FillGroupData(vGpRes);
#ifdef 	DEBUG
	TimeUtil t_cx;
#endif	
	CombineXmlInfo(pad,ctrl,vGpRes,vRes,vecRed,strRes);
#ifdef DEBUG
	iTime = t_cx.getPassedTime();
	cout<<"CombineXmlData cost: "<<iTime<<endl;	
#endif
	
#ifdef 	DEBUG
	TimeUtil t_gp;
#endif	
	hash_map<string,string>::iterator itPar  = pad->m_hmUrlPara.find(ST);
	for(size_t i=0;i<vGpRes.size();++i)
	{
		vector<SGroupByInfo> & vgb = vGpRes[i].second;
		if(!vGpRes[i].second.size()&&
				strcmp(m_vFieldInfo[vGpRes[i].first].strFieldName.c_str(),CATE_PATH)!=0)
		{
			continue;
		}

		if(strcmp(m_vFieldInfo[vGpRes[i].first].strFieldName.c_str(),CATE_PATH)==0
				&& itPar!=pad->m_hmUrlPara.end() && strcmp(itPar->second.c_str(),FULL_SEARCH)!=0)
		{
			ShowMallOrPubCatInfo(vgb,sTmp,pad,sPath);
			strRes+=sPath;
			strRes+="<"+m_vFieldInfo[vGpRes[i].first].strFieldName+">";
			strRes+=sTmp;
			strRes+="</"+m_vFieldInfo[vGpRes[i].first].strFieldName+">";
		}
		else if(strcmp(m_vFieldInfo[vGpRes[i].first].strFieldName.c_str(),CATE_PATH)!=0)
		{
			strRes+="<"+m_vFieldInfo[vGpRes[i].first].strFieldName+">";
			for(size_t j=0;j<vgb.size();++j)
				PackCatInfo(strRes,vgb[j]);

			strRes+="</"+m_vFieldInfo[vGpRes[i].first].strFieldName+">";
		}
	}

#ifdef DEBUG
	iTime = t_gp.getPassedTime();
	cout<<"ShowMallOrPubCatInfo cost: "<<iTime<<endl;	
#endif
	strRes+="</StatInfo>";


	string strName;

	//mark red......

	vector<int>& vShowFields=ctrl.vecShowFieldIds;
	vector<char> vBuf;
	vector<char*> vFieldPtr;

	hash_map<string,string>::iterator itST = pad->m_hmUrlPara.find(ST);

	vector< long long > vPk;
	vector< int > vDoc;
	CAbstractInfo m_absInfo;
	int iData;
	strRes+="<Body>";
	if (vRes.size())
	{
		CXmlHttp xh;
		string sRed;
		int fid;
		int iPub;
		for (size_t i=ctrl.nF; i<ctrl.nT;++i)
		{
			#ifdef SIZE	
			TimeUtil t_bit;
			#endif	
			strRes+="<Product>";
			strRes+="<DocId>";
			sprintf(chBuf,"%d",vRes[i].nDocId);
			strRes+=chBuf;
			strRes+="</DocId>";

			strRes+="<Score>";
			sprintf(chBuf,"%d",vRes[i].nScore);
			strRes+=chBuf;
			strRes+="</Score>";

			strRes+="<Weight>";
			sprintf(chBuf,"%d",vRes[i].nWeight);
			strRes+=chBuf;
			strRes+="</Weight>";
			/*strRes+="<Term>";
			bool dot = false;
			for(int vt = 0;vt<vecStr.size();vt++)
			{
				if(!dot)
					dot = true;
				else
					strRes+=",";	
				strRes+=vecStr[vt];

			}
			strRes+="</Term>";
			*/
			m_funcDocInfoPtr(vRes[i].nDocId,vShowFields,vFieldPtr,vBuf ,m_pSearcher);

			#ifdef SIZE	
			TimeUtil t_size;
			#endif	
			bool IfPromo = false;	
			vPk.clear();
			vDoc.clear();
            vDoc.resize(2,0);
			iData = GetFieldId(PRODUCT_ID);	
			
			int iEbook = 0;
            iEbook = m_funcFrstInt(m_EbookIdProfile,vRes[i].nDocId);
            if(iEbook > 0)
			{
                vPk.push_back(iEbook);

			    m_funcGetDocsByPkPtr(m_pSearcher,iData,vPk,vDoc); 			
            }
			
			for (size_t j=0;j<vShowFields.size();++j)
			{
				fid=vShowFields[j];
				strName=m_vFieldInfo[fid].strFieldName;
				
				#ifdef SIZE	
					TimeUtil t_bit;
				#endif	
				//pProfile = FindProfileByName(IS_PUBLICATION);	
				iPub = m_funcFrstInt(m_PubProfile,vRes[i].nDocId);				

				//full search need to know whether the docid is pub	
				if(strcmp(itST->second.c_str(),FULL_SEARCH)==0)
				{
					if(iPub)
					{
						if(!m_pPubBit->TestBit(fid))
						{
							continue;
						}
					}
					else 
					{
						if(!m_pMallBit->TestBit(fid))
						{
							continue;
						}
					}
				}

				//1.add classcode( mall show ) 
				if(strcmp(strName.c_str(),CATE_PATH)==0)
				{
					code ="";
					//string tmp = vFieldPtr[j];
					vector< string > vecCat;
					SplitToVecEx(vFieldPtr[j],vecCat,SPLIT);
					if(vecCat.size())		
					{
						GetCode(vecCat[0],code);
						if(code!="")
						{
							strRes+="<ClassCode>";
							xh.XmlPackText(code.c_str(),strRes);
							//xh.XmlPackTextWithTrim(code.c_str(),strRes);
							strRes+="</ClassCode>";
						}
					}
				}

				strRes+="<";
				strRes+=strName;
				strRes+=">";

				//text red	
				if(m_vFieldInfo[fid].chDataType!=NUMBER)
				{

					#ifdef RED	
					TimeUtil t_red;
					#endif	
					string tmpStr = vFieldPtr[j];	
					//text	
					if ((vecRed.empty() && vecStr.empty())||!IsRedField(strName,vRes[i].nDocId))
					{
						xh.XmlPackText(vFieldPtr[j],strRes);
						//xh.XmlPackTextWithTrim(vFieldPtr[j],strRes);
					}
					else if(vecStr.empty())
					{
						sRed.clear();
						m_absInfo.Process(vecRed,tmpStr,sRed);
						xh.XmlPackText(sRed.c_str(),strRes);
						//xh.XmlPackTextWithTrim(sRed.c_str(),strRes);
					}
					else
					{
						sRed.clear();
						if(!(m_absInfo.Process(vecRed,tmpStr,sRed)) && strcmp(strName.c_str(),PRODUCT_NAME)==0)
						{
							sRed.clear();	
							m_absInfo.Process(vecStr,tmpStr,sRed);	
						}

						xh.XmlPackText(sRed.c_str(),strRes);
						//xh.XmlPackTextWithTrim(sRed.c_str(),strRes);
					}
					#ifdef RED 
					iTime = t_red.getPassedTime();
					cout<<"RedProcess cost: "<<iTime<<endl;	
					#endif
				}
				else
				{	
					// whether promo is effected	
					if(strcmp(strName.c_str(),PROMO_TYPE)==0||
							strcmp(strName.c_str(),CAT_PROMO_TYPE)==0)
					{
						if(!JudgeIfPromo(atoi(vFieldPtr[j]),vRes[i].nDocId,strName))
							strcpy(vFieldPtr[j],"0");
					}
					else if(strcmp(strName.c_str(),PRICE_SPAN)==0)
					{
						GetRealPrice(iPrice,vRes[i].nDocId,m_PriceProfile);
						sprintf(chBuf,"%d",iPrice);	
						strcpy(vFieldPtr[j],chBuf);
					}
					else if(strcmp(strName.c_str(),PRICE_SPAN_MAX)==0)
					{
						GetRealPrice(iPrice,vRes[i].nDocId,m_PriceMaxProfile);
						sprintf(chBuf,"%d",iPrice);	
						strcpy(vFieldPtr[j],chBuf);

					}
					else if(strcmp(strName.c_str(),IS_HAS_EBOOK)==0)
					{
                        //cout<<vDoc[0]<<" "<<vFieldPtr[j]<<endl;
						if(vDoc[0]==-1)
							strcpy(vFieldPtr[j],"0");	
					}
					else if(strcmp(strName.c_str(),EBOOK_PRODUCT_ID)==0)
					{
						if(vDoc[0]==-1)	
							strcpy(vFieldPtr[j],"0");	
					}
					else if(strcmp(strName.c_str(), DISCOUNT) == 0)
					{
							GetRealPrice(iPrice,vRes[i].nDocId,m_PriceProfile);
							int price = 0;
							price = m_funcFrstInt(m_PurPriceProfile,vRes[i].nDocId);
							float discount = (float)iPrice/price;
							int realDis = (int)(discount*100+0.5);
							sprintf(chBuf,"%d",realDis);	
							strcpy(vFieldPtr[j],chBuf);
					}
					strRes+=vFieldPtr[j];
				}
				strRes+="</";
				strRes+=strName;
				strRes+=">";
				#ifdef SIZE 
				iTime = t_bit.getPassedTime();
				cout<<"bit cost: "<<iTime<<endl;	
				#endif

			}	
			#ifdef SIZE 
			iTime = t_size.getPassedTime();
			cout<<"ShowField cost: "<<iTime<<endl;	
			#endif
			//add ebook	xml ebook_sale_price,discount,display_status,book_id

			if(iPub)
			{
				//pProfile = FindProfileByName(HAS_EBOOK);	
				int iHasEbook = m_funcFrstInt(m_HasEbookProfile,vRes[i].nDocId);				
			    iData = -1;	

			/*	vPk.clear();
				vDoc.clear();
				iData = GetFieldId(PRODUCT_ID);	
			*/

				if(iHasEbook)
				{
					//pProfile = FindProfileByName(EBOOK_ID);	
					//int iEbook = m_funcFrstInt(m_EbookIdProfile,vRes[i].nDocId);
					//vPk.push_back(iEbook);

					//m_funcGetDocsByPkPtr(m_pSearcher,iData,vPk,vDoc); 			
					if(vDoc[0]==-1)
						goto OUT;
					//m_vecAddEbook must int type	
					for(int k = 0;k<m_vecAddEbook.size();k++)
					{
						iData = -1;	
						if(strcmp(m_vecAddShowEbook[k].c_str(),"is_ebook")==0)	
							iData = 0;
						else
							iData = m_funcFrstInt(m_vecProfile[k],vDoc[0]);

						strRes+="<";	
						strRes+=m_vecAddShowEbook[k];
						strRes+=">";
						if(iData>=0)
						{
							sprintf(chBuf,"%d",iData);
							strRes+=chBuf;
						}
						strRes+="</";
						strRes+=m_vecAddShowEbook[k];
						strRes+=">";
					}
				}
				else
				{
					//all write zero
					for(int c = 0;c<m_vecAddEbook.size();c++)
					{
							
						iData = -1;
						//1.a product must judge with the name which is is_ebook	
						if(strcmp(m_vecAddShowEbook[c].c_str(),"is_ebook")==0)
						{
							iData = m_funcFrstInt64(m_vecProfile[c],vRes[i].nDocId); 	
							if(iData>=1900000000 && iData<=2000000000)
								iData = 1;	
							else
								iData = 0;

							
						}
						// 2.ebook must show ebook_display_status,not display_staus
						// but display is not removed
						else if(strcmp(m_vecAddShowEbook[c].c_str(),"ebook_display_status")==0)
							iData = m_funcFrstInt(m_vecProfile[c],vRes[i].nDocId);	
						strRes+="<";	
						strRes+=m_vecAddShowEbook[c];
						strRes+=">";
						if(strcmp(m_vecAddShowEbook[c].c_str(),"is_ebook")==0 && iData>=0)
						{
							sprintf(chBuf,"%d",iData);
							strRes+=chBuf;
						}
						else if(strcmp(m_vecAddShowEbook[c].c_str(),"ebook_display_status")==0
								&& iData >= 0)
						{
							sprintf(chBuf,"%d",iData);
							strRes+=chBuf;
						}
						strRes+="</";
						strRes+=m_vecAddShowEbook[c];
						strRes+=">";
					}
				}
			}
OUT:
			strRes+="</Product>";
		}
	}
	strRes+="</Body>";
#ifdef DEBUG
	iTime = t_sh.getPassedTime();
	cout<<"ShowResult cost: "<<iTime<<endl;	
#endif
}

bool CQueryResultPack::JudgeIfPromo(int promo_type,int iDocId,string name)
{
	int iPromoStart = 0;	
	int iPromoEnd   = 0;
	int iCatPromoStart = 0;
	int iCatPromoEnd = 0;
	bool bCatFlag = false;
	bool bProdFlag = false;
	bool bFlag = true;
	int  iPromoFilt = 0;
	time_t now;
	time(&now);

	//1.cat promo type judge 
	if(strcmp(name.c_str(),CAT_PROMO_TYPE)==0 && 
			promo_type)
	{
		if(m_CatPromoStartProfile && m_CatPromoEndProfile)
		{
			iCatPromoStart = m_funcFrstInt(m_CatPromoStartProfile,iDocId);	
			iCatPromoEnd   = m_funcFrstInt(m_CatPromoEndProfile,iDocId);
			if(now>=iCatPromoStart && now<=iCatPromoEnd)
				bCatFlag = true;
			bFlag = bCatFlag;	
		}
		else
			return false;
	}

	//2.product promo type judge
	if((strcmp(name.c_str(),PROMO_TYPE)==0 && promo_type)
			||(bCatFlag && m_funcFrstInt(m_PromoTypeProfile,iDocId)))
	{
		if(m_PromoStartProfile && m_PromoEndProfile)	
		{
			iPromoStart = m_funcFrstInt(m_PromoStartProfile,iDocId);	
			iPromoEnd   = m_funcFrstInt(m_PromoEndProfile,iDocId);
			if(now>=iPromoStart && now<=iPromoEnd)
			{
				iPromoFilt = m_funcFrstInt(m_PromoFiltProfile,iDocId);
				if(iPromoFilt)
					bProdFlag = true;
			}
			bFlag = bProdFlag;
		}
		else
			return false;

	}
	if(!promo_type)
		return false;
	else if(bProdFlag)
	{
		return bProdFlag;
	}
	else if(bCatFlag)
	{
		return bCatFlag;
	}
	else
		return false;

}

void CQueryResultPack::GetRealPrice(int& sale_price,int iDocId,void* profile)
{
	int promo_price = 0;
	int dd_price	= 0;
	int iPromoType  = 0;

	if(m_PromoTypeProfile && m_CatPromoTypeProfile)
	{
		iPromoType = m_funcFrstInt(m_PromoTypeProfile,iDocId);
		if(JudgeIfPromo(iPromoType,iDocId,PROMO_TYPE))
				//|| JudgeIfPromo(iPromoType,iDocId,CAT_PROMO_TYPE))
			promo_price = m_funcFrstInt(m_PromoProfile, iDocId);
	}

	//dd_price = m_funcFrstInt(m_PriceProfile,iDocId);
	dd_price = m_funcFrstInt(profile,iDocId);
	sale_price = ( (promo_price == 0) ? dd_price : (promo_price < dd_price ? promo_price :  dd_price) );
}

void CQueryResultPack::ShowStatisticData(GROUP_RESULT &vGpRes,string& str, IAnalysisData* pad,
		CControl& ctrl,vector<SResult>& vRes)
{
	char chBuf[256];
	int iNum = -1;
	int iMod = 0;
	string strTmp;
	int iMallNum = 0;
	int iPubNum = 0;
	char szChar[1024];
	int iTotal=0;
	int iEbookFlt =0;
	KStatNum kStatNum;
	memset(&kStatNum,0x0,sizeof(KStatNum));
	hash_map< string,string > hSD;
	vector< string > tmp;
	vector< string > Str;
	hash_map<string,string>::iterator itSt = pad->m_hmUrlPara.find(ST);
	hash_map<string,string>::iterator itPub = pad->m_hmUrlPara.find(IS_PUBLICATION);
	hash_map<string,string>::iterator itShow;

	tmp.clear();
	if(pad->m_strReserve.c_str()!="")
	{
		//strcpy(szChar,pad->m_strReserve.c_str());
		//SplitToVecEx(szChar,tmp,";");
		SplitToVecEx(pad->m_strReserve.c_str(),tmp,";");
	}
	for(int i=0;i<tmp.size();i++)
	{
		Str.clear();
		//cout<<tmp[i]<<endl;
		//strcpy(szChar,tmp[i].c_str());	
		//SplitToVecEx(szChar,Str,":");	
		SplitToVecEx(tmp[i].c_str(),Str,":");	
		if(Str.size()==2 && Str[0].length()>0 && Str[1].length()>0)	
			hSD.insert(hash_map< string,string >::value_type(Str[0],Str[1]));	
	}
	
	itShow = hSD.find(EBOOK_NUM_FILT);	
	if(itShow!=hSD.end())
		kStatNum.iEbookFlt = atoi(itShow->second.c_str());	
	
	itShow = hSD.find(EBOOK_NUM_GP);
	if(itShow != hSD.end())
	{
		Str.clear();
		//strcpy(szChar,itShow->second.c_str());	
		//SplitToVecEx(szChar,Str,",");
		SplitToVecEx(itShow->second.c_str(),Str,",");
		if(Str.size()==2)
		{
			kStatNum.iPapNum = atoi(Str[1].c_str());
			kStatNum.iEbookNum = atoi(Str[0].c_str());
		}
			
	}
	
	if(itSt!=pad->m_hmUrlPara.end() && strcmp(itSt->second.c_str(),FULL_SEARCH)==0)	
		iTotal = vRes.size()+kStatNum.iEbookFlt;
	else
		iTotal = vRes.size();
	str+="<Summary>";
	str+="<TotalCnt>";
	sprintf(chBuf,"%d",iTotal);
	str+=chBuf;
	str+="</TotalCnt>";
	if(itSt!=pad->m_hmUrlPara.end() && strcmp(itSt->second.c_str(),FULL_SEARCH)==0)	
	{
		iTotal = kStatNum.iEbookNum+kStatNum.iEbookFlt;	
		str+="<ResultCountEbook>";
		sprintf(chBuf,"%d",iTotal);
		str+=chBuf;
		str+="</ResultCountEbook>";

		str+="<ResultCountPbook>";
		sprintf(chBuf,"%d",kStatNum.iPapNum);
		str+=chBuf;
		str+="</ResultCountPbook>";

		iNum =GetGpIndex(vGpRes,IS_PUBLICATION); 
		if(iNum!=-1)
		{
			vector< SGroupByInfo > &sg = vGpRes[iNum].second;	
			if(sg.size())
			{
				for(int i = 0;i<sg.size();i++)
				{
					if(sg[i].nGid == 0)
						iMallNum = sg[i].nCnt;	
					else
						iPubNum = sg[i].nCnt+kStatNum.iEbookFlt;	
				}
				//vGpRes[iNum].second.clear();
				sg.clear();
			}
			str+="<ResultCountMall>";
			sprintf(chBuf,"%d",iMallNum);
			str+=chBuf;
			str+="</ResultCountMall>";
			
			str+="<ResultCountPub>";
			sprintf(chBuf,"%d",iPubNum);
			str+=chBuf;
			str+="</ResultCountPub>";
		}

		//set template
		itShow = hSD.find(WEB);
		if(itShow!=hSD.end())
		{
			str+="<WebTemplate>";
			str+=itShow->second;
			str+="</WebTemplate>";
		}

	}	
	iNum = vRes.size()/ctrl.usRetCnt;
	iMod = vRes.size()%ctrl.usRetCnt;
	if(iMod)	
		iNum++;

	str+="<Page>";
	str+="<PageIndex>";
	sprintf(chBuf,"%d",ctrl.usRetOff);
	str+=chBuf;
	str+="</PageIndex>";

	str+="<PageSize>";
	sprintf(chBuf,"%d",ctrl.usRetCnt);
	str+=chBuf;
	str+="</PageSize>";

	str+="<PageCount>";
	sprintf(chBuf,"%d",iNum);
	str+=chBuf;
	str+="</PageCount>";
	str+="</Page>";
	str+="</Summary>";	

}


void CQueryResultPack::ShowMallOrPubCatInfo(vector<SGroupByInfo> &sg,string& str,
		IAnalysisData* pad,string& sPath)
{
	int iSize = sg.size();	
	string sLev;
	string sTmp;
	u64 tmpCls = 0;
	int iLev = 0;
	u64 clsID =0;
	CXmlHttp xh;
	int i = 0;
	int iCurLev = 0;
	int iEndLev = 0; 
	bool bFlag = false;
	int has_child = 1;
	char chBuf[512];
	u64 iTmpID = 0;
	char szPath[18];
	string cat_path;
	string code = "";

	string sCatLev[MAX_CATE_LEVEL] = {"level1","level2","level3","level4","level5","level6"};
	

	hash_map<string,string>::iterator itCat;
	if((( itCat= pad->m_hmUrlPara.find(CATE_PATH))!=pad->m_hmUrlPara.end())
			||((itCat= pad->m_hmUrlPara.find(FILT_CATE_PATH))!=pad->m_hmUrlPara.end()))
	{
		clsID = TranseClsPath2ID(itCat->second.c_str(),itCat->second.length()); 	
		iLev = GetClsLevel(clsID);	
	}

	iEndLev = GetClsLevel(sg[iSize-1].nGid);

	if(iEndLev == iLev)
	{
		bFlag = true;
		has_child = 0; 
		tmpCls = GetClassByLevel(iLev-1,clsID);
	}

	sPath = "<Path>";
	sPath+="<items>";

	for(int p=1;p<iLev+1;p++)
	{
		iTmpID = GetClassByLevel(p,clsID);	
		/*if(iTmpID == m_clsID||!m_PathToCatHash[szPath].catid[0])
			continue;*/
		if(iTmpID == m_clsID)
			continue;
			
		TranseID2ClsPath(szPath,iTmpID,MAX_CATE_LEVEL);
		if(p==2)
			cat_path ="-";
		else
			cat_path+= "-";
		cat_path+=m_PathToCatHash[szPath].catid;
		sPath+="<item>";	
		sPath+="<Name>";
		//xh.XmlEncode(sg[i].bufName,sPath);
		xh.XmlEncode(m_CatInfoHash[iTmpID].chCLs,sPath);
		sPath+="</Name>";
		sPath+="<CatPath>";
		//sPath+=sg[i].bufId2str;
		sPath+=szPath;	
		sPath+="</CatPath>";
		sPath+="<CatID>";
		sPath+=m_PathToCatHash[szPath].catid;
		sPath+="</CatID>";
		sPath+="<ClassCode>";
		sPath+=cat_path;
		sPath+="</ClassCode>";
		sPath+="<guan_id>";
		sprintf(chBuf,"%d",m_PathToCatHash[szPath].guan_id);
		sPath+=chBuf;
		sPath+="</guan_id>";
		sPath+="</item>";

	}
	sPath+="</items>";
	sPath+="</Path>";
	while(i!=iSize)
	{
		iCurLev =GetClsLevel(sg[i].nGid); 
		/*if(iCurLev < iLev)
		  {
		  sPath+="<item>";	
		  sPath+="<Name>";
		  xh.XmlEncode(sg[i].bufName,sPath);
		  sPath+="</Name>";
		  sPath+="<CatPath>";
		  sPath+=sg[i].bufId2str;
		  sPath+="</CatPath>";
		  sPath+="<CatID>";
		  sPath+=m_PathToCatHash[sg[i].bufId2str].catid;
		  sPath+="</CatID>";
		  sPath+="</item>";
		  }*/

		if(m_clsID == sg[i].nGid)
		{
			i++;	
			continue;
		}
		if(str=="")	
			str="<"+sCatLev[iCurLev-1]+">";	
		else
			str+="<"+sCatLev[iCurLev-1]+">";
		str+="<items>";

		while(i<iSize && iCurLev == GetClsLevel(sg[i].nGid))
		{

			i++;

			/*if((sg[i-1].nGid == clsID && iLev == iCurLev))
			  {
			  sPath+="<item>";	
			  sPath+="<Name>";	
			  xh.XmlEncode(sg[i-1].bufName,sPath);
			  sPath+="</Name>";
			  sPath+="<CatPath>";
			  sPath+=sg[i-1].bufId2str;
			  sPath+="</CatPath>";
			  sPath+="<CatID>";
			  sPath+=m_PathToCatHash[sg[i-1].bufId2str].catid;
			  sPath+="</CatID>";
			  sPath+="</item>";
			  }*/

			if((sg[i-1].nGid == clsID && iLev == iCurLev)
					||(bFlag && iLev == iCurLev+1 && sg[i-1].nGid == tmpCls))
			{
				PackCatInfo(sTmp,sg[i-1]);
				continue;
			}
			if(iLev == iCurLev &&(sg[i-1].nGid != clsID)||
					(bFlag && iLev == iCurLev+1 && sg[i-1].nGid != tmpCls))
			{	
				PackCatInfo(sLev,sg[i-1]);	
				continue;
			}

			PackCatInfo(str,sg[i-1]);	

		}
		if(iLev == iCurLev||(bFlag && iLev == iCurLev+1))
		{
			str+=sTmp;
			str+=sLev;
		}
		sTmp = "";
		sLev ="";
		str+="</items>";	
		str+="</"+sCatLev[iCurLev-1]+">";
	}
	/*sPath+="</items>";	
	  sPath+="</Path>";*/

	sPath+="<has_child>";
	sprintf(chBuf,"%d",has_child);
	sPath+=chBuf;
	sPath+="</has_child>";
}

void CQueryResultPack::PackCatInfo(string& s,SGroupByInfo& sg)
{
	char chBuf[512];
	CXmlHttp xh;
	string code ="";
	GetClassCode(sg.nGid,code);
	if(sg.bufName[0]=='\0')
	{
		return;
	}
	if(s=="")
		s="<CategoryInfo>";
	else
		s+="<CategoryInfo>";
	s+="<CatPath>";	
	s+=sg.bufId2str;
	s+="</CatPath>";
	s+="<Name>";
	xh.XmlEncode(sg.bufName,s);
	s+="</Name>";
	s+="<Count>";
	sprintf(chBuf,"%d",sg.nCnt);
	s+=chBuf;
	s+="</Count>";
	s+="<CatID>";
	s+=m_PathToCatHash[sg.bufId2str].catid;
	s+="</CatID>";
	if(code!="")
	{
		s+="<ClassCode>";
		s+=code;
		s+="</ClassCode>";
	}

	s+="</CategoryInfo>";

}

bool CQueryResultPack::IsRedField(string &fld,int iDoc)
{
	void* pProfile;		
	//pProfile = FindProfileByName(IS_PUBLICATION);	
	int IsPub = m_funcFrstInt(m_PubProfile,iDoc);

	if(IsPub)
	{
		for(int pub = 0;pub<m_vecPubRed.size();pub++)
		{
			if(fld==m_vecPubRed[pub])
			{
				return true;
			}
		}
	}
	else
	{
		for(int bh = 0;bh<m_vecBhRed.size();bh++)
		{
			if(fld == m_vecBhRed[bh])
			{
				return true;	
			}
		}
	}
	return false;
}

int CQueryResultPack::GetGpIndex(GROUP_RESULT &vGpRes,const char* gp)
{
	int iNum = -1;	
	int FldID = -1;
	if((FldID = GetFieldId(gp))==-1)
	{
		return iNum;
	}

	for(int m = 0;m<vGpRes.size();m++)
	{
		if(vGpRes[m].first == FldID)
		{
			iNum = m;
			break;
		}
	}

	return iNum;
}

void CQueryResultPack::CombineXmlInfo(IAnalysisData* pad, CControl& ctrl, GROUP_RESULT &vGpRes,
		vector<SResult>& vRes, vector<string>& vecRed, string& strRes)
{	
	hash_map<string,string>::iterator itShop = pad->m_hmUrlPara.find(SHOP_INFO);
	hash_map<string,string>::iterator itPar  = pad->m_hmUrlPara.find(ST);
	if(itShop != pad->m_hmUrlPara.end() && itPar != pad->m_hmUrlPara.end() && 0 == strcmp(itPar->second.c_str(),SHOP_SEARCH))
	{	
		ShowShopInfo(itShop->second,strRes);
	}
	if(!vGpRes.size())
	{
		return ;
	}
	hash_map< string,string > hP;
	vector< string > tmp;
	vector< string > str;
	hash_map<string,string>::iterator itShow;
	char szChar[1024];	
	tmp.clear();
	if(pad->m_strReserve.c_str()!="")
	{
		//strcpy(szChar,pad->m_strReserve.c_str());
		//SplitToVecEx(szChar,tmp,";");
		SplitToVecEx(pad->m_strReserve.c_str(),tmp,";");
	}

	for(int i=0;i<tmp.size();i++)
	{
		str.clear();	
		strcpy(szChar,tmp[i].c_str());	
		SplitToVecEx(szChar,str,":");	
		if(str.size()==2 && str[0].length()>0 && str[1].length()>0)	
			hP.insert(hash_map< string,string >::value_type(str[0],str[1]));	
	}

	int iNum = -1;	

	if(itPar!=pad->m_hmUrlPara.end())	
	{
		if(strcmp(itPar->second.c_str(),MALL_SEARCH)==0)
		{
			//shop group info pack
			iNum  =	GetGpIndex(vGpRes,SHOP_INFO); 
			//shop group info pack
			if(iNum!=-1)
			{
				if(vGpRes[iNum].second.size())
				{
					ShowShopGroupInfo(vGpRes[iNum].second,strRes,vRes);	
					vGpRes[iNum].second.clear();
				}
			}

			iNum =GetGpIndex(vGpRes,DD_SELL); 
			if(iNum!=-1)
			{
				if(vGpRes[iNum].second.size())
				{
					ShowDangDangAndShopNum(vGpRes[iNum].second,strRes,vRes);
					vGpRes[iNum].second.clear();
				}
			}

			// price span	
			//StatisticPriceSpan(vRes,strRes);	
				
			itShow=hP.find(PRICE_SPAN);
			if(itShow!=hP.end())
					strRes+=itShow->second;
			
			//brand info pack
			iNum  =	GetGpIndex(vGpRes,BRAND_INFO); 
			if(iNum !=-1)
			{
				if(vGpRes[iNum].second.size())
				{
					ShowBrandInfo(vGpRes[iNum].second,strRes,pad);		
					vGpRes[iNum].second.clear();
				}
			}

			//attr info pack	
			iNum  =	GetGpIndex(vGpRes,ATT_INFO); 

			if(iNum!=-1)
			{
				//u64 clsID = TranseClsPath2ID(it->second.c_str(),CATE_PATH_LEN); 	
				//int lev = GetClsLevel(clsID);	
				//if(lev>=ATTR_SHOW_LEVEL)
				{
					//show attr info	
					ShowAttrInfo(vGpRes[iNum].second,strRes,pad);	
					vGpRes[iNum].second.clear();
				}
			}

		}
		else if(strcmp(itPar->second.c_str(),PUB_SEARCH)==0)
		{
			// price span	
			//if(pad->m_hmUrlPara[SP]!=SPCIALSALE)
			{
				//StatisticPriceSpan(vRes,strRes);
				//strRes+=m_sPriceStr;
			}
			/* else
			   {
			   ShowSpecialPriceSpan(vRes,strRes);
			   }*/
		}
		else if(strcmp(itPar->second.c_str(),FULL_SEARCH)==0)
		{
			iNum = GetGpIndex(vGpRes,ATTR_CATEGORY);		

			if(iNum!=-1)
			{
				//show full category info	
				ShowFullCatInfo(vGpRes[iNum].second,strRes);
				vGpRes[iNum].second.clear();
			}

		}
		else
		{
			SESSION_LOG(SL_ERROR,"url param wrong!!");	
			return ;
		}

		if(strcmp(itPar->second.c_str(),MALL_SEARCH)!=0)
		{
			iNum = GetGpIndex(vGpRes,ATT_INFO);	
			if(iNum!=-1)
			{
				vGpRes[iNum].second.clear();
			}

			iNum = GetGpIndex(vGpRes,BRAND_INFO);	
			if(iNum!=-1)
			{
				vGpRes[iNum].second.clear();
			}

			iNum = GetGpIndex(vGpRes,SHOP_INFO);	
			if(iNum!=-1)
			{
				vGpRes[iNum].second.clear();
			}
		}
	}
	else
	{
		SESSION_LOG(SL_ERROR,"url param has not search type !!");
		return ;
	}

	return ;
}

void CQueryResultPack::GetClassCode(u64	&clsID,string &code)
{
	u64 iTmpID = GetClassByLevel(1,clsID);  
	int iLev   = GetClsLevel(clsID);
	char szPath[18];
	char chBuf[512];

	if(iTmpID != m_clsID)
	{    
		return;
	} 

	for(int i = 2;i<=iLev;i++)
	{
		iTmpID = GetClassByLevel(i,clsID);	
		TranseID2ClsPath(szPath,iTmpID,MAX_CATE_LEVEL);
		if(code=="")	
			code ="-";
		else
			code+="-";
		if(!m_PathToCatHash[szPath].catid)
			return;
		code+=m_PathToCatHash[szPath].catid;
	}
}

void CQueryResultPack::GetCode(string &cat_path,string& code)
{
	if(cat_path=="")
		return;
		
	u64 clsID =TranseClsPath2ID(cat_path.c_str(),17);
	GetClassCode(clsID,code);
}

void CQueryResultPack::ShowFullCatInfo(vector<SGroupByInfo> &sg,string& str)
{
	string strLevel[2]={"level1","level2"};
	int iLev = 0;
	int iPreLev = -1;
	int iPPreLev = 0;
	int clsID =TranseClsPath2ID("58",2); 			
	char chBuf[512];

	CXmlHttp xh;
	str+="<";	
	str+=ATTR_CATEGORY;	
	str+=">";
	//if(!sg.size())	
		str+="<"+strLevel[iLev]+">";	
	str+="<items>";
	sort(sg.begin(),sg.end(),SORT_BY_CNT);

	for(int i=0;i<sg.size();i++)
	{
		//filt 58.00.00.00.00.00	
		if(sg[i].nGid == clsID)
		{
			continue;
		}

		//show level1,level2	
		if(GetClassByLevel(1,sg[i].nGid)==m_clsID)
		{
			if(GetClsLevel(sg[i].nGid)==2)
			{
				iLev = 0;
			}
			else
			{
				iLev = 1;
			}
		}
		else
		{
			if(GetClsLevel(sg[i].nGid)==1)
			{
				iLev = 0;
			}
			else
			{
				iLev = 1;
			}

		}

		/*if(iPreLev!=iLev)	
		{
			iPreLev = iLev;

			if(iPPreLev!=iPreLev)
			{
				//str+="</items>";
				str+="</"+strLevel[iLev]+">";
				iPPreLev = iPreLev;
			}

			str+="<"+strLevel[iLev] +">";
			//str+="<items>";
		}*/

		/*str+="<item id=\"";
		  str+=sg[i].bufId2str;
		  str+="\" name=\"";
		  xh.XmlEncode(sg[i].bufName,str);
		  str+="\" count=\"";
		  sprintf(chBuf,"%d",sg[i].nCnt);
		  str+=chBuf; 
		  str+="\"/>";*/
		PackCatInfo(str,sg[i]);

	}
	str+="</items>";	
	str+="</"+strLevel[iLev]+">";
	str+="</";	
	str+=ATTR_CATEGORY;	
	str+=">";
}


bool CQueryResultPack::Init(SSearchDataInfo* psdi, const string& strConf)
{
	CModule::Init(psdi, strConf);	
	if(!LoadAttrData())
	{
		COMMON_LOG(SL_ERROR,"load attr data error!!");	
		return false;
	}
	if(!LoadShopData())
	{
		COMMON_LOG(SL_ERROR,"load shop data error!!");
		return false;
	}
	//load config file
	LoadConfig();

	//load global data
	if(!InitGlobalVal())
	{
		COMMON_LOG(SL_ERROR,"load global val error!!");
		return false;
	}

	//load brand info	
	KBrandInfo kBrandInfo;		
	kBrandInfo.priority = -1;
	string file_name = m_strModulePath+BRAND_DATA_RDX;

	if(!m_BrandInfoHash.load_serialized_hash_file(file_name.c_str(),kBrandInfo))
	{
		COMMON_LOG(SL_ERROR,"load brand data error!!");
		return false;
	}
	
	 file_name = m_strModulePath+BRAND_TO_NAME;
	if(!m_BrdToName.load_serialized_hash_file(file_name.c_str(),kBrandInfo))
	{
		COMMON_LOG(SL_ERROR,"load brand data error!!");
		return false;
	}

	//load shop info 	
	KShopInfo kShopInfo;	
	kShopInfo.shop_name[0]='\0';	
	file_name = m_strModulePath+SHOP_DATA_RDX;

	if(!m_ShopInfoHash.load_serialized_hash_file(file_name.c_str(),kShopInfo))
	{
		COMMON_LOG(SL_ERROR,"load shop data error!!");		
		return false;
	}

	//load category path info
	cls_buf kCategoryInfo;
	kCategoryInfo.chCLs[0]='\0';
	file_name = m_strModulePath+CATEGORY_PATH;

	if(!m_CatInfoHash.load_serialized_hash_file(file_name.c_str(),kCategoryInfo))
	{
		COMMON_LOG(SL_ERROR,"load category info error!!");	
		return false;
	}

	//load category info
	KCatToPath kCatToPath;
	kCatToPath.path[0]='\0';
	file_name = m_strModulePath+CATID_TO_PATH;

	if(!m_CatToPathHash.load_serialized_hash_file(file_name.c_str(),kCatToPath))
	{
		COMMON_LOG(SL_ERROR,"load category to path error!!");	
		return false;
	}

	KPathToCat kPathToCat;
	kPathToCat.catid[0]='\0';
	kPathToCat.guan_id = 0;
	file_name = m_strModulePath+PATH_TO_CATID;

	if(!m_PathToCatHash.load_serialized_hash_file(file_name.c_str(),kPathToCat))
	{
		COMMON_LOG(SL_ERROR,"load path to category  error!!");	
		return false;
	}
	COMMON_LOG(SL_INFO,"query pack init ok!!");
	return true;
}

//void CQueryResultPack::StatisticPriceSpan(vector<SResult>& vRes,string& strRes)
void CQueryResultPack::StatisticPriceSpan(vector<int>& vRes,IAnalysisData* pad)
{
	char chBuf[256];
	string strRes="";
	strRes = "<PriceInterval>";
	
	if(vRes.size()<MIN_PRODUCT_LIMIT)
	{
		strRes+="</PriceInterval>"	;
		return;
	}
	strRes+="<items>";
	char * sNum;
	void *pProfile;
	int nPriceNum[vRes.size()];
	int iPrice = 0;
	//int iNum = 0;
	//int iPrice = 0;
	clsPriceSpan priceInterval;

	//price interval
	for(int m = 0;m<m_vFieldInfo.size();m++)
	{
		if(strcmp(m_vFieldInfo[m].strFieldName.c_str(),PRICE_SPAN)==0)
		{
			//1.get profile pointer
			//pProfile = FindProfileByName(PRICE_SPAN);

			//2.get price   
			for(int n = 0;n<vRes.size();n++)
			{
				//GetRealPrice(iPrice,vRes[n].nDocId,m_PriceProfile);	
				//nPriceNum[n] = iPrice;
				nPriceNum[n] = m_funcFrstInt(m_PriceProfile,vRes[n]);
				//iNum++;
			}
			if(!priceInterval.buildSpan(nPriceNum,vRes.size()))
			{	
				strRes+="</items>";
				strRes+="</PriceInterval>";
				return;		
			}

			//3.pack price interval xml
			int iStart = -1;
			for(int p = 0;p<PRICE_SPAN_SIZE;p++)
			{
				if(!priceInterval.m_PriceSpan[p].nCount)
				{
					if(iStart!=-1)	
						continue;
					else
					{
						iStart = priceInterval.m_PriceSpan[p].nStartPrice;
						continue;
					}
				}
				strRes+="<item>";
				strRes+="<Count>";

				sprintf(chBuf,"%d",priceInterval.m_PriceSpan[p].nCount);
				strRes+=chBuf;
				strRes+="</Count>";
				strRes+="<Min>";
				if(iStart!=-1)
					sprintf(chBuf,"%d",iStart);
				else
					sprintf(chBuf,"%d",priceInterval.m_PriceSpan[p].nStartPrice);
				strRes+=chBuf;
				strRes+="</Min>";
				strRes+="<Max>";
				sprintf(chBuf,"%d",priceInterval.m_PriceSpan[p].nEndPrice);
				strRes+=chBuf;
				strRes+="</Max>";
				strRes+="</item>";
				iStart = -1;
			}
			break;
		}
	}
	strRes+="</items>";
	strRes+="</PriceInterval>";
	if(pad->m_strReserve=="")	
	{
		pad->m_strReserve=PRICE_SPAN;
	}
	else
	{
		pad->m_strReserve+=";";
		pad->m_strReserve+=PRICE_SPAN;

	}
	pad->m_strReserve+=":";	
	pad->m_strReserve+=strRes;

}


void CQueryResultPack::FillGroupData(vector<pair<int, vector<SGroupByInfo> > >& vGpInfo)
{
	typedef pair<int, vector<SGroupByInfo> > PIV;
	vector<PIV>::iterator i;
	vector<SGroupByInfo>::iterator j;
	cls_buf kCatInfo;	
	KPathToCat kPathToCat;
	string str;

	for (i = vGpInfo.begin(); i != vGpInfo.end(); ++i)
	{
		if (strcmp(m_vFieldInfo[i->first].strFieldName.c_str(),CATE_PATH)==0)
		{
			for (j = i->second.begin(); j != i->second.end(); ++j)
			{
				kCatInfo = m_CatInfoHash[j->nGid];
				if(!kCatInfo.chCLs[0])
					continue;
				strncpy(j->bufName, kCatInfo.chCLs, sizeof(j->bufName));
				j->bufName[sizeof(j->bufName)-1] = 0;

				/*if(m_clsID == GetClassByLevel(1,j->nGid))
				  {
				  str = j->bufId2str;	
				  kPathToCat = m_PathToCatHash[str];	
				  if(!kPathToCat.catid[0])
				  continue;
				  strncpy(j->bufId2str, kPathToCat.catid, sizeof(j->bufId2str));
				  j->bufId2str[sizeof(j->bufId2str)-1] = 0;
				  }*/
			}
		}
		else
		{
			continue;
		}
	}
}

//special book price 
void CQueryResultPack::ShowSpecialPriceSpan(vector<SResult>& vRes,string &strRes)
{
	map<int, int> specialSaleDiscountMap;

	PRICESPAN price_span[SRICESPLEN];

	void *pProfile;
	char chBuf[256];
	int val = 0;
	for(int n = 0; n != SRICESPLEN; n++)
	{
		price_span[n].nStartPrice = g_PriceSpan[n][0];
		price_span[n].nEndPrice = g_PriceSpan[n][1];
		price_span[n].nCount = 0; 
	}

	for(int m = 0;m<m_vFieldInfo.size();m++)
	{
		if(strcmp(m_vFieldInfo[m].strFieldName.c_str(),PRICE_SPAN)==0
				||strcmp(m_vFieldInfo[m].strFieldName.c_str(),DISCOUNT)==0)
		{
			for (int i=0; i<vRes.size(); ++i)
			{
				//1.get profile pointer
				//pProfile = FindProfileByName(PRICE_SPAN);

				val = m_funcFrstInt(m_PriceProfile,vRes[i].nDocId);
				//2.get price	
				for(int k = 0; k != SRICESPLEN; k++)
				{
					if(val > price_span[k].nStartPrice 
							&& (val <= price_span[k].nEndPrice  || price_span[k].nEndPrice == 0)  )
					{
						price_span[k].nCount ++; 
						break;
					}
				}

				//1.get profile pointer
				//pProfile = FindProfileByName(DISCOUNT);

				val = m_funcFrstInt(m_DiscountProfile,vRes[i].nDocId);

				//2.get discount
				for(int j = 0; j != DISCOUNTLEN; j++)
				{
					if(val > g_Discount[j][0] 
							&& (val <= g_Discount[j][1] || g_Discount[j][1] == 0))
					{
						specialSaleDiscountMap[j]++;
						break;
					}
				}
			}					    
		}
	}

	//3.pack xml info
	strRes+="<SpecialSale>";	
	strRes+="<items>";

	for(int p = 0;p<SRICESPLEN;p++)
	{
		strRes+="<item>";
		strRes+="<Count>";

		sprintf(chBuf,"%d",price_span[p].nCount);
		strRes+=chBuf;
		strRes+="</Count>";
		strRes+="<Min>";
		sprintf(chBuf,"%d",price_span[p].nStartPrice);
		strRes+=chBuf;
		strRes+="</Min>";
		strRes+="<Max>";
		sprintf(chBuf,"%d",price_span[p].nEndPrice);
		strRes+=chBuf;
		strRes+="</Max>";
		strRes+="</item>";
	}
	strRes+="</items>";
	strRes+="</SpecialSale>";

	strRes+="<Discount>";
	strRes+="<items>";

	for (int m=0; m<DISCOUNTLEN; ++m)
	{
		if(specialSaleDiscountMap[m] > 0)
		{

			strRes+="<item>";
			strRes+="<Count>";

			sprintf(chBuf,"%d",specialSaleDiscountMap[m]);
			strRes+=chBuf;
			strRes+="</Count>";
			strRes+="<Min>";
			sprintf(chBuf,"%d",g_Discount[m][0]);
			strRes+=chBuf;
			strRes+="</Min>";
			strRes+="<Max>";
			sprintf(chBuf,"%d",g_Discount[m][1]);
			strRes+=chBuf;
			strRes+="</Max>";
			strRes+="</item>";
		}
	}
	strRes+="</items>";
	strRes+="</Discount>";
	return;
}


bool CQueryResultPack::InitGlobalVal()
{
	void* pProfile=NULL;	

	//1.all ebook profile field ptr	
	for(int i = 0;i<m_vecAddEbook.size();i++)
	{
		pProfile = FindProfileByName(m_vecAddEbook[i].c_str());
		if(!pProfile)
		{
			COMMON_LOG(SL_ERROR, "%s does not exist",m_vecAddEbook[i].c_str());
			return false;
		}
		m_vecProfile.push_back(pProfile);
		pProfile=NULL;
	}

	//2.mall and pub field bit map
	m_pMallBit = new CBitMap(m_vFieldInfo.size());
	m_pPubBit = new CBitMap(m_vFieldInfo.size());
	SetShowBitMap(m_pMallBit,m_vecMallField);	
	SetShowBitMap(m_pPubBit,m_vecPubField);	

	//3.set gloal variable
	m_PubProfile = FindProfileByName(IS_PUBLICATION);
	if (m_PubProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",IS_PUBLICATION);
		return false;
	}

	m_HasEbookProfile = FindProfileByName(HAS_EBOOK);
	if (m_HasEbookProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",HAS_EBOOK);
		return false;
	}

	m_EbookIdProfile = FindProfileByName(EBOOK_ID);
	if (m_EbookIdProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",EBOOK_ID);
		return false;
	}

	m_PriceProfile = FindProfileByName(PRICE_SPAN);
	if (m_PriceProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PRICE_SPAN)
			return false;
	}
	m_PurPriceProfile = FindProfileByName(PRICE);
	if (m_PurPriceProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PRICE);
			return false;
	}
	
    m_ProductBit = FindProfileByName(PRODUCT_BIT);
    if (m_ProductBit == NULL)
    {
        COMMON_LOG(SL_ERROR, "product_bit profile does not exist");
        return false;
    }

	m_PriceMaxProfile = FindProfileByName(PRICE_SPAN_MAX);
	if (m_PriceMaxProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PRICE_SPAN_MAX)
			return false;
	}

	/*m_DiscountProfile = FindProfileByName(DISCOUNT);
	  if (m_DiscountProfile == NULL)
	  {
	  COMMON_LOG(SL_ERROR, "discount profile does not exist");
	  return false;
	  }*/

	m_PapProfile = FindProfileByName(PAPER_PRODUCT_ID);	
	if (m_PapProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PAPER_PRODUCT_ID);
		return false;
	}

	m_PromoProfile = FindProfileByName(PROMO_PRICE);	
	if (m_PromoProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PROMO_PRICE);
		return false;
	}

	m_PromoTypeProfile = FindProfileByName(PROMO_TYPE);	
	if (m_PromoTypeProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PROMO_TYPE);
		return false;
	}

	m_CatPromoTypeProfile = FindProfileByName(CAT_PROMO_TYPE);	
	if (m_CatPromoTypeProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",CAT_PROMO_TYPE);
		return false;
	}

	m_PromoStartProfile = FindProfileByName(PROMO_START_DATE);	
	if (m_PromoStartProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PROMO_START_DATE);
		return false;
	}

	m_PromoEndProfile = FindProfileByName(PROMO_END_DATE);	
	if (m_PromoEndProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PROMO_END_DATE);
		return false;
	}

	// no error temporarily	
	m_CatPromoStartProfile = FindProfileByName(CAT_PROMO_START_DATE);	
	if (m_CatPromoStartProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",CAT_PROMO_START_DATE);
		return false;
	}

	//no error temporarily
	m_CatPromoEndProfile = FindProfileByName(CAT_PROMO_END_DATE);	
	if (m_CatPromoEndProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",CAT_PROMO_END_DATE);
		return false;
	}
	
	m_ProdProfile = FindProfileByName(PRODUCT_ID);	
	if (m_ProdProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PRODUCT_ID);
		return false;
	}
	
	m_StockProfile = FindProfileByName(STOCK_STATUS);	
	if (m_StockProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",STOCK_STATUS);
		return false;
	}
	
	m_DisplayProfile = FindProfileByName(DISPLAY_STATUS);	
	if (m_DisplayProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",DISPLAY_STATUS);
		return false;
	}
	
	m_PromoFiltProfile = FindProfileByName(PROMOTION_FILT);	
	if (m_PromoFiltProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PROMOTION_FILT);
		return false;
	}
	
	m_ProdFiltProfile = FindProfileByName(PROD_PROMOTION_FILT);	
	if (m_ProdFiltProfile == NULL)
	{
		COMMON_LOG(SL_ERROR, "%s profile does not exist",PROD_PROMOTION_FILT);
		return false;
	}

	m_clsID = TranseClsPath2ID("58",2);	

	return true;
}

void CQueryResultPack::SetShowBitMap(CBitMap* pBit,vector< string > & vec)
{
	int fld = -1;	
	for(int n = 0;n<vec.size();n++)
	{
		fld = -1;
		fld = GetFieldId(vec[n].c_str());
		if(fld !=-1)
			pBit->SetBit(fld);
	}

}

void CQueryResultPack::LoadConfig()
{
	m_mapShowField.clear();	
	m_vecSeq.clear();
	string rs;	
	string file_name = m_strModulePath+CONFIG_FILE;
	ResourceUtil::configure(file_name.c_str());
	vector< string > tmp;
	vector< string > str;
	char szChar[1024];
	KSequence kSeq;
	//load pub,bh,full,showfield
	rs = ResourceUtil::getProperty(MALL_SEARCH); 				
	m_mapShowField.insert(hash_map< string,string >::value_type(MALL_SEARCH,rs));	
	SplitToVecEx(rs.c_str(),m_vecMallField,",");	

	rs = ResourceUtil::getProperty(PUB_SEARCH);
	m_mapShowField.insert(hash_map< string,string >::value_type(PUB_SEARCH,rs));
	SplitToVecEx(rs.c_str(),m_vecPubField,",");	

	rs = ResourceUtil::getProperty(FULL_SEARCH);
	m_mapShowField.insert(hash_map< string,string >::value_type(FULL_SEARCH,rs));

	rs = ResourceUtil::getProperty(EBOOK_SEARCH);
	SplitToVecEx(rs.c_str(),m_vecShowEbook,",");	


	rs = ResourceUtil::getProperty(EBOOK_SHOW);
	SplitToVecEx(rs.c_str(),tmp,";");


	rs = ResourceUtil::getProperty(PUB_RED_FIELD);	
	SplitToVecEx(rs.c_str(),m_vecPubRed,",");	

	rs = ResourceUtil::getProperty(MALL_RED_FIELD);	
	//strcpy(szChar,rs.c_str());	
	//SplitToVecEx(szChar,m_vecBhRed,",");	
	SplitToVecEx(rs.c_str(),m_vecBhRed,",");	

	for(int i=0;i<tmp.size();i++)
	{
		str.clear();	
		//strcpy(szChar,tmp[i].c_str());	
		//SplitToVecEx(szChar,str,",");
		SplitToVecEx(tmp[i].c_str(),str,",");
		m_vecAddEbook.push_back(str[0]);	
		m_vecAddShowEbook.push_back(str[1]);
	}
	
	//load city stock info
	tmp.clear();	
	rs = ResourceUtil::getProperty(CITY_CODE_FILE);
	SplitToVecEx(rs.c_str(),tmp,",");

	for(int m = 0;m<tmp.size();m++)
		m_sCityCode[atoi(tmp[m].c_str())] = m;

	// load sequence field
	tmp.clear();	
	rs = ResourceUtil::getProperty(SEQUENCE);		
	SplitToVecEx(rs.c_str(),tmp,";");

	int iFld = -1;
	for(int l = 0;l<tmp.size();l++)
	{
		str.clear();	
		memset(&kSeq,0x0,sizeof(KSequence));
		
		SplitToVecEx(tmp[l].c_str(),str,",");
		
		//strcpy(kSeq.cat_name,str[0].c_str());	
		kSeq.nType = atoi(str[1].c_str());	
		kSeq.nId = atoi(str[2].c_str());
		kSeq.nCid = atoi(str[3].c_str());
		if(kSeq.nCid!=-1)
		{
			iFld = GetFieldId(str[0].c_str());
			kSeq.nFld = iFld;
		}
		else
		{
			kSeq.nFld = -1;
		}

		m_vecSeq.push_back(kSeq);
	}

}

void CQueryResultPack::BeforeGroupBy(IAnalysisData* pad, vector<SResult>& vRes, 
		vector<PSS>& vGpName, GROUP_RESULT& vGpRes)
{
	;
}

/*IAnalysisData* CQueryResultPack::QueryAnalyse(SQueryClause& qc)
{
	CKeySrchAnalysis* p = new CKeySrchAnalysis;
	p->m_hmUrlPara = qc.hmUrlPara;
	SESSION_LOG(SL_NOTICE, "query_pack query analyse");

	return p;
}*/
#ifndef _WIN32
// linux dll
//CQueryResultPack promo_ranking;
CQueryResultPack query_pack;
#endif
