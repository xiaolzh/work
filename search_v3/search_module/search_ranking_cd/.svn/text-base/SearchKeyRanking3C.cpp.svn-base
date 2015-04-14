#include "SearchKeyRanking.h"
#include <iostream>
using namespace std;

void CSearchKeyRanking::SingleRank3C(CDDAnalysisData* pa, SMatchElement& me, SResult& rt) 
{
	//基本信息
	int docID = rt.nDocId;

	//字段信息
	int fieldScr = 0;									//字段得分[0-4]
	bool ifContainAll_T = false;			//是否在重要字段出现
	bool ifContainAll_ti = false;			//标题是否出现
	bool ifContainAll_syn = false;		//在同义标题出现(仅同义字段)
	bool ifContainAll_bottom = false;	//在底级类别中出现
	bool ifContainAll_upper = false;	//在高级类别中出现
	bool ifContainAll_sub= false;	//在副标题中出现
	bool ifInBrand = false;						//品牌字段支持
	bool ifDisMatch = true;					  //词距是否匹配
	{
		vector<SFieldOff> fieldPtr = me.vFieldsOff; //出现在哪些字段
		int ivtCnt = fieldPtr.size();
		int fid, fieldNum;
		for (int cnt = 0; cnt < ivtCnt; ++cnt)
		{
			fid = fieldPtr[cnt].field;
			if (m_fid2fi[fid] >= 0)
			{
				fieldNum = m_fid2fi[fid];
				if (fieldNum >= field_id[TMINNUM].id)
				{
                                        //主区
					ifContainAll_T = true;

					if (fieldNum == field_id[TITLENAME].id)
					{ 
                                                //标题
						ifContainAll_ti = true;
					}
					else if (fieldNum == field_id[TITLESYN].id)
					{ 
                                                //同义扩展
						if (false == ifContainAll_ti)
						{
							ifContainAll_syn = true;
						}
                    }
					else if (fieldNum == field_id[BOTTOMCATE].id)
					{ 
                                                //底级类
						ifContainAll_bottom = true;
					}
					else if(fieldNum == field_id[SUBNAME].id)
					{
						ifContainAll_sub = true;
					}
					else if (fieldNum == field_id[BRAND].id)
					{	//品牌字段支持
						ifInBrand = true;
					}
				}
			}
		}
                
		if (ifContainAll_T)
		{
			if (ifContainAll_ti)
			{
				fieldScr = 4;
			}
			else if(ifInBrand)
			{
				fieldScr = 1;
				float queBrd = (float)me.vTerms[0].len / me.vFieldLen[0];
				if(1 == (me.vFieldLen[0] % 2) || queBrd >= 0.6)
				{
					fieldScr = 4;
				}
				else if(queBrd >= 0.3)
				{
					fieldScr = 3;
				}
			}
			else if(ifContainAll_syn)
			{
				fieldScr = 3;
			}
			else if(ifContainAll_bottom)
			{
				fieldScr = 2;
			}
			else if(ifContainAll_sub)
			{
				fieldScr = 1;
			}
		}
	}
	if(0 == fieldScr)
	{
		return;
	}

	//特殊信息(类别反馈/单品反馈/产品中心)
	int relScr = 0;						//精简相关标识[0-2]
	int fbCateWeight = 0;
	int fbPidWeight = 0;
	int pdtCoreWeight = 0;
	{
		ComputeSpecialWeight3C(
			docID, pa, fbCateWeight, fbPidWeight, pdtCoreWeight, relScr);
	}
        
        //单品反馈  类别反馈 中心词
        bool hasProductBack = false, hasCatBack = false, hasCore=false;
		//取消对单品反馈打分
        /*if(fbPidWeight>0)
        {
            hasProductBack=true;
        }*/
        
        /*if(fbCateWeight>0)
        {
            hasCatBack=true;
        }*/
        
        if(pdtCoreWeight > 0)
        {
            hasCore=true;
        }
        
        //商业因素
        int comScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
	    ComputeCommerceWeight3C(rt.nDocId, comScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);	
        int valueForCommerce = comScr + salNumScr + commentScr;

        rt.nScore = (fbPidWeight<<28)+(fieldScr<<24)+(3<<22)+(hasCore<<21)+(ifInBrand<<20)+(hasCatBack<<16)+(1<<15)+(ifHavImg<<14)+ (salPreScr<<6)+ (valueForCommerce<<1)+ifPub;
        
        //if(fieldScr > 0)
        //{
            //基本相关
            rt.nScore += (1<<27);
        //}

		ChangeWeight3C(rt, pa);
	return;
}

