//////////////////////////////////////////////////////////////////////
//文件名称: StateInfo.h:
//摘要:     工作状态结点
//当前版本: 1.0
//作者:     qinfei
//完成日期: 20090530 
///////////////////////////////////////////////////////

#ifndef _WORKSTATEINFO_H_
#define _WORKSTATEINFO_H_
#include "publicfunction.h"


struct WORKSTATEINFO
{
	WORKSTATEINFO()
	{
		m_tTime=0;		
		m_siState=0;
		m_bWork=false;
	}

	~WORKSTATEINFO()
	{
	}

	//////////////////////////////////////////////////////////////////////
	//函数：     　 Clear
	//功能：	　　清空所有状态
	//入参：
	//出参：          
	//返回值: 
	//备注： 
	//作者:      qinfei
	//完成日期:  20090530
	//////////////////////////////////////////////////////////////////////
	void Clear()
	{			
		m_Mutex.Lock();
		m_tTime=0;		
		m_siState=0;
		m_bWork=false;
		m_Mutex.UnLock();
	}

	//////////////////////////////////////////////////////////////////////
	//函数：     　 LogOut
	//功能：	　　注销当前状态
	//入参：
	//出参：          
	//返回值: 
	//备注： 
	//作者:      qinfei
	//完成日期:  20090530
	//////////////////////////////////////////////////////////////////////
	void LogOut()
	{
		m_Mutex.Lock();
		
		m_tTime =time(NULL)-m_tTime; 
	    m_bWork =false;	
				
		m_Mutex.UnLock();
	}

	//////////////////////////////////////////////////////////////////////
	//函数：     　 CheckIn
	//功能：	　　登记当前状态
	//入参：
	//              m_u8State:登记当前状态
	//出参：          
	//返回值: 
	//备注： 
	//作者:      qinfei
	//完成日期:  20090530
	//////////////////////////////////////////////////////////////////////
	void CheckIn(const UINT8_T u8State)
	{
		m_Mutex.Lock();
	
		m_tTime =time(NULL); 
		m_siState =u8State;
		m_bWork =true;			
						
		m_Mutex.UnLock();
	}
	
	//////////////////////////////////////////////////////////////////////
	//函数：     　 GetStateInfo
	//功能：	　　取状态信息
	//入参：
	//              iOverTime:超时值
	//              uOutBufSize:lszpOutBuf的SIZE
	//出参：          
	//              lszpOutBuf:输出状态BUFFER
	//              uOutBufLen:输出数据的Length
	//返回值:
	//              0:状态正常    >0：有超时数据   
	//备注： 
	//              lszpOutBuf==NULL: 只取函数内是否有超时的处理  否则 取函数内是否有超时的处理,并装错误信息添加到
	//作者:      qinfei
	//完成日期:  20101223
	//////////////////////////////////////////////////////////////////////
	const INT32_T GetStateInfo(const INT32_T iOverTime,char * const lszpOutBuf,const INT32_T iOutBufSize,INT32_T &iOutBufLen)
	{
		if(iOverTime<=0)
			return 0;

		iOutBufLen=0;				
		INT32_T iErr=0;	

		m_Mutex.Lock();
		INT32_T iTime = (INT32_T)(time(NULL) - m_tTime);
	
		if(m_bWork && iTime>iOverTime)
		{
			if(lszpOutBuf && iOutBufSize>0)
			{		
				iOutBufLen=_snprintf(lszpOutBuf,iOutBufSize-1,"%d_%d;",m_siState,iTime);				
			}
			iErr=1;	
		}

		m_Mutex.UnLock();		

		return iErr;
	}

private:
	CThreadMutex m_Mutex;
	
	time_t    m_tTime;             //uTime: 当前状态所用时间,m_bWork==true  uTime：起始时间
	UINT16_T   m_siState;          //当前状态
	bool	   m_bWork;            //当前状态是否工作 true:工作       

};

typedef struct _thread_info_
{
    WORKSTATEINFO     sState;
    void             *vpInfo;
    char             *lpszBuf;
    var_4             iBufSize;

    _thread_info_()
    {    
        vpInfo = NULL;
        lpszBuf = NULL;
        iBufSize = 0; 
    }    

}THREAD_INFO;

class CThreadState
{
public:
	CThreadState()
	{
		m_iThreadMax=0;
		m_spState=NULL;
		m_iStateMax=0;
	}

