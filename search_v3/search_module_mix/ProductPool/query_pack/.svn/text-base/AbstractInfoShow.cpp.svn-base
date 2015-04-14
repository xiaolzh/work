#include "AbstractInfoShow.h"

CAbstractInfo::CAbstractInfo()
{
	interpunctionOffset.clear();
	interpunctionList.clear();
	vecWindList.clear();
}

void CAbstractInfo::InitUnDealSet()
{
	const char *list[] = {"。", "；", "，", "！", "？", ",", ".", "?", "!", " ", "\r", "\n","、",":", "：", ";",  "/", "class", "font", "=", "\\", "dot", "<", ">"};
	string src, dest;
	for(int i = 0; i < (int)(sizeof(list)/sizeof(list[0])); i++)
	{
		src = list[i];

		//_UTF8_to_GBK_(src, dest);
		interpunctionList.push_back(dest);
	}
}

void CAbstractInfo::Init()
{
	interpunctionOffset.clear();
	interpunctionList.clear();
	vecWindList.clear();
}
void CAbstractInfo::GetInterpunctionOffset(string& str)
{

	int iPos = -1;	
	int iTime = 0;
	string tmpStr;	
	for(int i = 0; i != (int)interpunctionList.size(); i++)
	{
		tmpStr = str;	
		while((iPos = tmpStr.find_last_of(interpunctionList[i]))!=string::npos)
		{
			KPunc temp;
			temp.pos = iPos;
			if (interpunctionList[i].length() == ALL_TAG_SIZE)
			{
				temp.interSize = ALL_TAG_SIZE;
			}
			else if(interpunctionList[i].length() == HALF_TAG_SIZE)
			{
				temp.interSize = HALF_TAG_SIZE;
			}
			else
				return ;
			interpunctionOffset.push_back(temp);
			tmpStr.assign(str.c_str(),iPos-1);
		}

	}
	
	sort(interpunctionOffset.begin(), interpunctionOffset.end());
}

// get sub abstract
void CAbstractInfo::MakeAbstractWind(string& str)
{
	int iSize = interpunctionOffset.size();	
	int iLen = str.length();	
	int iPos = 0;	
	KWind kWind;
	memset(&kWind,0x0,sizeof(KWind));
	vecWindList.clear();

	if(!iSize)
	{
		kWind.start		= 0;	
		kWind.end		= MAX_TEXT_LENGTH; 
		vecWindList.push_back(kWind);
		return;
	}
	
	//get the started and ended pos of abstract 	
	for(int i = 0;i<iSize;i++)
	{
		memset(&kWind,0x0,sizeof(KWind));	

		/*if(iPos=0|| iPos<MAX_TEXT_LENGTH)
		{
			kWind.start = 0;	
			kWind.end = MAX_TEXT_LENGTH; 
			//kWind.end = iPos; 
		}
		else*/
		{
			kWind.start = iPos+interpunctionOffset[i].interSize;	
			// last interpunction	
			if(kWind.start >iLen)
			{
				break;
			}

			if(iPos+MAX_TEXT_LENGTH<=iLen)
			{
				kWind.end = iLen;
			}
			else
			{
				kWind.end =	iPos+interpunctionOffset[i].interSize+MAX_TEXT_LENGTH; 
			}
		}

		vecWindList.push_back(kWind);
	}
	
}

int CAbstractInfo::ComputeAbstractWind(string& str,VEC_PR_II& vpii)
{
	int iWindSize = vecWindList.size();	
	int iKeySize  = vpii.size();
	int iMaxScore = -1; 
	int iMaxPos   = -1;

	//1.compute every window's score	
	for(int i =0; i<iWindSize;i++)
	{
		for(int j = 0;j<iKeySize;j++)
		{
			if(vpii[j].first>=vecWindList[i].start &&
					vpii[j].first<vecWindList[i].end)
			{
				vecWindList[i].wordKind++;
				vecWindList[i].score++;
			}
		}
	}


	//2.get max score and the window's pos	
	for(int m =0;m<iWindSize;m++)
	{
		if(iMaxScore<vecWindList[m].score)
		{
			iMaxScore = vecWindList[m].score;
			iMaxPos   =  m;
		}
	}

	//3.store the max window
	return iMaxPos;	
}

int CAbstractInfo::GetSubAbstract(string& input, int maxLen,string& rs)
{
	//	string rs;
	char szChar[]="<font class=dot>...</font>";	
	int iLen = strlen(szChar);	
	if (maxLen < 3)
	{
		// 将全部字符串清空
		return rs.length();
	}

	int curLen = 0;
	if (input.size() == 0)
	{
		return rs.length();
	}
	else
	{
		char curChar = input[curLen];

		while (curLen<maxLen && curLen<(int)input.size())
		{
			if (curLen >= maxLen-3)
			{
				rs += "<font class=dot>...</font>";
				return rs.length()-iLen; 
			}   
			else if (curLen == maxLen-4)
			{   
				if (IsHalf(curChar) == true)
				{   
					rs += curChar;
					curLen += 1;
				}   
				else
				{   
					rs += "<font class=dot>...</font>";
					return rs.length()-iLen; 
				}   
			}  
			else
			{
				if (IsHalf(curChar) == true)
				{
					rs += curChar;
					curLen += 1;
				}
				else
				{
					rs += curChar;
					curLen += 1;
					curChar = input[curLen];
					rs += curChar;
					curLen += 1;
				}
			}

			curChar = input[curLen];
		}
	}

	return rs.length()-iLen;
}