void CSearchKeyRanking::MultiRank3C(CDDAnalysisData* pa, SMatchElement& me, SResult& rt) {
	//cout << "enter MultiRank..." << endl;
	int querySz = (int)me.vTerms.size();


	//基本信息
	int docID = rt.nDocId;

	//字段信息
	int fieldScr = 0;									//字段得分[0-4]
	bool ifContainAll_T = false;			//重要字段是否出全
	bool ifContainAll_ti = false;			//标题是否出全(仅标题)
	bool ifContainAll_syn = false;		//标题是否出全(标题+同义扩展)
	bool ifContainAll_bottom = false;	//底级类别中是否出全
	bool ifContainAll_upper = false;	//高级类别中是否出全
	bool ifInBrand = false;						//品牌字段支持
	//bool ifDisMatch = true;						//词距是否匹配（标题）
	//{
		int sz_T = 0, sz_ti = 0, sz_sys = 0;	//主区/标题/仅同义标题
		int sz_b = 0, sz_ob = 0, sz_u = 0; 	  //底级类/仅底级类/高级类出词数
		//vector<vector<int> > vPosTi(querySz);	//标题出词位置记录
                vector<int> posInTi;	//以前是vector<vector<int> >类型，这样会有问题吗？
                posInTi.resize(querySz);//张平
		{
			//Invert* ivtAddr;
			int ivtCnt;
			int fid, fieldNum;
			bool added_T, added_ti, added_syn, added_b, added_u;

			for (int k = 0; k < querySz; ++k)
			{	//for one term
				added_T = added_ti = added_syn = added_b = added_u = false;

                                fid = me.vFieldsOff[k].field;
                                if (m_fid2fi[fid] >= 0)
                                {
                                        fieldNum = m_fid2fi[fid];
                                        if (fieldNum >= field_id[TMINNUM].id)
                                        {
                                                //主区
                                                ++sz_T;
                                                
                                                if (fieldNum == field_id[TITLENAME].id)
                                                { 
                                                        //标题
                                                        ++sz_ti;
                                                        
                                                        posInTi[k] = me.vFieldsOff[k].off;
                                                        //vPosTi[k].push_back(me.vFieldsOff[k].off);
                                                }
                                                else if (fieldNum == field_id[TITLESYN].id)
                                                {
                                                        //同义扩展
                                                        ++sz_sys;
                                                }
                                                else if (fieldNum == field_id[BOTTOMCATE].id)
                                                { 
                                                        //底级类
                                                        ++sz_b;
                                                        ++sz_ob;
                                                }
                                                else if (fieldNum == field_id[BRAND].id && 2 == pa->m_AnalysisPart.type[k])
                                                        //(pa->m_AnalysisPart.ifPdtQuery || 2 == pa->m_AnalysisPart.type[k]))
                                                {
                                                        //品牌支持
                                                        ifInBrand = true;
                                                        ++sz_ti;
                                                }
                                        }
                                }
			}
		}
                
		if (sz_T == querySz)
		{
			ifContainAll_T = true;
			if (sz_ti == querySz)
			{
				ifContainAll_ti = true;
				fieldScr = 4;
			}
			else if (sz_ti + sz_sys == querySz)
			{
				ifContainAll_syn = true;
				fieldScr = 3;
			}
			else if (sz_b == querySz)
			{
				ifContainAll_bottom = true;
				fieldScr = 3;
			}
			else if (sz_ti + sz_sys + sz_ob == querySz)
			{
				fieldScr = 2;
			}
			else if (sz_u == querySz)
			{
				ifContainAll_upper = true;
				fieldScr = 1;
			}
		}
		if(0 == fieldScr)
		{
			return;
		}

	//判断词距是否匹配
	int disScr = 3;
	const char* key = pa->m_AnalysisPart.key.c_str();
	if(pa->m_AnalysisPart.needJudgeDis)
	{
		int termScr, sumDisScr = 0;
		//if(ifAllIn_ti)
		{
			int offSize;
			int noSameArea = 0;
			for(int i = 1; i < querySz; i++)
			{
				if(0 == pa->m_AnalysisPart.dis[i] && 0 == pa->m_AnalysisPart.type[i-1] && 0 == pa->m_AnalysisPart.type[i])
				{
			       if( me.vFieldsOff[i-1].field != me.vFieldsOff[i].field )
				   {
				        ++noSameArea;
						continue;
				   }
                    

					if(0 == isalnum(key[me.vTerms[i-1].pos]) && 0 == isalnum(key[me.vTerms[i].pos]))	//前后两个词项都为中文
					{
						offSize = posInTi[i] - posInTi[i-1];
						if(offSize < 0)
						{
							if(me.vTerms[i-1].len <= 2 || me.vTerms[i].len <= 2)
							{
								termScr = 0;
							}
							else 
							{
								termScr = 1;
							}
						}
						else
						{
							if(me.vTerms[i-1].len <= 2 && me.vTerms[i].len <= 2)
							{
								termScr = (offSize - me.vTerms[i-1].len == 0) ? 3 : 0;
							}
							else if(offSize - me.vTerms[i-1].len == 0)
							{
								termScr = 3;
							}else if(offSize - me.vTerms[i-1].len <= 2)
							{
								termScr = 2;
							}
							else
							{
								termScr = 1;
							}
						}
						sumDisScr += termScr;
					}
					else if(0 != isalnum(key[me.vTerms[i-1].pos]) && 0 != isalnum(key[me.vTerms[i].pos]))	//前后两个词项为英文数字
					{
						offSize = posInTi[i] - posInTi[i-1];
						if(offSize > 0 && (offSize - me.vTerms[i-1].len) <= 2)
						{
							sumDisScr += 3;
						}
					}
					else	//前后两个词项为中英文或者中文数字
					{
						if(isalpha(key[me.vTerms[i-1].pos]) || isalpha(key[me.vTerms[i].pos]))	//前后两个词项是中英文
						{
							offSize = posInTi[i] - posInTi[i-1];
							//两个查询词在字段与查询串中的顺序相同且英文词的长度为1
							if(offSize > 0 && (1 == me.vTerms[i-1].len || 1 == me.vTerms[i].len))
							{
								if(0 == (offSize - me.vTerms[i-1].len))
								{
									sumDisScr += 3;
								}
							}
						}
						else	//前后两个词项是中文数字
						{
							sumDisScr += 3;
						}
					}
				}
				else
				{
					sumDisScr += 3;
				}
			}
			disScr = sumDisScr / (querySz- 1);	//确信 queSize - 1 != 0
			if(noSameArea == querySz-1)
				disScr=3;
		}
                
	}

	
	//特殊信息(类别反馈/单品反馈/产品中心)
	int relScr = 0;						//精简相关标识[0-2]
	int fbCateWeight = 0;
	int fbPidWeight = 0;
	int pdtCoreWeight = 0;
	{

		ComputeSpecialWeight3C(
			docID, pa, fbCateWeight, fbPidWeight, pdtCoreWeight, relScr);
	}
        
        
                //单品反馈  类别反馈 中心词
        bool hasProductBack = false, hasCatBack = false, hasCore=false;
		//取消对单品反馈、分类反馈打分
        /*if(fbPidWeight>0)
        {
            hasProductBack=true;
        }*/
        
        /*if(fbCateWeight>0)
        {
            hasCatBack=true;
        }*/
        
        if(pdtCoreWeight > 0)
        {
            hasCore=true;
        }
        
        //商业因素
        int comScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
	    ComputeCommerceWeight3C(rt.nDocId, comScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);	
        
        int valueForCommerce = comScr + salNumScr + commentScr;
        rt.nScore = (fbPidWeight<<28) + (fieldScr<<24) + (disScr<<22) + (hasCore<<21) + (ifInBrand<<20) + (hasCatBack<<16) + (1<<15) + (ifHavImg<<14) + (salPreScr<<6) + (valueForCommerce<<1) + ifPub;
		//if(ifContainAll_T)
		//{
            //基本相关
            rt.nScore += (1<<27);
		//}

		ChangeWeight3C(rt, pa);
	return;
}