	~CThreadState()
	{
		if(m_spState)
		{
			delete[] m_spState;
			m_spState=NULL;
		}
		m_iThreadMax=0;
		m_iStateMax=0;
	}
	//////////////////////////////////////////////////////////////////////
	//函数：    Init
	//功能：	初始化
	//入参：	
	//         
	//出参：
	//返回值:
	//      0:初始化成功  <0：失败
	//备注：
	//作者:      qinfei
	//完成日期:  20101222
	//////////////////////////////////////////////////////////////////////
	const INT32_T Init(const INT32_T iThreadMax)
	{
		if(iThreadMax<=0)
			return -1;

		m_iThreadMax=iThreadMax;
		m_spState=new WORKSTATEINFO[m_iThreadMax];
		if(m_spState==NULL)
			return -2;

		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//函数：    Clear
	//功能：	清空所有状态
	//入参：	
	//         
	//出参：
	//返回值:
	//备注：
	//作者:      qinfei
	//完成日期:  20101223
	//////////////////////////////////////////////////////////////////////
	void Clear()
	{
		m_Mutex.Lock();
		m_iStateMax=0;
		m_Mutex.UnLock();
	}

	//////////////////////////////////////////////////////////////////////
	//函数：    Add
	//功能：	增加线程状态
	//入参：	
	//         
	//出参：
	//返回值:
	//          当前增加的线程号
	//备注：
	//作者:      qinfei
	//完成日期:  20101223
	//////////////////////////////////////////////////////////////////////
	const INT32_T Add()
	{
		m_Mutex.Lock();
		if(m_iStateMax>=m_iThreadMax)
		{
			m_Mutex.UnLock();
			return -1;
		}
		INT32_T iCur=m_iStateMax++;
		m_Mutex.UnLock();

		m_spState[iCur].Clear();

		return iCur;
	}
	
	//////////////////////////////////////////////////////////////////////
	//函数：     　 LogOut
	//功能：	　　注销当前状态
	//入参：
	//              iThreadNo:线程号
	//出参：          
	//返回值: 
	//              0:成功  <0:失败
	//备注： 
	//作者:      qinfei
	//完成日期:  20101223
	//////////////////////////////////////////////////////////////////////
	const INT32_T LogOut(const INT32_T iThreadNo)
	{
		if(iThreadNo<0 || iThreadNo>=m_iStateMax)
			return -1;

		m_spState[iThreadNo].LogOut();
		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//函数：     　 CheckIn
	//功能：	　　登记当前状态
	//入参：
	//              iThreadNo:线程号
	//              u8State:登记当前状态
	//出参：          
	//返回值: 
	//              0:成功  <0:失败
	//备注： 
	//作者:      qinfei
	//完成日期:  20101223
	//////////////////////////////////////////////////////////////////////
	const INT32_T CheckIn(const INT32_T iThreadNo,const UINT8_T u8State)
	{
		if(iThreadNo<0 || iThreadNo>=m_iStateMax)
			return -1;

		m_spState[iThreadNo].CheckIn(u8State);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//函数：     　 GetStateInfo
	//功能：	　　取状态信息
	//入参：
	//              iOverTime:超时值
	//              uOutBufSize:lszpOutBuf的SIZE
	//出参：          
	//              lszpOutBuf:输出状态BUFFER
	//              uOutBufLen:输出数据的Length
	//返回值:
	//              0:状态正常    >0：有超时数据   
	//备注： 
	//              lszpOutBuf==NULL: 只取函数内是否有超时的处理  否则 取函数内是否有超时的处理,并装错误信息添加到
	//作者:      qinfei
	//完成日期:  20101223
	//////////////////////////////////////////////////////////////////////
	const INT32_T GetStateInfo(const INT32_T iOverTime,char * const lszpOutBuf,const INT32_T iOutBufSize,INT32_T &iOutBufLen)
	{
		INT32_T iErr=0,iRet=0;

		iOutBufLen=0;
		char *lpszOut=(char*)lszpOutBuf,*lpszOutEnd=lpszOut+iOutBufSize;
		m_Mutex.Lock();

		for(INT32_T i=0;i<m_iStateMax;i++)
		{
			iRet=m_spState[i].GetStateInfo(iOverTime,lpszOut,lpszOutEnd-lpszOut,iOutBufLen);
			if(iRet>0)
			{
				iErr+=iRet;
				lpszOut+=iOutBufLen;
			}
		}

		iOutBufLen=lpszOut-lszpOutBuf;

		m_Mutex.UnLock();
		return iErr;
	}
private:
	WORKSTATEINFO    *m_spState;
	INT32_T           m_iThreadMax;

	INT32_T           m_iStateMax;

	CThreadMutex      m_Mutex;
};
#endif

