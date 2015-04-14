#include <math.h>
#include "SearchKeyRanking.h"

void CSearchKeyRanking::ComputeWeightCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	rt.nDocId = me.id;
	rt.nWeight = 0;
	rt.nScore = 0;
	if(true == pa->m_AnalysisPart.ifBrdQuery)
	{
#ifdef DEBUG
		cout<<"BrandQRankCloth"<<endl;
#endif
		BrandQRankCloth(pa, me, rt);	//计算品牌词查询权重
	}
	else if(1 == me.vTerms.size())
	{
#ifdef DEBUG
		cout<<"SingleRankCloth"<<endl;
#endif
		SingleRankCloth(pa, me, rt);	//计算单个词查询权重
	}
	else
	{
#ifdef DEBUG
		cout<<"MultiRankCloth"<<endl;
#endif
		MultiRankCloth(pa, me, rt);		//计算多个词查询权重
	}
}

void CSearchKeyRanking::BrandQRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//计算文本权重
	float queBrd = 0.0;
	int disScr = 3;
	bool ifInBrand = false, ifInTitle = false, ifInTitsyn = false;
	int fid, fieldNum, queLen, fieldLen, fieldScr = 0;
	int sum_T = 0, sum_brd = 0, sum_ti = 0, sum_syn = 0;	//分词后的多个查询词中有几个在重要、品牌、标题、同义字段
	int queSize = me.vTerms.size();
	if(queSize > 1 && 2 == pa->m_AnalysisPart.type[0])	//查询词有多个且都是品牌词
	{
		for(int i = 0; i < queSize; i++)
		{
			fid = me.vFieldsOff[i].field;
            if(fid == m_vFieldIndex[BRAND])	//查询词在品牌字段
            {
                ifInBrand = true;
                break;
            }
            else if(fid == m_vFieldIndex[TITLENAME])	//查询词在标题
            {
                ifInTitle = true;
            }
            else if(fid == m_vFieldIndex[TITLESYN])		//查询词在同义字段
            {
                ifInTitsyn = true;
            }
		}
		if(ifInBrand)
		{
			fieldScr = 4;
			MoveLeftBit(rt.nScore, 1, INBRDBIT);	//品牌字段匹配
		}
		else if(ifInTitle)
		{
			fieldScr = 3;
		}
		else if(ifInTitsyn)
		{
			fieldScr = 2;
		}
		MoveLeftBit(rt.nScore, 1, BASERELBIT);		//基本相关
		MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//文本匹配
	}
	else	//整个查询串是品牌词
	{
		for(int i = 0; i < queSize; i++)
		{
			fid = me.vFieldsOff[i].field;
			fieldNum = m_fid2fi[fid];
			queLen = me.vTerms[i].len;
			fieldLen = me.vFieldLen[i];
			if(fieldNum >= field_id[TMINNUM].id)
			{
				++sum_T;
				if(fid == m_vFieldIndex[BRAND])
				{
					++sum_brd;
					queBrd += (float)queLen / fieldLen;
				}
				else if(fid == m_vFieldIndex[TITLENAME])
				{
					++sum_ti;
				}
				else if(fid == m_vFieldIndex[TITLESYN])
				{
					++sum_syn;
				}
			}
		}
		if(sum_T == queSize)
		{
			if(sum_brd == queSize)
			{
				if(queBrd >= 0.6)//品牌字段是否有英文
				{
					fieldScr = 4;
					MoveLeftBit(rt.nScore, 1, INBRDBIT);//品牌字段匹配
				}
				else if(queBrd >= 0.3)
				{
					fieldScr = 3;
					MoveLeftBit(rt.nScore, 1, INBRDBIT);//品牌字段匹配
				}
				else
				{
					fieldScr = 2;
				}
			}
			else if(sum_ti == queSize)
			{
				fieldScr = 3;
			}
			else if(sum_syn == queSize)
			{
				fieldScr = 2;
			}
			MoveLeftBit(rt.nScore, 1, BASERELBIT);		//基本相关
			MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//文本匹配
		}
		if(pa->m_AnalysisPart.needJudgeDis)
		{
			for(int i = 1; i < queSize; i++)
			{
				//两个查询词在同一个字段、在字段出现的顺序与查询串中出现的顺序相同、在字段中出现的位置没有间隔，
				//不满足这几个条件中的任一个，则词距不匹配
				if(!((me.vFieldsOff[i-1].field == me.vFieldsOff[i].field) && 
				(0 == (me.vFieldsOff[i].off - me.vFieldsOff[i-1].off - me.vTerms[i-1].len))))
				{
					disScr = 0;
					break;
				}
			}
		}
	}
	if(0 == fieldScr)
	{
		return;
	}
	MoveLeftBit(rt.nScore, disScr, TERMDISBIT);		//词距匹配
	//MoveLeftBit(rt.nScore, 1, PDTCOREBIT);			//中心词匹配

	//记录文档类别信息
	/*size_t cateCnt = 0;
	int relSpeScr = 0;
	u64* cates = (u64*)m_funcValPtr(m_clsProfile, rt.nDocId, cateCnt);
	if(ifInBrand || queBrd >= 0.3)
	{
		relSpeScr = 2;
		++pa->m_AnalysisPart.relCnt_brd;
		int fbSize = pa->m_AnalysisPart.vBrdCate.size();
		if(0 != cateCnt)
		{
			for(int i = 0; i < fbSize; i++)
			{
				if(cates[0] == pa->m_AnalysisPart.vBrdCate[i].first)
				{
					//MoveLeftBit(rt.nScore, 1, FBCATEBIT);	//分类反馈
					u64 topCate = GetClassByLevel(1, cates[0]);
                    pa->m_AnalysisPart.topCate_brd.insert(topCate);
					break;
				}
			}
		}
	}
	else if(ifInTitle || sum_ti == queSize)
	{
		relSpeScr = 1;
		++pa->m_AnalysisPart.relCnt_ti;
		if(0 != cateCnt)
		{
			u64 topCate = GetClassByLevel(1, cates[0]);
			pa->m_AnalysisPart.topCate_ti.insert(topCate);
		}
	}
	rt.nWeight += relSpeScr * RelIndicator;
	*/

	//计算商业权重
	int salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
	ComputeCommerceWeightCloth(rt.nDocId, salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);
	
	MoveLeftBit(rt.nScore, 1, STOCKBIT);			//是否有库存
	MoveLeftBit(rt.nScore, ifHavImg, IMAGEBIT);		//是否有图片
	MoveLeftBit(rt.nScore, salPreScr, SALEPREBIT);	//销售预测
	MoveLeftBit(rt.nScore, salAmtScr + salNumScr + commentScr, COMMERCEBIT);//销售额、销量、评论数
	MoveLeftBit(rt.nScore, ifPub, ISPUBBIT);		//是否公用品
	ChangeWeightCloth(rt, pa);
	//ChangeWeightCloth(rt);
}