void CSearchKeyRanking::ComputeCommerceWeight3C(const int docId, int& salAmtScr, int& ifPub, 
	int& salNumScr, int& salPreScr, int& ifHavImg, int& commentScr)
{
	salAmtScr = ifPub = salNumScr = salPreScr = ifHavImg = commentScr = 0;

	//计算商业因素、销量细节得分
	int salDayAmt = m_funcFrstInt(m_saleDayAmtProfile, docId);
	int salWeekAmt = m_funcFrstInt(m_saleWeekAmtProfile, docId);
	int salMonAmt = m_funcFrstInt(m_saleMonthAmtProfile, docId);
	int salDayNum = m_funcFrstInt(m_saleDayProfile, docId);
	int salWeekNum = m_funcFrstInt(m_saleWeekProfile, docId);
	int salMonNum = m_funcFrstInt(m_saleMonthProfile, docId);
	int simiWeekAmt = (5 * salDayAmt + 7 * salWeekAmt + salMonAmt) / 12;	//近似周销售额
	int simiWeekNum = (5 * salDayNum + 7 * salWeekNum + salMonNum) / 12;	//近似周销量
	if(simiWeekAmt < 0 || simiWeekNum < 0)
	{
		return;
	}
	if(simiWeekAmt < 10000)
	{
		//商业因素得分最高为12分，该公式计算结果最高为9
		//salAmtScr = (5 + (int)floor(1/(1 + exp((-1) * log10(0.1 * simiWeekAmt) + 2)) * 120)) / 10;
		salAmtScr = m_salAmtScr[simiWeekAmt];
	}
	else if(simiWeekAmt < 20000)
	{
		salAmtScr = 10;
	}
	else if(simiWeekAmt < 100000)
	{
		salAmtScr = 11;
	}
	else
	{
		salAmtScr = 12;
	}

	//是否是公用品
	ifPub = m_funcFrstInt(m_isShareProductProfile, docId);
#ifdef DEBUG
	if(ifPub != 0)
		cout << "ComputeCommerceWeight3C::ifPub: " << ifPub << endl;
#endif

	if(simiWeekNum < 1000)
	{
		//销量得分最高为10分，该公式计算结果最高为8
		//salNumScr = (5 + (int)floor(1/(1 + exp((-1) * log10(0.5 * simiWeekNum) + 1)) * 100)) / 10;
		salNumScr = m_salNumScr[simiWeekNum];
	}
	else if(simiWeekNum < 5000)
	{
		salNumScr = 9;
	}
	else
	{
		salNumScr = 10;
	}

	//计算销售预测得分
	float avgDayAmt = (3 * salDayAmt + salWeekAmt) / 10;
	float avgWeekAmt = (3 * salWeekAmt + salMonAmt) / 50;
	float avgMonAmt = salMonAmt / 30;
	if(avgMonAmt < avgWeekAmt && avgWeekAmt < avgDayAmt)
	{
		salPreScr = (2 * salNumScr + 5) / 10 + 1;
	}
	else if(avgMonAmt < avgWeekAmt || avgWeekAmt < avgDayAmt)
	{
		salPreScr = 2 * salNumScr / 10 + 1;
	}
	else
	{
		salPreScr = 2 * salNumScr / 10;
	}

	//是否有图片
	ifHavImg = m_funcFrstInt(m_numImagesProfile, docId) > 0 ? 1 : 0;

	//计算评论数得分
	int commentCnt = m_funcFrstInt(m_totalReviewCountProfile, docId);
	if(commentCnt <= 0)
	{
		commentScr = 0;
	}
	else if(commentCnt < 100)
	{
		//评论数得分最高为9分,该公式计算结果最高为7
		//commentScr = (5 + (int)floor(1/(1 + exp((-1) * log10(0.5 * commentCnt) + 1)) * 100)) / 10;
		commentScr = m_commentScr[commentCnt];
	}
	else if(commentCnt < 1000)
	{
		commentScr = 8;
	}
	else
	{
		commentScr = 9;
	}
}

