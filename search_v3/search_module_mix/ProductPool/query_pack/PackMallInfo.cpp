#include "QueryResultPacker.h"

void CQueryResultPack::GetAttrData(SGroupByInfo &sg ,vector< KAttrPack >& vPack,IAnalysisData* pad,
		hash_map< int,int > &hCnt)
{
	u64 attr_id     = 0;
	u64 sub_attr_id = 0;
	u64 off         = 0xffffffff;
	attr_id         =sg.nGid >>32;
	sub_attr_id     =sg.nGid & 0xffffffff;
	KAttrIndex      kIndex;
	KAttrPack       kPack;
	KSubAttrInfo    kSubAttr;
	long long cat_id =0;
	int iLev = 0;
	u64 clsID =0;
	u64 TmpCls = 0;
	u64 CurCls = 0;
	u64 ParCls = 0;
	string rs;
	string str;
	char chBuf[52];
	memset(&kPack,0x0,sizeof(KAttrPack));
	memset(&kIndex,0x0,sizeof(KAttrIndex));
	memset(&kSubAttr,0x0,sizeof(KSubAttrInfo));
	kPack.cnt = sg.nCnt;
	hash_map<string,string>::iterator itST = pad->m_hmUrlPara.find(ST);	
	hash_map<string,string>::iterator it;

	if((strcmp(itST->second.c_str(),MALL_SEARCH)==0)&&
			((it=pad->m_hmUrlPara.find(FILT_CATE_PATH))!=pad->m_hmUrlPara.end()
			||(it=pad->m_hmUrlPara.find(CATE_PATH))!=pad->m_hmUrlPara.end()))
	{
		str=it->second;
		clsID = TranseClsPath2ID(str.c_str(),str.length());
		iLev = GetClsLevel(clsID);
		ParCls = GetClassByLevel(iLev-1,clsID);
	}
	KSubAttrInfo* pSubInfo;
	//get data offset
	off = m_AttrInfoHash[attr_id];
	if(off==0xffffffff)
		return;

	KAttrIndex* pIdx = (KAttrIndex*)(&m_vecIdxData[off]);
	memcpy(&kIndex,pIdx,sizeof(KAttrIndex));

	
	//get data
	KAttrInfo* pDat = (KAttrInfo*)(&m_vecDatData[kIndex.offset]);
	memcpy(&kPack.kAttr,pDat,sizeof(KAttrInfo));
	
	//get clsID	
	if(iLev>1&&kPack.kAttr.catid>0)
	{
		sprintf(chBuf,"%ld", kPack.kAttr.catid);
		rs = m_CatToPathHash[chBuf].path;	
		strncpy(chBuf,rs.c_str(),3*iLev-1);
		chBuf[3*iLev-1]='\0';
		CurCls = TranseClsPath2ID(chBuf,strlen(chBuf));	
		TmpCls = GetClassByLevel(iLev-1,CurCls);
		if(ParCls != TmpCls)
		{
			return;
		}
	}
	//cout<<"after: "<<"attr_id: "<<attr_id<<" sub_attr_id: "<<sub_attr_id<<endl;
	
	hash_map< int,int >::iterator itCnt =hCnt.find(attr_id) ;
	if(itCnt!=hCnt.end())
		itCnt->second += kPack.cnt;
	else
		hCnt.insert(make_pair(attr_id,kPack.cnt));	

	off = kIndex.offset+sizeof(KAttrInfo);
	pSubInfo = ((KSubAttrInfo* )&m_vecDatData[off]);
	for(int i = 0;i<kIndex.cnt;i++)
	{
		memcpy(&kPack.kSubAttr,pSubInfo,sizeof(KSubAttrInfo));
		if(kPack.kSubAttr.sub_attrib_id == sub_attr_id)
		{
			vPack.push_back(kPack);
			break;
		}
		pSubInfo++;

	}
}