void CSearchKeyRanking::SingleRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	int fieldScr = 0;
	vector<SFieldOff> fidOffs = me.vFieldsOff;
#ifdef DEBUG
	int fidOffSize = fidOffs.size();
	cout <<"fidOffSize: " << fidOffSize << endl;
#endif
	int fid, fieldNum;

	//计算文本权重
	fid = fidOffs[0].field;
	if(fid < m_fid2fi.size())
	{
		fieldNum = m_fid2fi[fid];
		if(fieldNum >= field_id[TMINNUM].id)	//查询词在重要字段
		{
			if(fieldNum == field_id[TITLENAME].id)	//查询词在标题
			{
				fieldScr = 4;
			}
			else if(fieldNum == field_id[TITLESYN].id)	//查询词在同义字段
			{
				fieldScr = 3;
			}
			else if(fieldNum == field_id[BOTTOMCATE].id)	//查询词在底级类字段
			{
				fieldScr = 2;
			}
			else if(fieldNum == field_id[BRAND].id)	//查询词在品牌字段
			{
				fieldScr = 1;
				float queBrd = (float)me.vTerms[0].len / me.vFieldLen[0];
				if(queBrd >= 0.6)	//查询词是品牌词但不在品牌库文件中,品牌字段是否有中英文
				{
					fieldScr = 4;
				}
				else if(queBrd >= 0.3)
				{
					fieldScr = 3;
				}
				MoveLeftBit(rt.nScore, 1, INBRDBIT);	//品牌字段匹配
			}
			else if(fieldNum == field_id[SUBNAME].id)
			{
				fieldScr = 1;
			}
			MoveLeftBit(rt.nScore, 1, BASERELBIT);		//基本相关
			MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//文本匹配
			MoveLeftBit(rt.nScore, 3, TERMDISBIT);		//词距匹配,词距最高为3
		}
	}
	if(0 == fieldScr)
	{
		return;
	}

	//计算特殊信息权重(分类反馈、单品反馈、产品中心词匹配)
	int fbCateScr, fbPidScr, pdtCoreScr, relSpeScr;
	ComputeSpecialWeightCloth(rt.nDocId, pa, fbCateScr, fbPidScr, pdtCoreScr, relSpeScr);
	//rt.nWeight += relSpeScr * RelIndicator;

	//MoveLeftBit(rt.nScore, fbCateScr, FBCATEBIT);	//分类反馈
	//MoveLeftBit(rt.nScore, fbPidScr, FBPIDBIT);		//单品反馈
	MoveLeftBit(rt.nScore, pdtCoreScr, PDTCOREBIT);	//中心词匹配

	//计算商业因素权重(销售额、是否是公用品、销售量、销售预测、是否有图片、评论数)
	int salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
	ComputeCommerceWeightCloth(rt.nDocId, salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);	

	MoveLeftBit(rt.nScore, 1, STOCKBIT);			//是否有库存
	MoveLeftBit(rt.nScore, ifHavImg, IMAGEBIT);		//是否有图片
	MoveLeftBit(rt.nScore, salPreScr, SALEPREBIT);	//销售预测
	MoveLeftBit(rt.nScore, salAmtScr + salNumScr + commentScr, COMMERCEBIT);//销售额、销量、评论数
	MoveLeftBit(rt.nScore, ifPub, ISPUBBIT);		//是否公共品

	//根据商业权重情况进行调整
	ChangeWeightCloth(rt, pa);
	//ChangeWeightCloth(rt);
}