void CSearchKeyRanking::BrandQRank3C(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
    //进入该函数到前提：输入分词结果是  整个串是品牌词、处理过后的串是品牌词或者分开到词汇都是品牌词汇
	//例如：妖精的口袋---妖精 口袋  但是妖精的口袋是品牌  根本上是分词的事情
	//      阿迪小达斯---阿迪达斯   防止了用户到误输入
	//      苹果 nokia---多品牌词汇到输入
	//
	//      特别处理妖精的口袋问题，分词结果都不是品牌，那么要求两个词汇必须在一个字段出现
	
	//品牌搜索相关度计算
	//基本信息
        float queBrd = 0.0;//能够解决分词遗留的很多问题，品牌字段的占比很重要
	int docID = rt.nDocId;

	//字段信息
	int queLen, fieldLen, fieldScr = 0;		//字段得分[0-3]
	bool ifInTitle = false;				//是否出在标题
	bool ifInSyn = false;				//是否出在同义扩展
	bool ifInBrand = false;	
    int disScr=3;
    //是否出在品牌
	{
		int ivtCnt, cnt;
		int fid, fieldNum;
        int bscore =4;
		int querySize = me.vTerms.size();
		int size=0;//判断分词结果都是非品牌词
		for (int k = 0; k < querySize; ++k)
		{
            //刘向前修改部分
            //for one term
			queLen = me.vTerms[k].len;
            fieldLen = me.vFieldLen[k];
			if (2 == pa->m_AnalysisPart.type[k])
			{
				//品牌词
				//fid = ivtAddr[cnt].fieldID;
				fid = me.vFieldsOff[k].field;
                                
                 //注意顺序和添加break
                if (fid == m_vFieldIndex[BRAND])
				{
                    //品牌
					ifInBrand = true;
				}
				else if(fid == m_vFieldIndex[TITLENAME])
				{
                    //标题
					ifInTitle = true;
				}
				else if(fid == m_vFieldIndex[TITLESYN])
				{
                    //同义扩展
					ifInSyn = true;
				}
			}
			else//分词结果不是品牌
			{
		        size++;	
				fid = me.vFieldsOff[k].field;
                
                if(fid == m_vFieldIndex[BRAND])
				{
				    ifInBrand = true;
				}
				else if(fid == m_vFieldIndex[TITLENAME])
				{
				    ifInTitle = true;
				}else if(fid == m_vFieldIndex[TITLESYN])
				{
				    ifInSyn = true;
				}
			}
		}

		//都不是品牌词汇，出现在两个域都不能给分
        if(size == querySize)
		{
		   if((ifInBrand&&(ifInTitle||ifInSyn)) || (ifInTitle&&ifInSyn) )
		   {
		      ifInBrand=ifInTitle=ifInSyn=false;
		   }
		}
         

		fieldScr = ifInBrand ? bscore : (ifInTitle ? 3 : (ifInSyn ? 2 : 0));
		if(0 == fieldScr)
		{
			return;
		}
                
        //计算距离评分  进入该函数，如果第一个词为品牌，那么后续所有的词汇应该都是品牌,disSrc=3
        //如果品牌词汇被分词分开，那么主要处理这种情况
        if(pa->m_AnalysisPart.needJudgeDis&&2 != pa->m_AnalysisPart.type[0])
		{
			for(int i = 1; i < querySize; i++)
            {
                if(!((me.vFieldsOff[i-1].field == me.vFieldsOff[i].field) && (me.vFieldsOff[i-1].off < me.vFieldsOff[i].off) 
					&& (0 == (me.vFieldsOff[i].off - me.vFieldsOff[i-1].off - me.vTerms[i-1].len))))
				{
					disScr = 0;
					break;
				}
                            
			}
        }
	}

	//商业权重
	//int commerceScr = 0; //[0-6]
    int comScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
	int saleDetail = 0;
	{
        //ComputeCommerceWeight3C(docID, commerceScr, saleDetail);
        ComputeCommerceWeight3C(rt.nDocId, comScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);
	}
	//int sale = m_funcFrstInt(m_saleWeekProfile, docID);

	//相关类别记录
    /*bool havFBCate = false;
	size_t nCnt = 0;
	u64* ptr = (u64*)m_funcValPtr(m_clsProfile, docID, nCnt); //返回类别指针
	int cateNum = 0;		//记录底级类编号（cateStat的下标+1）
	int relScr = 0;			//精简相关标识[0-2]
	if (ifInBrand) //对品牌字段中的所有商品处理
	{
		relScr = 2;
		++pa->m_AnalysisPart.relCnt_brd; //在品牌字段中的商品数
		int fbSize = pa->m_AnalysisPart.vBrdCate.size();
		for (int k=0; k<nCnt; k++) 
        {
             //遍历该单品所有类别
			//注意这里的id与数据库中的不同，所以反馈数据的id要转化！！！
			u64 bottomCate = ptr[k];	//底级类
			for(int i = 0; i < fbSize; i++)
			{
				if (bottomCate == pa->m_AnalysisPart.vBrdCate[i].first)
				{
                    //已有此类别
                    //havFBCate = true;取消对反馈类打分
					u64 top_cate = GetClassByLevel(1, ptr[k]);
					pa->m_AnalysisPart.topCate_brd.insert(top_cate); //相关顶级类集合
					break;
				}
			}                      
		}
	}
	else if (ifInTitle)
	{
		relScr = 1;
		++pa->m_AnalysisPart.relCnt_ti;
		for (int k=0; k<nCnt; k++) { //遍历该单品所有类别
			u64 top_cate = GetClassByLevel(1, ptr[k]);
			pa->m_AnalysisPart.topCate_ti.insert(top_cate);//相关顶级类
		}
	}*/

        //相关度：文本匹配、词距、
        int valueForCommerce = comScr + salNumScr + commentScr;

		//rt.nScore = (fieldScr<<24) + (disScr<<22) + (ifInBrand<<20) + (havFBCate<<16) + (1<<15) + (ifHavImg<<14) + (salPreScr<<6) + (valueForCommerce<<1) + ifPub;
		rt.nScore = (fieldScr<<24) + (disScr<<22) + (ifInBrand<<20) + (1<<15) + (ifHavImg<<14) + (salPreScr<<6) + (valueForCommerce<<1) + ifPub;
        //if(fieldScr > 0)
			rt.nScore += 1<<27;

		ChangeWeight3C(rt, pa);
}


