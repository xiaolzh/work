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
		BrandQRankCloth(pa, me, rt);	//����Ʒ�ƴʲ�ѯȨ��
	}
	else if(1 == me.vTerms.size())
	{
#ifdef DEBUG
		cout<<"SingleRankCloth"<<endl;
#endif
		SingleRankCloth(pa, me, rt);	//���㵥���ʲ�ѯȨ��
	}
	else
	{
#ifdef DEBUG
		cout<<"MultiRankCloth"<<endl;
#endif
		MultiRankCloth(pa, me, rt);		//�������ʲ�ѯȨ��
	}
}

void CSearchKeyRanking::BrandQRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//�����ı�Ȩ��
	float queBrd = 0.0;
	int disScr = 3;
	bool ifInBrand = false, ifInTitle = false, ifInTitsyn = false;
	int fid, fieldNum, queLen, fieldLen, fieldScr = 0;
	int sum_T = 0, sum_brd = 0, sum_ti = 0, sum_syn = 0;	//�ִʺ�Ķ����ѯ�����м�������Ҫ��Ʒ�ơ����⡢ͬ���ֶ�
	int queSize = me.vTerms.size();
	if(queSize > 1 && 2 == pa->m_AnalysisPart.type[0])	//��ѯ���ж���Ҷ���Ʒ�ƴ�
	{
		for(int i = 0; i < queSize; i++)
		{
			fid = me.vFieldsOff[i].field;
            if(fid == m_vFieldIndex[BRAND])	//��ѯ����Ʒ���ֶ�
            {
                ifInBrand = true;
                break;
            }
            else if(fid == m_vFieldIndex[TITLENAME])	//��ѯ���ڱ���
            {
                ifInTitle = true;
            }
            else if(fid == m_vFieldIndex[TITLESYN])		//��ѯ����ͬ���ֶ�
            {
                ifInTitsyn = true;
            }
		}
		if(ifInBrand)
		{
			fieldScr = 4;
			MoveLeftBit(rt.nScore, 1, INBRDBIT);	//Ʒ���ֶ�ƥ��
		}
		else if(ifInTitle)
		{
			fieldScr = 3;
		}
		else if(ifInTitsyn)
		{
			fieldScr = 2;
		}
		MoveLeftBit(rt.nScore, 1, BASERELBIT);		//�������
		MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//�ı�ƥ��
	}
	else	//������ѯ����Ʒ�ƴ�
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
				if(queBrd >= 0.6)//Ʒ���ֶ��Ƿ���Ӣ��
				{
					fieldScr = 4;
					MoveLeftBit(rt.nScore, 1, INBRDBIT);//Ʒ���ֶ�ƥ��
				}
				else if(queBrd >= 0.3)
				{
					fieldScr = 3;
					MoveLeftBit(rt.nScore, 1, INBRDBIT);//Ʒ���ֶ�ƥ��
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
			MoveLeftBit(rt.nScore, 1, BASERELBIT);		//�������
			MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//�ı�ƥ��
		}
		if(pa->m_AnalysisPart.needJudgeDis)
		{
			for(int i = 1; i < queSize; i++)
			{
				//������ѯ����ͬһ���ֶΡ����ֶγ��ֵ�˳�����ѯ���г��ֵ�˳����ͬ�����ֶ��г��ֵ�λ��û�м����
				//�������⼸�������е���һ������ʾ಻ƥ��
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
	MoveLeftBit(rt.nScore, disScr, TERMDISBIT);		//�ʾ�ƥ��
	//MoveLeftBit(rt.nScore, 1, PDTCOREBIT);			//���Ĵ�ƥ��

	//��¼�ĵ������Ϣ
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
					//MoveLeftBit(rt.nScore, 1, FBCATEBIT);	//���෴��
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

	//������ҵȨ��
	int salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
	ComputeCommerceWeightCloth(rt.nDocId, salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);
	
	MoveLeftBit(rt.nScore, 1, STOCKBIT);			//�Ƿ��п��
	MoveLeftBit(rt.nScore, ifHavImg, IMAGEBIT);		//�Ƿ���ͼƬ
	MoveLeftBit(rt.nScore, salPreScr, SALEPREBIT);	//����Ԥ��
	MoveLeftBit(rt.nScore, salAmtScr + salNumScr + commentScr, COMMERCEBIT);//���۶������������
	MoveLeftBit(rt.nScore, ifPub, ISPUBBIT);		//�Ƿ���Ʒ
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

	//�����ı�Ȩ��
	fid = fidOffs[0].field;
	if(fid < m_fid2fi.size())
	{
		fieldNum = m_fid2fi[fid];
		if(fieldNum >= field_id[TMINNUM].id)	//��ѯ������Ҫ�ֶ�
		{
			if(fieldNum == field_id[TITLENAME].id)	//��ѯ���ڱ���
			{
				fieldScr = 4;
			}
			else if(fieldNum == field_id[TITLESYN].id)	//��ѯ����ͬ���ֶ�
			{
				fieldScr = 3;
			}
			else if(fieldNum == field_id[BOTTOMCATE].id)	//��ѯ���ڵ׼����ֶ�
			{
				fieldScr = 2;
			}
			else if(fieldNum == field_id[BRAND].id)	//��ѯ����Ʒ���ֶ�
			{
				fieldScr = 1;
				float queBrd = (float)me.vTerms[0].len / me.vFieldLen[0];
				if(queBrd >= 0.6)	//��ѯ����Ʒ�ƴʵ�����Ʒ�ƿ��ļ���,Ʒ���ֶ��Ƿ�����Ӣ��
				{
					fieldScr = 4;
				}
				else if(queBrd >= 0.3)
				{
					fieldScr = 3;
				}
				MoveLeftBit(rt.nScore, 1, INBRDBIT);	//Ʒ���ֶ�ƥ��
			}
			else if(fieldNum == field_id[SUBNAME].id)
			{
				fieldScr = 1;
			}
			MoveLeftBit(rt.nScore, 1, BASERELBIT);		//�������
			MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//�ı�ƥ��
			MoveLeftBit(rt.nScore, 3, TERMDISBIT);		//�ʾ�ƥ��,�ʾ����Ϊ3
		}
	}
	if(0 == fieldScr)
	{
		return;
	}

	//����������ϢȨ��(���෴������Ʒ��������Ʒ���Ĵ�ƥ��)
	int fbCateScr, fbPidScr, pdtCoreScr, relSpeScr;
	ComputeSpecialWeightCloth(rt.nDocId, pa, fbCateScr, fbPidScr, pdtCoreScr, relSpeScr);
	//rt.nWeight += relSpeScr * RelIndicator;

	//MoveLeftBit(rt.nScore, fbCateScr, FBCATEBIT);	//���෴��
	//MoveLeftBit(rt.nScore, fbPidScr, FBPIDBIT);		//��Ʒ����
	MoveLeftBit(rt.nScore, pdtCoreScr, PDTCOREBIT);	//���Ĵ�ƥ��

	//������ҵ����Ȩ��(���۶�Ƿ��ǹ���Ʒ��������������Ԥ�⡢�Ƿ���ͼƬ��������)
	int salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
	ComputeCommerceWeightCloth(rt.nDocId, salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);	

	MoveLeftBit(rt.nScore, 1, STOCKBIT);			//�Ƿ��п��
	MoveLeftBit(rt.nScore, ifHavImg, IMAGEBIT);		//�Ƿ���ͼƬ
	MoveLeftBit(rt.nScore, salPreScr, SALEPREBIT);	//����Ԥ��
	MoveLeftBit(rt.nScore, salAmtScr + salNumScr + commentScr, COMMERCEBIT);//���۶������������
	MoveLeftBit(rt.nScore, ifPub, ISPUBBIT);		//�Ƿ񹫹�Ʒ

	//������ҵȨ��������е���
	ChangeWeightCloth(rt, pa);
	//ChangeWeightCloth(rt);
}

void CSearchKeyRanking::MultiRankCloth(CDDAnalysisData* pa, SMatchElement& me, SResult& rt)
{
	//�����ı�ƥ��Ȩ��
	bool ifInBrd = false;
	int fieldScr = 0, sum_T = 0, sum_ti = 0, sum_syn = 0, sum_btm = 0;	//�ִʺ�Ķ����ѯ�����м�������Ҫ�����⡢ͬ�塢�׼����ֶ�
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
			if(fieldNum >= field_id[TMINNUM].id)	//��ѯ������Ҫ�ֶ�
			{
				++sum_T;
				if(fieldNum == field_id[TITLENAME].id)	//��ѯ���ڱ����ֶ�
				{
					++sum_ti;
					posInTi[i] = me.vFieldsOff[i].off;
				}
				else if(fieldNum == field_id[TITLESYN].id)	//��ѯ����ͬ���ֶ�
				{
					++sum_syn;
				}
				else if(fieldNum == field_id[BOTTOMCATE].id)	//��ѯ���ڵ׼����ֶ�
				{
					++sum_btm;
				}
				else if(fieldNum == field_id[BRAND].id && 2 == pa->m_AnalysisPart.type[i])//��ѯ����Ʒ���ֶ��Ҹô���Ʒ�ƴ�
				{
					++sum_ti;
					ifInBrd = true;
					MoveLeftBit(rt.nScore, 1, INBRDBIT);		//Ʒ���ֶ�ƥ��
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
		MoveLeftBit(rt.nScore, 1, BASERELBIT);			//�������
		MoveLeftBit(rt.nScore, fieldScr, TEXTRELBIT);	//�ı�ƥ��
	}
	if(0 == fieldScr)
	{
		return;
	}
	
	//�жϴʾ��Ƿ�ƥ��
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
				//������ѯ�ʶ���һ������ڲ�ѯ����û�м��
				if(0 == pa->m_AnalysisPart.dis[i] && 0 == pa->m_AnalysisPart.type[i-1] && 0 == pa->m_AnalysisPart.type[i])
				{
					if(0 == isalnum(key[me.vTerms[i-1].pos]) && 0 == isalnum(key[me.vTerms[i].pos]))	//ǰ���������Ϊ����
					{
						offSize = posInTi[i] - posInTi[i-1];
						if(offSize < 0)	//������ѯ�����ֶ��е�˳�����ڲ�ѯ���е�˳���෴
						{
							if(me.vTerms[i-1].len <= 2 || me.vTerms[i].len <= 2)	//������ѯ��������һ���ǵ���
							{
								termScr = 0;
							}
							else
							{
								termScr = 1;
							}
						}
						else	//������ѯ�����ֶ��е�˳�����ڲ�ѯ���е�˳����ͬ
						{
							int spaLen = offSize - me.vTerms[i-1].len;	//���ֶ���������ѯ�ʵļ������
							if(me.vTerms[i-1].len <= 2 && me.vTerms[i].len <= 2)	//������ѯ�ʶ��ǵ���
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
					else if(0 != isalnum(key[me.vTerms[i-1].pos]) && 0 != isalnum(key[me.vTerms[i].pos]))	//ǰ����������ΪӢ������
					{
						offSize = posInTi[i] - posInTi[i-1];
						if(offSize > 0 && (offSize - me.vTerms[i-1].len) <= 2)
						{
							sumDisScr += 3;
						}
					}
					else	//ǰ����������Ϊ��Ӣ�Ļ�����������
					{
						if(isalpha(key[me.vTerms[i-1].pos]) || isalpha(key[me.vTerms[i].pos]))	//ǰ��������������Ӣ��
						{
							offSize = posInTi[i] - posInTi[i-1];
							//������ѯ�����ֶ����ѯ���е�˳����ͬ��Ӣ�Ĵʵĳ���Ϊ1
							if(offSize > 0 && (1 == me.vTerms[i-1].len || 1 == me.vTerms[i].len))
							{
								if(0 == (offSize - me.vTerms[i-1].len))
								{
									sumDisScr += 3;
								}
							}
						}
						else	//ǰ��������������������
						{
							sumDisScr += 3;
						}
					}
				}
				else	//������ѯ�ʲ�����һ��ʻ��ڲ�ѯ�����м��
				{
					sumDisScr += 3;
				}
			}
		}
		disScr = sumDisScr / (queSize - 1);	//ȷ�� queSize - 1 != 0
	}
	MoveLeftBit(rt.nScore, disScr, TERMDISBIT);		//�ʾ�ƥ��

	//����������ϢȨ��(���෴������Ʒ��������Ʒ���Ĵ�ƥ��)
	int fbCateScr, fbPidScr, pdtCoreScr, relSpeScr;
    ComputeSpecialWeightCloth(rt.nDocId, pa, fbCateScr, fbPidScr, pdtCoreScr, relSpeScr);
    //rt.nWeight += relSpeScr * RelIndicator;
	
	//MoveLeftBit(rt.nScore, fbCateScr, FBCATEBIT);	//���෴��
	//MoveLeftBit(rt.nScore, fbPidScr, FBPIDBIT);		//��Ʒ����
	MoveLeftBit(rt.nScore, pdtCoreScr, PDTCOREBIT);	//���Ĵ�ƥ��

	//������ҵ����Ȩ��(���۶�Ƿ��ǹ���Ʒ��������������Ԥ�⡢�Ƿ���ͼƬ��������)
	int salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr;
    ComputeCommerceWeightCloth(rt.nDocId, salAmtScr, ifPub, salNumScr, salPreScr, ifHavImg, commentScr);

	MoveLeftBit(rt.nScore, 1, STOCKBIT);			//�Ƿ��п��
	MoveLeftBit(rt.nScore, ifHavImg, IMAGEBIT);		//�Ƿ���ͼƬ
	MoveLeftBit(rt.nScore, salPreScr, SALEPREBIT);	//����Ԥ��
	MoveLeftBit(rt.nScore, salAmtScr + salNumScr + commentScr, COMMERCEBIT);//���۶������������
	MoveLeftBit(rt.nScore, ifPub, ISPUBBIT);		//�Ƿ���Ʒ

	//�����Ƿ���Ʒ���ʾ�ƥ��������е���
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
	//���෴��
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

	//��Ʒ����
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

	//��Ʒ���Ĵ�ƥ��
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

	//������ҵ���ء�����ϸ�ڵ÷�
	int salDayAmt = m_funcFrstInt(m_saleDayAmtProfile, docId);
	int salWeekAmt = m_funcFrstInt(m_saleWeekAmtProfile, docId);
	int salMonAmt = m_funcFrstInt(m_saleMonthAmtProfile, docId);
	int salDayNum = m_funcFrstInt(m_saleDayProfile, docId);
	int salWeekNum = m_funcFrstInt(m_saleWeekProfile, docId);
	int salMonNum = m_funcFrstInt(m_saleMonthProfile, docId);
	int simiWeekAmt = (5 * salDayAmt + 7 * salWeekAmt + salMonAmt) / 12;	//���������۶�
	int simiWeekNum = (5 * salDayNum + 7 * salWeekNum + salMonNum) / 12;	//����������
	if(simiWeekAmt < 0 || simiWeekNum < 0)
	{
		return;
	}
	if(simiWeekAmt < 10000)
	{
		//��ҵ���ص÷����Ϊ12�֣��ù�ʽ���������Ϊ9
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

	//�Ƿ��ǹ���Ʒ
	ifPub = m_funcFrstInt(m_isShareProductProfile, docId);
#ifdef DEBUG
	if(ifPub != 0)
		cout << "ComputeCommerceWeightCloth::ifPub: " << ifPub << endl;
#endif

	if(simiWeekNum < 1000)
	{
		//�����÷����Ϊ10�֣��ù�ʽ���������Ϊ8
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

	//��������Ԥ��÷�
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

	//�Ƿ���ͼƬ
	ifHavImg = m_funcFrstInt(m_numImagesProfile, docId) > 0 ? 1 : 0;

	//�����������÷�
	int commentCnt = m_funcFrstInt(m_totalReviewCountProfile, docId);
	if(commentCnt <= 0)
	{
		commentScr = 0;
	}
	else if(commentCnt < 100)
	{
		//�������÷����Ϊ9��,�ù�ʽ���������Ϊ7
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
	int dateMod = m_funcFrstInt(m_modifyTime, rt.nDocId);	//��ʾ�����ϼ�ʱ�����1970��1��1�յ�����
	int dateCur = time(NULL);
	int modDays = (dateCur - dateMod) / (3600 * 24);		//�����ϼ�ʱ��������ڵ�����
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