void CQueryResultPack::ShowBrandInfo(vector<SGroupByInfo> &sg,string& str,IAnalysisData* pad)
{
	vector< KResBrand > vResBrand;
	KResBrand kResBrand;
	CXmlHttp xh;
	KBrandKey kBrandKey;
	string rs="";
	string Str="";
	u64 nBrandKey = 0;
	memset(&kBrandKey,0x0,sizeof(KBrandKey));	
	hash_map<string,string>::iterator itST = pad->m_hmUrlPara.find(ST);	
	hash_map<string,string>::iterator it;
	
	if((strcmp(itST->second.c_str(),MALL_SEARCH)==0)&&
			((it=pad->m_hmUrlPara.find(FILT_CATE_PATH))!=pad->m_hmUrlPara.end()
			||(it=pad->m_hmUrlPara.find(CATE_PATH))!=pad->m_hmUrlPara.end()))
	{
		Str=it->second;
		Str+=".00";
		rs = m_PathToCatHash[Str].catid;
		
		if(rs!="")
			kBrandKey.cat_id = atoi(rs.c_str());	
		else
			kBrandKey.cat_id = 0;
	}
	for(int m =0;m<sg.size();m++)
	{
		memset(&kResBrand,0x0,sizeof(KResBrand));
		kBrandKey.brand_id = sg[m].nGid; 
		memcpy(&nBrandKey, &kBrandKey, sizeof(nBrandKey));
		if(!kBrandKey.brand_id)
		{
			continue;
		}
		if(m_BrandInfoHash[nBrandKey].priority==-1
				||!strlen(m_BrandInfoHash[nBrandKey].brand_name))
		{
			if(m_BrdToName[sg[m].nGid].priority ==-1||
					!strlen(m_BrdToName[sg[m].nGid].brand_name))
			{
				continue;
			}
			else
			{
				strcpy(kResBrand.brand_name,m_BrdToName[sg[m].nGid].brand_name);
				kResBrand.priority = m_BrdToName[sg[m].nGid].priority;
				kResBrand.brand_id = sg[m].nGid;
				kResBrand.nCnt = sg[m].nCnt;
				vResBrand.push_back(kResBrand);
			}
		}
		else
		{
			strcpy(kResBrand.brand_name,m_BrandInfoHash[nBrandKey].brand_name);
			kResBrand.priority = m_BrandInfoHash[nBrandKey].priority;
			kResBrand.brand_id = sg[m].nGid;
			kResBrand.nCnt = sg[m].nCnt;
			vResBrand.push_back(kResBrand);
			kBrandKey.brand_id = 0;
		}

	}
	sort(vResBrand.begin(),vResBrand.end());

	char chBuf[256];

	str+="<Brands>";
	str+="<items>";
	for(int n = 0;n<vResBrand.size();n++)
	{
		str+="<item>";
		str+="<ID>";
		sprintf(chBuf,"%d",vResBrand[n].brand_id);
		str+=chBuf;
		str+="</ID>";
		str+="<Name>";
		//str+=vResBrand[n].brand_name;
		xh.XmlEncode(vResBrand[n].brand_name,str);	
		str+="</Name>";
		str+="<Count>";
		sprintf(chBuf,"%d",vResBrand[n].nCnt);
		str+=chBuf;
		str+="</Count>";
		str+="<Priority>";
		sprintf(chBuf,"%d",vResBrand[n].priority);
		str+=chBuf;
		str+="</Priority>";
		str+="</item>";
	}
	str+= "</items>";
	str+= "</Brands>";
}

