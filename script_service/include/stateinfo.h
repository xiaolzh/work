//////////////////////////////////////////////////////////////////////
//�ļ�����: StateInfo.h:
//ժҪ:     ����״̬���
//��ǰ�汾: 1.0
//����:     qinfei
//�������: 20090530 
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
	//������     �� Clear
	//���ܣ�	�����������״̬
	//��Σ�
	//���Σ�          
	//����ֵ: 
	//��ע�� 
	//����:      qinfei
	//�������:  20090530
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
	//������     �� LogOut
	//���ܣ�	����ע����ǰ״̬
	//��Σ�
	//���Σ�          
	//����ֵ: 
	//��ע�� 
	//����:      qinfei
	//�������:  20090530
	//////////////////////////////////////////////////////////////////////
	void LogOut()
	{
		m_Mutex.Lock();
		
		m_tTime =time(NULL)-m_tTime; 
	    m_bWork =false;	
				
		m_Mutex.UnLock();
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� CheckIn
	//���ܣ�	�����Ǽǵ�ǰ״̬
	//��Σ�
	//              m_u8State:�Ǽǵ�ǰ״̬
	//���Σ�          
	//����ֵ: 
	//��ע�� 
	//����:      qinfei
	//�������:  20090530
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
	//������     �� GetStateInfo
	//���ܣ�	����ȡ״̬��Ϣ
	//��Σ�
	//              iOverTime:��ʱֵ
	//              uOutBufSize:lszpOutBuf��SIZE
	//���Σ�          
	//              lszpOutBuf:���״̬BUFFER
	//              uOutBufLen:������ݵ�Length
	//����ֵ:
	//              0:״̬����    >0���г�ʱ����   
	//��ע�� 
	//              lszpOutBuf==NULL: ֻȡ�������Ƿ��г�ʱ�Ĵ���  ���� ȡ�������Ƿ��г�ʱ�Ĵ���,��װ������Ϣ��ӵ�
	//����:      qinfei
	//�������:  20101223
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
	
	time_t    m_tTime;             //uTime: ��ǰ״̬����ʱ��,m_bWork==true  uTime����ʼʱ��
	UINT16_T   m_siState;          //��ǰ״̬
	bool	   m_bWork;            //��ǰ״̬�Ƿ��� true:����       

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
	//������    Init
	//���ܣ�	��ʼ��
	//��Σ�	
	//         
	//���Σ�
	//����ֵ:
	//      0:��ʼ���ɹ�  <0��ʧ��
	//��ע��
	//����:      qinfei
	//�������:  20101222
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
	//������    Clear
	//���ܣ�	�������״̬
	//��Σ�	
	//         
	//���Σ�
	//����ֵ:
	//��ע��
	//����:      qinfei
	//�������:  20101223
	//////////////////////////////////////////////////////////////////////
	void Clear()
	{
		m_Mutex.Lock();
		m_iStateMax=0;
		m_Mutex.UnLock();
	}

	//////////////////////////////////////////////////////////////////////
	//������    Add
	//���ܣ�	�����߳�״̬
	//��Σ�	
	//         
	//���Σ�
	//����ֵ:
	//          ��ǰ���ӵ��̺߳�
	//��ע��
	//����:      qinfei
	//�������:  20101223
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
	//������     �� LogOut
	//���ܣ�	����ע����ǰ״̬
	//��Σ�
	//              iThreadNo:�̺߳�
	//���Σ�          
	//����ֵ: 
	//              0:�ɹ�  <0:ʧ��
	//��ע�� 
	//����:      qinfei
	//�������:  20101223
	//////////////////////////////////////////////////////////////////////
	const INT32_T LogOut(const INT32_T iThreadNo)
	{
		if(iThreadNo<0 || iThreadNo>=m_iStateMax)
			return -1;

		m_spState[iThreadNo].LogOut();
		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� CheckIn
	//���ܣ�	�����Ǽǵ�ǰ״̬
	//��Σ�
	//              iThreadNo:�̺߳�
	//              u8State:�Ǽǵ�ǰ״̬
	//���Σ�          
	//����ֵ: 
	//              0:�ɹ�  <0:ʧ��
	//��ע�� 
	//����:      qinfei
	//�������:  20101223
	//////////////////////////////////////////////////////////////////////
	const INT32_T CheckIn(const INT32_T iThreadNo,const UINT8_T u8State)
	{
		if(iThreadNo<0 || iThreadNo>=m_iStateMax)
			return -1;

		m_spState[iThreadNo].CheckIn(u8State);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� GetStateInfo
	//���ܣ�	����ȡ״̬��Ϣ
	//��Σ�
	//              iOverTime:��ʱֵ
	//              uOutBufSize:lszpOutBuf��SIZE
	//���Σ�          
	//              lszpOutBuf:���״̬BUFFER
	//              uOutBufLen:������ݵ�Length
	//����ֵ:
	//              0:״̬����    >0���г�ʱ����   
	//��ע�� 
	//              lszpOutBuf==NULL: ֻȡ�������Ƿ��г�ʱ�Ĵ���  ���� ȡ�������Ƿ��г�ʱ�Ĵ���,��װ������Ϣ��ӵ�
	//����:      qinfei
	//�������:  20101223
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