void CSearchKeyRanking::MultiRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//计算文本匹配权重
	bool ifInBrd = false;
	int fieldScr = 0, sum_T = 0, sum_ti = 0, sum_syn = 0, sum_btm = 0;	//分词后的多个查询词中有几个在重要、标题、同义、底级类字段
	int queSize = me.vTerms.size();
	vector<int> posInTi;
	posInTi.resize(queSize);
	int fid, fieldNum;
	for(int i = 0; i < queSize; i++)
	{
		fid = me.vFieldsOff[i].field;
		if(fid < m_fid2fi.size())
		{
			fieldNum = m_fid2fi[fid];
			if(fieldNum >= field_id[TMINNUM].id)	//查询词在重要字段
			{
				++sum_T;
				if(fieldNum == field_id[TITLENAME].id)	//查询词在标题字段
				{
					++sum_ti;
					posInTi[i] = me.vFieldsOff[i].off;
				}
				else if(fieldNum == field_id[TITLESYN].id)	//查询词在同义字段
				{
					++sum_syn;
				}
				else if(fieldNum == field_id[BOTTOMCATE].id)	//查询词在底级类字段
				{
					++sum_btm;
				}
				else if(fieldNum == field_id[BRAND].id && 2 == pa->m_AnalysisPart.type[i])//查询词在品牌字段且该词是品牌词
				{
					++sum_ti;
					ifInBrd = true;
					MoveLeftBit(rt.nScore, 1, INBRDBIT);		//品牌字段匹配
				}
			}
		}
	}
	bool ifAllIn_T = false, ifAllIn_ti = false;
	if(sum_T == queSize)
	{
		ifAllIn_T = true;
		if(sum_ti == queSize)
		{
			fieldScr = 4;
			ifAllIn_ti = true;
		}
		else if(sum_ti + sum_syn == queSize || sum_ti + sum_btm == queSize)
		{
			fieldScr = 3;
		}
		else if(sum_ti + sum_syn + sum_btm == queSize)
		{
			fieldScr = 2;
		}
		else if(sum_ti > 0 || sum_syn > 0 || sum_btm > 0)
		{
			fieldScr = 1;
		}
		MoveLeftBit(rt.nScore, 1, BASERELBIT);			//基本相关
		MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//文本匹配
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
		if(ifAllIn_ti)
		{
			int offSize;
			for(int i = 1; i < queSize; i++)
			{
				//两个查询词都是一般词且在查询串中没有间隔
				if(0 == pa->m_AnalysisPart.dis[i] && 0 == pa->m_AnalysisPart.type[i-1] && 0 == pa->m_AnalysisPart.type[i])
				{
					if(0 == isalnum(key[me.vTerms[i-1].pos]) && 0 == isalnum(key[me.vTerms[i].pos]))	//前后两个词项都为中文
					{
						offSize = posInTi[i] - posInTi[i-1];
						if(offSize < 0)	//两个查询词在字段中的顺序与在查询串中的顺序相反
						{
							if(me.vTerms[i-1].len <= 2 || me.vTerms[i].len <= 2)	//两个查询词至少有一个是单字
							{
								termScr = 0;
							}
							else
							{
								termScr = 1;
							}
						}
						else	//两个查询词在字段中的顺序与在查询串中的顺序相同
						{
							int spaLen = offSize - me.vTerms[i-1].len;	//在字段中两个查询词的间隔长度
							if(me.vTerms[i-1].len <= 2 && me.vTerms[i].len <= 2)	//两个查询词都是单字
							{
								termScr = (spaLen == 0) ? 3 : 0;
							}
							else if(0 == spaLen)
							{
								termScr = 3;
							}
							else if(spaLen <= 2)
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
				else	//两个查询词不都是一般词或在查询串中有间隔
				{
					sumDisScr += 3;
				}
			}
		}
		disScr = sumDisScr / (queSize - 1);	//确信 queSize - 1 != 0
	}
	MoveLeftBit(rt.nScore, disScr, TERMDISBIT);		//词距匹配

	//计算特殊信息权重(分类反馈、单品反馈、产品中心词匹配)
	int fbCateScr, fbPidScr, pdtCoreScr, relSpeScr;
    ComputeSpecialWeightCloth(rt.nDocId, pa, fbCateScr, fbPidScr, pdtCoreScr, relSpeScr);
    //rt.nWeight += relSpeScr * RelIndicator;
	
	//MoveLeftBit(rt.nScore, fbCateScr, FBCATEBIT);	//分类反馈
	//MoveLeftBit(rt.nScore, fbPidScr, FBPIDBIT);		//单品反馈
	MoveLeftBit(rt.nScore, pdtCoreScr, PDTCOREBIT);	//中心词匹配

	//计算商业因素权重(销售额、是否是公用品、销售量、销售预测、是否有图片、评论数)
	int salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
    ComputeCommerceWeightCloth(rt.nDocId, salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);

	MoveLeftBit(rt.nScore, 1, STOCKBIT);			//是否有库存
	MoveLeftBit(rt.nScore, ifHavImg, IMAGEBIT);		//是否有图片
	MoveLeftBit(rt.nScore, salPreScr, SALEPREBIT);	//销售预测
	MoveLeftBit(rt.nScore, salAmtScr + salNumScr + commentScr, COMMERCEBIT);//销售额、销量、评论数
	MoveLeftBit(rt.nScore, ifPub, ISPUBBIT);		//是否公用品

	//根据是否新品、词距匹配情况进行调整
	ChangeWeightCloth(rt, pa);
	//ChangeWeightCloth(rt);
}

void CSearchKeyRanking::ComputeSpecialWeightCloth(const int docId, CDDAnalysisData* pa, 
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
	//分类反馈
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

	//单品反馈
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

	//产品中心词匹配
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

void CSearchKeyRanking::ComputeCommerceWeightCloth(const int docId, int& salAmtScr, int& ifPub, 
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
		cout << "ComputeCommerceWeightCloth::ifPub: " << ifPub << endl;
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

void CSearchKeyRanking::ChangeWeightCloth(SResult& rt, CDDAnalysisData* pa)
//void CSearchKeyRanking::ChangeWeightCloth(SResult& rt)
{
	int dateMod = m_funcFrstInt(m_modifyTime, rt.nDocId);	//表示最新上架时间距离1970年1月1日的秒数
	int dateCur = time(NULL);
	int modDays = (dateCur - dateMod) / (3600 * 24);		//最新上架时间距离现在的天数
#ifdef DEBUG
	cout<<"ChangeWeightCloth::modDays: " << modDays << endl;
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