void CQueryResultPack::ShowAttrInfo(vector<SGroupByInfo> &sg,string& str,IAnalysisData* pad)
{
	vector< KAttrPack > vecPack;
	vecPack.clear();
	int attr_id = -1;
	char chBuf[512];
	hash_map< int,int >	hCnt;
	hash_map< string,KNameToCnt> hAttr;
	CXmlHttp xh;	
	hash_map< int,int >::iterator itCnt ;
	hash_map< string, KNameToCnt >::iterator itAttr;
	KNameToCnt kNameToCnt;
	for(int m = 0;m<sg.size();m++)
	{
		GetAttrData(sg[m],vecPack,pad,hCnt);
	}
	
	for(int v = 0;v<vecPack.size();v++)
	{
		itCnt = hCnt.find(vecPack[v].kAttr.attrib_id);	
		itAttr = hAttr.find(vecPack[v].kAttr.attr_name);
		if(itCnt!=hCnt.end())	
			vecPack[v].nTotalCnt = itCnt->second;
		if(itAttr!=hAttr.end())
		{
			if(itAttr->second.nCnt<vecPack[v].nTotalCnt)
			{
				itAttr->second.Id = vecPack[v].kAttr.attrib_id;		
				itAttr->second.nCnt = vecPack[v].nTotalCnt;
			}
		}
		else
		{
			kNameToCnt.nCnt = vecPack[v].nTotalCnt;
			kNameToCnt.Id = vecPack[v].kAttr.attrib_id;
			hAttr.insert(make_pair(vecPack[v].kAttr.attr_name,kNameToCnt));	
		}

	}
	
	//delete repeat attr name
	for(int ve = 0;ve<vecPack.size();ve++)
	{
		itAttr = hAttr.find(vecPack[ve].kAttr.attr_name);
		if(itAttr!=hAttr.end())		
		{
			if(vecPack[ve].kAttr.attrib_id!=itAttr->second.Id)
			{
				vecPack[ve].kAttr.attrib_priority = 999999;
				vecPack[ve].nTotalCnt = -1;
			}

		}
	}
	
	sort(vecPack.begin(),vecPack.end());
	
	/*for(int k = 0;k<vecPack.size();k++)
	{
		cout<<"attrib_id: "<<vecPack[k].kAttr.attrib_id<<"   "<<
			"nTotalCnt: "<<vecPack[k].nTotalCnt<<" sub: "<<vecPack[k].kAttr.attrib_priority<<endl;
	}*/
	//show attr info
	str += "<Attrib>";
	str += "<items>";
	int a = 0;
	while(a!=vecPack.size())
	{
		if(vecPack[a].kAttr.attrib_priority==999999)	
			goto OUT;
		attr_id = vecPack[a].kAttr.attrib_id;
		str+="<item>";
		str+="<Name>";
		//str+=vecPack[a].kAttr.attr_name;
		xh.XmlEncode(vecPack[a].kAttr.attr_name,str);	
		str+="</Name>";
		str+="<ID>";
		sprintf(chBuf,"%d",vecPack[a].kAttr.attrib_id);
		str+=chBuf;
		str+="</ID>";
		str+="<Category>";
		sprintf(chBuf,"%d",vecPack[a].kAttr.catid);
		str+=chBuf;
		str+="</Category>";
		str+="<Priority>";
		sprintf(chBuf,"%d",vecPack[a].kAttr.attrib_priority);
		str+=chBuf;
		str+="</Priority>";
		str+="<Count>";
		sprintf(chBuf,"%d",vecPack[a].nTotalCnt);
		str+=chBuf;
		str+="</Count>";
		//str+="</item>";
		str+="<Values>";
		str+="<items>";
		while(a<vecPack.size() && attr_id == vecPack[a].kAttr.attrib_id)
		{
			if(!strlen(vecPack[a].kSubAttr.sub_attrib_name))
			{
				a++;	
				continue;
			}
			str+="<item>";
			str+="<Name>";
			//str+=vecPack[a].kSubAttr.sub_attrib_name;
			xh.XmlEncode(vecPack[a].kSubAttr.sub_attrib_name,str);	
			str+="</Name>";
			str+="<ID>";
			sprintf(chBuf,"%d",vecPack[a].kSubAttr.sub_attrib_id);
			str+=chBuf;
			str+="</ID>";
			str+="<Priority>";
			sprintf(chBuf,"%d",vecPack[a].kSubAttr.sub_attrib_priority);
			str+=chBuf;
			str+="</Priority>";
			str+="<Count>";
			sprintf(chBuf,"%d",vecPack[a].cnt);
			str+=chBuf;
			str+="</Count>";
			str+="</item>";

			a++;
		}
		str+="</items>";
		str+="</Values>";
		str+="</item>";
	}
OUT:
	str+="</items>";
	str+="</Attrib>";

}

