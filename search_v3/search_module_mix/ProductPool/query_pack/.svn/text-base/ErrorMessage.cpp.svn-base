/*****************************************************************************
 ***author:			sunqian
 *** 
 ***date:			2012..5.28
 *** 
 *description:		catch the exception which may result to xml's error		

*****************************************************************************/

#include "QueryResultPacker.h"

bool CQueryResultPack::ErrorMessage(IAnalysisData* pad,GROUP_RESULT &vGpRes,string& strRes)
{
	hash_map<string,string>::iterator itCat ;

	if (pad == NULL) //url 解析时候出现了解析错误
	{    
		strRes+="<error>request_not_support</error>";
		return true;
	}    

	hash_map<string,string>::iterator itST = pad->m_hmUrlPara.find(ST);
	if(itST == pad->m_hmUrlPara.end())
	{    
		strRes+="<error>url must have st param !</error>";
		return true;
	}    

	/*if((itCat=pad->m_hmUrlPara.find(CATE_PATH))!=pad->m_hmUrlPara.end()||
			(itCat=pad->m_hmUrlPara.find(FILT_CATE_PATH))!=pad->m_hmUrlPara.end())
	{    
		if(itCat->second=="")
		{    
			strRes+="<error>category cannot be null or category_id  may write cat_paths!</error>";
			return true;
		}    
	}    */
	return false;

}
