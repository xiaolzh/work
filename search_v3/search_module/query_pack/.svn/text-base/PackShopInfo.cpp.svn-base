#include "QueryResultPacker.h"

void CQueryResultPack::ShowShopInfo(string &shop_id,string& str)
{
	char ch_buf[512];
	KShopIndex kShopIndex;
	memset(&kShopIndex,0x0,sizeof(KShopIndex));
	GetShopData(shop_id,kShopIndex);

	//show shop info
	str += "<Shop>";
	str += "<items>";
	for(int i = kShopIndex.strOff;i < kShopIndex.endOff;)
	{
		str += "<item>";
		str += "<Name>";
		str += m_vecShopData[i].name;
		str += "</Name>";
		str += "<shopID>";
		sprintf(ch_buf,"%d",m_vecShopData[i].shop_id);
		str += ch_buf;
		str += "</shopID>";
		str += "<Category>";
		str += m_vecShopData[i].catid;
		str += "</Category>";
		str += "</item>";
		str += "<Values>";
		str += "<items>";
		while(1)
		{
			if(strncmp(m_vecShopData[i].catid,m_vecShopData[i+1].catid,3) == 0)
			{
				i++;
				str += "<item>";
				str += "<Name>";
				str += m_vecShopData[i].name;
				str += "</Name>";
				str += "<shopID>";
				sprintf(ch_buf,"%d",m_vecShopData[i].shop_id);
				str += ch_buf;
				str += "</shopID>";
				str += "<Category>";
				str += m_vecShopData[i].catid;
				str += "</Category>";
				str += "</item>";
			}
			else
			{
				i++;
				break;
			}
		}
		str += "</items>";
		str += "</Values>";
	}
	str += "</items>";
	str += "</Shop>";
}

void CQueryResultPack::GetShopData(string& shop_id,KShopIndex& kShopIndex)
{
	int id = atoi(shop_id.c_str());
	//get index data
	kShopIndex.strOff = m_ShopIndexHash[id].strOff;
	if(-1 == kShopIndex.strOff)
		return;
	kShopIndex.endOff = m_ShopIndexHash[id].endOff;
}

void CQueryResultPack::GetProIndex(string& catid,KProIndex& kProIndex)
{
	int id = atoi(catid.c_str());
	kProIndex.strOff = m_ProIndexHash[id].strOff;

	if(-1 == kProIndex.strOff)
		return;
	kProIndex.endOff = m_ProIndexHash[id].endOff;
}

bool CQueryResultPack::LoadShopData()
{
	m_vecShopData.clear();
	int iLen = 0;

	//load static hash file 
	KShopIndex kShopIndex;
	kShopIndex.strOff = -1;
	string file_name = m_strModulePath+SHOP_INDEX_FILE_RDX;
	m_ShopIndexHash.load_serialized_hash_file(file_name.c_str(),kShopIndex);

	//load product index file
	KProIndex kProIndex;
	kProIndex.strOff = -1;
	file_name = m_strModulePath + PRODUCT_INDEX_FILE_RDX;
	m_ProIndexHash.load_serialized_hash_file(file_name.c_str(),kProIndex);

	//load shop data file
	file_name = m_strModulePath+SHOP_DATA_FILE_DAT;
	if(!LoadStruct(file_name.c_str(),m_vecShopData,false))
	{
		COMMON_LOG(SL_ERROR,"shop_data_file.dat open error!!");
		return false;
	}

	//read product data
	file_name = m_strModulePath+PRODUCT_SUBCAT_DATA_DAT;
	if(!LoadStruct(file_name.c_str(),m_vecProductData,false))
	{
		COMMON_LOG(SL_ERROR,"product_subcat_data_file.dat open error!!");
		return false;
	}
	return true;

}