void CSearchKeyRanking::ComputeSpecialWeight3C(const int docId, CDDAnalysisData* pa,
    int& fbCateScr, int& fbPidScr, int& pdtCoreScr, int& relSpeScr)
{
    size_t cateCnt = 0;
    fbCateScr = fbPidScr = pdtCoreScr = relSpeScr = 0;
    u64* cates = (u64*)m_funcValPtr(m_clsProfile, docId, cateCnt);

#ifdef DEBUG
    char buf[30]={0};
    for(int i=0; i<cateCnt; i++)
    {
        TranseID2ClsPath(buf,cates[i],6);
        cout<<buf<<' ';
    }
    cout<<endl;
#endif
	/*int fbSize = pa->m_AnalysisPart.vCate.size();
	if(1 == fbSize)
	{
		if(0 != cateCnt)
		{
			int level = GetClsLevel(pa->m_AnalysisPart.vCate[0].first);
			u64 cateLev = GetClassByLevel(level, cates[0]);
			if(cateLev == pa->m_AnalysisPart.vCate[0].first)
			{
				fbCateScr = 1;
			}
		}
	}*/
    /*if(false == pa->m_AnalysisPart.vCate.empty())
    {
        int fbSize = pa->m_AnalysisPart.vCate.size();
        if(0 != cateCnt)
        {
            for(int i = 0; i < fbSize; i++)
            {
                if(cates[0] == pa->m_AnalysisPart.vCate[i].first)
                {
                    fbCateScr = 1;
                    u64 topCate = GetClassByLevel(1, cates[0]);
                    pa->m_AnalysisPart.topCate_fb.insert(topCate);
                    ++pa->m_AnalysisPart.relCnt_fb;
                    relSpeScr = 2;
                    break;
				 }
            }
        }
    }*/

    /*int pid = m_funcFrstInt64(m_isPidProfile, docId);
    if(false == pa->m_AnalysisPart.vPid.empty())
    {
        int fbSize = pa->m_AnalysisPart.vPid.size();
        for(int i = 0; i < fbSize; i++)
        {
            if(pid == pa->m_AnalysisPart.vPid[i])
            {
                fbPidScr = 1;
                fbCateScr = (fbCateScr == 0) ? 1 : fbCateScr;
                break;
            }
        }
    }*/

    int pid = m_funcFrstInt64(m_isPidProfile, docId);
    int pdtCoreCnt = m_pid2Core[pid].count;
    if((true == pa->m_AnalysisPart.ifPdtQuery) && (pdtCoreCnt != 0))
    {
        HASHVECTOR words = m_pid2Core[pid];
        for(int i = 0; i < pdtCoreCnt; i++)
        {
            if(0 == pa->m_AnalysisPart.pdtWord.compare(words.data + i * words.size))
            {
                pdtCoreScr = 1;
                /*if(0 != cateCnt)
                {
                    u64 topCate = GetClassByLevel(1, cates[0]);
					pa->m_AnalysisPart.topCate_prd.insert(topCate);
                }
                ++pa->m_AnalysisPart.relCnt_prd;
                relSpeScr = (relSpeScr == 0) ? 1 : relSpeScr;*/
                break;
            }
        }
    }
}

