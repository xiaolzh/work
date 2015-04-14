#ifndef __PRICE_SPAN_H__
#define __PRICE_SPAN_H__
#include <string.h>

//价格区间数量  5个 存储空间为6 不要变
#define PRICE_SPAN_SIZE  6

//价格区间
struct PRICESPAN
{
	int nStartPrice;//起始价格
	int nEndPrice;//结束价格
	int nCount;//数量
};

template<class _Ty> inline
	void Swap(_Ty& _X, _Ty& _Y)
	{_Ty _Tmp = _X;
	_X = _Y, _Y = _Tmp; }

inline int CMP_SalePriceAscFroPriceSpan(
						 const int price1, 
						 const int price2
						 )
{
	if(price1==price2)
		return 0;
	return price1 < price2 ? -1 : 1;
};

class clsPriceSpan{
public:
	clsPriceSpan(){
		m_nPriceSpan=0;
	}
	void clear(){
		m_nPriceSpan=0;
	}
	// 有库存商品的 文档号
	bool buildSpan(int*pPrice/*价格的缓冲区*/,int nPriceCount/*价格的个数*/,int nTop=0/*提取前K个*/){
		/*
		m_nPriceSpan=0;
		if(nPriceCount==0) return false;

		if(nTop==0) nTop=nPriceCount;
		if(nTop>nPriceCount) nTop=nPriceCount;
		heapSortByPrice(pPrice,nPriceCount,nTop);
		m_nPriceSpan=0;
		
		buildSpan2(pPrice,nPriceCount);
		return true;*/

		// 2009-8-18 刘建国提供新代码
		m_nPriceSpan=0;
		if(nPriceCount<=0) return false;
		if(nTop==0) nTop=nPriceCount;
		if(nTop>nPriceCount) nTop=nPriceCount;

		heapSortByPrice(pPrice,nPriceCount,nTop);
		m_nPriceSpan=0;
		if(pPrice[nPriceCount-1]-pPrice[0]<=1000)
		   return false;
		buildSpan2(pPrice,nPriceCount);

		return true;

	}
	
private:
	inline void buildSpan2(int * pPrice,int nCount)
	{
		memset(m_PriceSpan,0,sizeof(PRICESPAN)*PRICE_SPAN_SIZE);
		
		int nStart,nEnd;
		int nStartPrice,nEndPrice;
		//我们认为两头只各占了5%,90%的商品都集中在中间部分,所以我们首先要确定这两个位置
		nStart=(int)(0.05*nCount);//计算初始位置
		nEnd=(int)(0.95*nCount);//计算初始结束位置
		
		int i;
		
		nStartPrice=pPrice[nStart];
		nEndPrice=pPrice[nEnd];
		
		//计算基本的价格区间
		int nBaseGap=(nEndPrice-nStartPrice)/15;
		
		m_PriceSpan[0].nStartPrice=0;
		m_PriceSpan[0].nEndPrice =pPrice[0];
		m_PriceSpan[5].nStartPrice=pPrice[nCount-1]; 
		int nGap=nBaseGap;
		for(i=1;i<5;i++)
		{
			m_PriceSpan[i].nStartPrice=m_PriceSpan[i-1].nEndPrice; 
			m_PriceSpan[i].nEndPrice=m_PriceSpan[i-1].nStartPrice+nGap; 
			nGap*=2;
		}
		
		//价格区间修正
		nGap=nBaseGap;
		if(nGap<1000)
			m_PriceSpan[0].nEndPrice=(m_PriceSpan[0].nEndPrice/100)*100;
		else if(nGap>=1000 && nGap<10000)  // 10-100
			m_PriceSpan[0].nEndPrice=(m_PriceSpan[0].nEndPrice/1000)*1000;
		else if(nGap>=10000 && nGap<100000) // 100-1000
			m_PriceSpan[0].nEndPrice=(m_PriceSpan[0].nEndPrice/10000)*10000;
		else if(nGap>=100000)  // >1000
			m_PriceSpan[0].nEndPrice=(m_PriceSpan[0].nEndPrice/50000)*50000;
		
		
		for(i=1;i<5;i++)
		{
			m_PriceSpan[i].nStartPrice=m_PriceSpan[i-1].nEndPrice;
			if(nGap<1000)
				m_PriceSpan[i].nEndPrice=(m_PriceSpan[i].nEndPrice/100)*100;
			else if(nGap>=1000 && nGap<10000)
				m_PriceSpan[i].nEndPrice=(m_PriceSpan[i].nEndPrice/1000)*1000;
			else if(nGap>=10000 && nGap<100000)
				m_PriceSpan[i].nEndPrice=(m_PriceSpan[i].nEndPrice/10000)*10000;
			else if(nGap>=100000)
				m_PriceSpan[i].nEndPrice=(m_PriceSpan[i].nEndPrice/100000)*100000;
			if(m_PriceSpan[i].nEndPrice<m_PriceSpan[i].nStartPrice)
				m_PriceSpan[i].nEndPrice=m_PriceSpan[i].nStartPrice;
			nGap*=2;
		}
		m_PriceSpan[5].nStartPrice=m_PriceSpan[i-1].nEndPrice;
		
		//统计各个区间内商品数目
		for(i=0;i<nCount;i++)
		{
			if(pPrice[i]<m_PriceSpan[0].nEndPrice)
				m_PriceSpan[0].nCount++; 
			else
				break;
		}
		for(;i<nCount;i++)
		{
			if(pPrice[i]<m_PriceSpan[1].nEndPrice)
				m_PriceSpan[1].nCount++; 
			else
				break;
		}
		for(;i<nCount;i++)
		{
			if(pPrice[i]<m_PriceSpan[2].nEndPrice)
				m_PriceSpan[2].nCount++; 
			else
				break;
		}
		for(;i<nCount;i++)
		{
			if(pPrice[i]<m_PriceSpan[3].nEndPrice)
				m_PriceSpan[3].nCount++; 
			else
				break;
		}
		for(;i<nCount;i++)
		{
			if(pPrice[i]<m_PriceSpan[4].nEndPrice)
				m_PriceSpan[4].nCount++; 
			else
				break;
		}
		m_PriceSpan[5].nCount=nCount-i; 
		
		m_nPriceSpan=6;
	}
	//按价格升序排序
	inline void heapSortByPrice( int* pPrice,
						int nSize,
						int nFirstCount
						)
	{
		int i;
		for(i=(nSize-2)/2;i>=0;i--)
			filterDown(pPrice,i,nSize-1);
		int nEnd;
		if(nFirstCount<nSize-1)
			nEnd=nSize-nFirstCount-1;
		else
			nEnd=1;
		for(i=nSize-1;i>=nEnd;i--)
		{
			Swap(pPrice[0],pPrice[i]);		
			filterDown(pPrice,0,i-1);
		}
	}
	inline void filterDown(int*pPrice,int i, int EndOfHeap)
	{
		int current = i; 
		int child = 2 * i + 1;
		int temp=pPrice[i];

		// 当CMP为空，默认以searchResult权重为比较对象
		// 否则比较方式按照CMP比较函数进行
		while (child <= EndOfHeap)
		{
			if (child < EndOfHeap 
				&& 
				CMP_SalePriceAscFroPriceSpan(pPrice[child], pPrice[child+1]) <= 0)
			{
				child++;
			}
			if ( CMP_SalePriceAscFroPriceSpan(temp, pPrice[child]) >= 0)
			{
				break;
			}
			else
			{
				pPrice[current]=pPrice[child];
				current = child;
				child = 2 * child + 1;
			}
		}
		pPrice[current]=temp;
	}
public:
	int m_nPriceSpan;
	PRICESPAN m_PriceSpan[PRICE_SPAN_SIZE];

	
private:
	int * m_pDocID;
	int * m_pPrice;
	int m_nDocCount;
};

#endif
