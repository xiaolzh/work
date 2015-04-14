#ifndef QUERYPARSE_H
#define QUERYPARSE_H
#ifdef NEW_SEG
#include "../segment_new/segmentor.h"
#else
#include "../segment/ta_dic_w.h"
#endif

//���Ԫ��Ϣ
struct CateStat
{
	CateStat() : pidCnt(0), saleSum(0), cateScore(0) {}
	int pidCnt;
	int saleSum;
	int cateScore;
	vector<int> camp;	//ÿ����Ӫ������ٽ��
};

class QueryItem
{
public:
	QueryItem():
		pos(0), fwdDis(0),
		length(0), ifSysExt(false), fwdSP(false){}

	string word;			//�ʴ�
	string field;			//�����ֵΪ�գ�����ȫ�����в�ѯ��������ָ�����в�ѯ
	unsigned int pos;			//�ִʱ��
	unsigned int fwdDis;	//��ǰ�ʴʾ�
	int length;				//�ʳ�
	bool ifSysExt;		//�Ƿ�ͬ����չ��
	bool fwdSP;				//�Ƿ����ո�
};

static bool IsHalf(const char *c)
{
	if (*((unsigned char*)c) < 0x80)
	{
		return true;
	}
	else
	{
		return false;
	}
}

static bool IsValidCharacter(const unsigned char *it, int characterLength)
{
	if (characterLength == 1)
	{
		// ����ַ� (ascii < 0x80)
		// ���ִ�ȫ: 0x30 ~ 0x39
		// ��д��ĸ: 0x41 ~ 0x5a
		// Сд��ĸ: 0x61 ~ 0x7a
		if (*it>=0x30 && *it<=0x39
			||
			*it>=0x41 && *it<=0x5a
			||
			*it>=0x61 && *it<=0x7a)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	if (characterLength == 2)
	{
		// �������
		if ((it[0]>=0x30 && it[0]<=0x39
			||
			it[0]>=0x41 && it[0]<=0x5a
			||
			it[0]>=0x61 && it[0]<=0x7a)
			&&
			(it[1]>=0x30 && it[1]<=0x39
			||
			it[1]>=0x41 && it[1]<=0x5a
			||
			it[1]>=0x61 && it[1]<=0x7a))
		{
			return true;
		}
		// ȫ���ַ�
		// 0xa3 (0xb0 ~ 0xb9) ��ȫ������:  ��~��Ӧ�ñ�������ʶ��
		// 0xa3 ȫ��Ӣ����ĸ: ������(0xc1 ~ 0xda) �ᣭ��(0xe1 ~ 0xfa)
		else if (*it == 0xa3)
		{
			if (*(it+1)>=0xb0 && *(it+1)<=0xb9
				||
				*(it+1)>=0xc1 && *(it+1)<=0xda
				||
				*(it+1)>=0xe1 && *(it+1)<=0xfa)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		//GBK definition table
		//0-6079 is the first chinese word table
		//6080-7789 is some chinese sign
		//7790-23845 is the left uncommon used chinese word
		if ((*it >= 129)&&(*(it+1) <= 254))
		{
			if (((*(it+1) >= 64)&&(*(it+1) < 127)) ||((*(it+1) > 127)&&(*(it+1) <= 254)))
			{
				int posit = (*it - 129) * 190 + (*(it+1) - 64) - (*(it+1)/128);
				if (posit>=0 && posit<=6079)
				{
					return true;
				}
				else if (posit>=7790 && posit<=23845)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}

static bool QueryParse(const string& query,
		               const vector<Term>& vTerms,
		               vector<QueryItem>& queryItems) {
	queryItems.clear();

	QueryItem item;
	string word_GBK;
	unsigned int pos = 0; //������Ч�ʱ��
	unsigned int punctLength = 0;	//�����㳤�ȣ����ڼ���ʼ�ࣩ

	for (size_t i = 0; i < vTerms.size(); ++i)
	{
		word_GBK = query.substr(vTerms[i].pos, vTerms[i].len);


		item.word = word_GBK;
		item.length = word_GBK.size();
		item.pos = pos++;
		if (item.pos != queryItems.size())
		{
			fprintf(stderr, "Work_New logic error!\n");
			return false;
		}
		if (queryItems.empty())
		{	//��һ����Ч��
			item.fwdDis = 0;
		}
		else
		{
			//item.fwdDis = punctLength;
			item.fwdDis = vTerms[i].pos-vTerms[i-1].pos - vTerms[i-1].len;
		}
		//punctLength = 0;	//���������
		/*if ( i == segRs.size()-1 && segRs.size() > 1
				&& false == fwdSP 
				&& 1 == word_GBK.size() && isdigit(word_GBK[0])
				&& IsAllChn(segRs[i-1]))
		{	//ĩβ��Ϊ��������ǰ��Ϊ����ʣ����ո�
			fwdSP = true;
		}*/
		queryItems.push_back(item);
	}

	return true;
}

#endif