bool CAbstractInfo::Process(vector< string >& vecRed,string& strIn,string& strOut)
{
	//init
	Init();

	InitUnDealSet();	
	int iTime = 0;

	//query term remove punctuation	
	vector <string>::iterator itVec = vecRed.begin();	
	int iPuncSize = interpunctionList.size();

	bProcess = true;
	while(itVec!=vecRed.end())
	{
		for(int j=0;j<iPuncSize;j++)
		{
			if(*itVec==interpunctionList[j])
			{
				vecRed.erase(itVec);
				break;
			}
		}
		itVec++;
	}
	#ifdef AB 
	    TimeUtil t_int;
	#endif 

	if(!(Init(vecRed,strIn,strOut)))
	{
		return false;
	}
	#ifdef AB
	iTime = t_int.getPassedTime();
	cout<<"RedText cost: "<<iTime<<endl;
	#endif
	return bProcess;		
}

bool CAbstractInfo::Init(vector< string >& vecRed,string& strIn,string& strOut)
{
	int iPos = -1;	
	int iLen = -1;
	int iTime = 0;
	//vecRed process
	for(int m = 0;m<vecRed.size();m++)
	{
		QueryProcess(vecRed[m]);		
	}
	
	//1.load vecRed to datrie tree
	vector<string> vec=vecRed;
	sort(vec.begin(), vec.end());
	vec.erase(unique(vec.begin(), vec.end()),  vec.end());
	VEC_PR_SD vprsd;
	vprsd.reserve(vprsd.size());
	for(int i=0;i<vec.size();++i)
	{
		vprsd.push_back(make_pair(vec[i], i));
	}
	CDic_w dic;
	dic.LoadDicFromMem(vprsd,false);

	//2.get key's pos from text field

	string strTmp = strIn;
	QueryProcess(strTmp);
	
	VEC_PR_II vpii;

	dic.SearchKeyWordInDicEx((char*)strTmp.c_str(), (char*)strTmp.c_str()+strTmp.length(), vpii);
	if (vpii.empty())
	{
		strOut=strIn;
		//return true;
		return false;
	}

	//3.compute text length, if length larger than MAX_TEXT_LENGTH,then choose the sentence which
	// the score is the highest ,else mark the whole sentence
	if(strIn.length()<=MAX_TEXT_LENGTH)
	{
		RedText(strIn,vpii,strOut);	
		return true;	
	}

	//4. get the position of the punctuation
	GetInterpunctionOffset(strIn);	

	//5.make the window of sentences
	MakeAbstractWind(strIn);

	//6.compute the score of sentence

	if((iPos = ComputeAbstractWind(strIn,vpii))<0)		
	{
		return false;	
	}
	  
	//7.auto abstract generate
	string result;	
	result.assign(strIn,vecWindList[iPos].start,vecWindList[iPos].end);
	string rs;
	if(!(iLen= GetSubAbstract(result,MAX_TEXT_LENGTH,rs)))
	{
		return true;	
	}

	vecWindList[iPos].end = vecWindList[iPos].start+iLen;	
	RedText(rs,vpii,iPos,strOut);	
	
	return true;
}


void CAbstractInfo::RedText(string& str,VEC_PR_II& vpii,int iPos,string& strOut)
{
	const char* ps=str.c_str();
	const char* pt=ps;
	

	for(int i=0;i<vpii.size();i++)
	{
		if(vpii[i].first>=vecWindList[iPos].start && vpii[i].second<=vecWindList[iPos].end)	
		{

			strOut+=string(pt, ps+vpii[i].first-vecWindList[iPos].start);
			strOut+="<font class=\"skcolor_ljg\">";
			strOut+=string(ps+vpii[i].first-vecWindList[iPos].start,ps+vpii[i].second-vecWindList[iPos].start);
			strOut+="</font>";
			pt=ps+vpii[i].second-vecWindList[iPos].start;
			bProcess = true;
		}
	}

	//strOut+=string(ps+vpii.back().second,ps+str.length());
	strOut+=string(pt,ps+str.length());	
}

void  CAbstractInfo::RedText(string& str,VEC_PR_II& vpii,string& strOut)
{

	const char* ps=str.c_str();
	const char* pt=ps;

	for (int i=0;i<vpii.size();++i)
	{
		strOut+=string(pt, ps+vpii[i].first);
		strOut+="<font class=\"skcolor_ljg\">";
		strOut+=string(ps+vpii[i].first,ps+vpii[i].second);
		strOut+="</font>";
		pt=ps+vpii[i].second;
		bProcess = true;
	}
	strOut+=string(ps+vpii.back().second,ps+str.length());

}

void CAbstractInfo::QueryProcess(string& str)
{
	int iLen = str.length();		
	//vector<char> vec(&str[0],&str[iLen-1]+1);
	vector<char> vec;
	vec.assign(str.begin(),str.end());
	vec.push_back(0);
	char* pchBuf = &vec[0];
	char* pchBufSrc = pchBuf;
	while (*pchBuf!=0)
	{
		//chinese word	
		if (*pchBuf<0&&pchBuf[1]!=0&&IsGBKWord(*pchBuf,pchBuf[1])<HALFGBK)
		{
			pchBuf+=2;
		}
		//english word	
		else
		{
			if(*pchBuf>='A' && *pchBuf<='Z')
			{
				*pchBuf = *pchBuf + 32;		
			}
			++pchBuf;
		}				
	}
	
	str = pchBufSrc;
	//str.assign(vec.begin(),vec.end());
	return;
}