void CSearchKeyRanking::ComputeWeight3C(CDDAnalysisData* pa, SMatchElement& me, SResult& rt) {
	rt.nDocId = me.id;
	rt.nWeight = rt.nScore = 0;

	if (true == pa->m_AnalysisPart.ifBrdQuery) {
		BrandQRank3C(pa, me, rt);
	} else {
		//fprintf(stderr,"size=%d\n",me.vTerms.size());
		1 == me.vTerms.size()?
			SingleRank3C(pa, me, rt):
			MultiRank3C(pa, me, rt);
	}
}

void CSearchKeyRanking::ChangeWeight3C(SResult& rt, CDDAnalysisData* pa)
{
	int dateMod = m_funcFrstInt(m_modifyTime, rt.nDocId);	//表示最新上架时间距离1970年1月1日的秒数
	int dateCur = time(NULL);
	int modDays = (dateCur - dateMod) / (3600 * 24);		//最新上架时间距离现在的天数
#ifdef DEBUG
	cout<<"ChangeWeight3C::modDays: " << modDays << endl;
#endif
	int textRelScr = (rt.nScore >> TEXTRELBIT) & ScoreBitMap[TEXTRELBITNUM];
	int commerceScr = (rt.nScore >> COMMERCEBIT) & ScoreBitMap[COMMERCEBITNUM];
	int disScr = (rt.nScore >> TERMDISBIT) & ScoreBitMap[TERMDISBITNUM];
	if(textRelScr >= 3)
	{
		if(commerceScr <= 8)
		{
			if(modDays <= 7)
			{
				MoveLeftBit(rt.nScore, 7, COMMERCEBIT);
			}
			else if(modDays <= 15)
			{
				MoveLeftBit(rt.nScore, 2, COMMERCEBIT);
			}
		}
		if(0 == disScr)
		{
			rt.nScore -= (1 << TEXTRELBIT);
		}
	}
	else if(textRelScr >= 1 && commerceScr >= 15)
	{
		size_t cateCnt = 0, fbWeight = 0;
		u64* cates = (u64*)m_funcValPtr(m_clsProfile, rt.nDocId, cateCnt);
		if(false == pa->m_AnalysisPart.vCate.empty())
		{
			int fbSize = pa->m_AnalysisPart.vCate.size();
			if(0 != cateCnt)
			{
				for(int i = 0; i < fbSize; i++)
				{
					if(cates[0] == pa->m_AnalysisPart.vCate[i].first)
					{
						fbWeight = pa->m_AnalysisPart.vCate[i].second;
						break;
					}
				}
			}
		}
		if(fbWeight >= 6)
		{
			rt.nScore += (1 << TEXTRELBIT);
		}
	}
}