void CQueryResultPack::ShowShopGroupInfo(vector<SGroupByInfo> &vGpRes,string& str,vector<SResult>& vRes)
{
	if(!vGpRes.size())
	{
		return ;
	}

	CXmlHttp xh;	
	//shop group info (limit number 5)
	int iSize = vGpRes.size()>SHOP_INFO_LIMIT? SHOP_INFO_LIMIT:vGpRes.size();
	int iDangCnt = 0;
	int iShopCnt = 0;
	char chBuf[256];
	int iTotalCnt =vRes.size();

	str+="<ShopGroupInfo>";
	str+="<items>";

	for(int i = 0;i<iSize;i++)
	{
		if(m_ShopInfoHash[vGpRes[i].nGid].shop_name[0]=='\0')
		{
			continue;
		}
		//show group info
		str+="<item>";
		str+="<ShopID>";
		sprintf(chBuf,"%d",vGpRes[i].nGid);
		str+=chBuf;
		str+="</ShopID>";
		str+="<ShopName>";
		xh.XmlEncode(m_ShopInfoHash[vGpRes[i].nGid].shop_name,str);	
		//str+=m_ShopInfoHash[vGpRes[i].nGid].shop_name;
		str+="</ShopName>";
		str+="<Count>";
		sprintf(chBuf,"%d",vGpRes[i].nCnt);
		str+=chBuf;
		str+="</Count>";
		str+="</item>";
	}

	str+="</items>";

	if(SHOP_INFO_LIMIT>=vGpRes.size())
	{
		str+="</ShopGroupInfo>";
		return ;
	}

	str+="<ShopList>";
	sprintf(chBuf,"%d",vGpRes[iSize].nGid);
	str += chBuf;

	for(int l = iSize+1;l<vGpRes.size();l++)
	{
		sprintf(chBuf," %d",vGpRes[l].nGid);
		str+=chBuf;
	}
	str+="</ShopList>";
	str+="</ShopGroupInfo>";

}

void CQueryResultPack::ShowDangDangAndShopNum(vector<SGroupByInfo> &vGpRes,
		string& str,vector<SResult>& vRes)
{
	if(!vGpRes.size())
	{
		return ;
	}
	int iDangCnt = 0;
	int iShopCnt = 0;
	int iTotalCnt = 0;
	char chBuf[256];
	for(int i = 0; i<vGpRes.size();i++)
	{
		if(vGpRes[i].nGid ==1)
			iDangCnt = vGpRes[i].nCnt;
		else
			iShopCnt =  vGpRes[i].nCnt;
	}
	iTotalCnt = iDangCnt + iShopCnt;

	str+="<DangDangCount>";
	sprintf(chBuf,"%d",iDangCnt);
	str+=chBuf;
	str+="</DangDangCount>";
	str+="<DangDangShowCount>";
	sprintf(chBuf,"%d",iDangCnt);
	str+=chBuf;
	str+="</DangDangShowCount>";

	str+="<ShopShowCount>";
	sprintf(chBuf,"%d",iShopCnt);
	str+=chBuf;
	str+="</ShopShowCount>";
	str+="<ShopTotalCount>";
	sprintf(chBuf,"%d",iTotalCnt);
	str+=chBuf;
	str+="</ShopTotalCount>";

}

bool CQueryResultPack::LoadAttrData()
{
	int iLen = 0;

	//load static hash file
	u64 data = 0xffffffff;
	string file_name = m_strModulePath+ATTR_DATA_RDX;
	if(!m_AttrInfoHash.load_serialized_hash_file(file_name.c_str(),data))
	{
		COMMON_LOG(SL_ERROR,"load attrib info error!!");
	    return false;
	}

	//load index file
	file_name = m_strModulePath+ATTR_DATA_IDX;
	if(!LoadStruct(file_name.c_str(),m_vecIdxData,false))
	{
		COMMON_LOG(SL_ERROR,"attr_data.rdx open error!!");
		return false;
	}

	//load data file
	file_name = m_strModulePath+ATTR_DATA_DAT;
	if(!LoadStruct(file_name.c_str(),m_vecDatData,false))
	{
		COMMON_LOG(SL_ERROR,"attr_data.dat open error!! ");
		return false;
	}

	return true;
}

