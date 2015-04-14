//////////////////////////////////////////////////////////////////////
//�ļ�����: SocketPack.h:
//ժҪ:     SOCKETͨ�Ű�
//��ǰ�汾: 1.0
//����:     qinfei
//�������: 20090530 
///////////////////////////////////////////////////////

#ifndef _SOCKET_PACK_H_
#define _SOCKET_PACK_H_


#pragma warning(disable:4996)
#include "publicint.h"

#ifdef _LINUX_ENV_
	typedef  int					SOCKET;

	#define  INVALID_SOCKET			    -1
	#define  SOCKET_ERROR				-1

    #define WSAGetLastError()         errno

#endif

#define L_SOCK_PACK_IP_SIZE 16

class CSocketPack  
{
public:
	CSocketPack()
	{
	}

	virtual ~CSocketPack()
	{
	}	
	
	//////////////////////////////////////////////////////////////////////
	//������     �� Startup
	//���ܣ�	������ʼ��socket
	//��Σ�
	//���Σ�          
	//����ֵ: 
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T Startup()
	{
#ifndef _LINUX_ENV_
		WSAData cWSAData;
		if(WSAStartup(MAKEWORD(2, 2), &cWSAData))
			return -1;
#else
		sigset_t signal_mask;
		sigemptyset(&signal_mask);
		sigaddset(&signal_mask, SIGPIPE);
		if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL))
		{
			printf("block sigpipe error\n");
			return -1;
		}
#endif	
		return 0;	
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� Cleanup
	//���ܣ�	�����ͷ�socket
	//��Σ�
	//���Σ�          
	//����ֵ: 
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T Cleanup()
	{
#ifndef _LINUX_ENV_
		return WSACleanup();
#else
		return 0;
#endif
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� SetSendTimeOut
	//���ܣ�	��������SOCKET���ͳ�ʱ
	//��Σ�
	//              soClient:SOCKET���
	//              iTimeOut:��ʱֵ
	//���Σ�          
	//����ֵ:
	//�������:     20090530
	//              0:���óɹ� <0ʧ��
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T SetSendTimeOut (const SOCKET soClient, const INT32_T iTimeOut)
	{
		char *lpszTime=NULL;
		INT32_T iSize=0;
#ifdef _LINUX_ENV_
		struct timeval sOverTime;
		sOverTime.tv_sec = iTimeOut/1000;
		sOverTime.tv_usec = 0;
		if(sOverTime.tv_sec <= 0)
		{
			sOverTime.tv_sec = 0;
			sOverTime.tv_usec = iTimeOut * 1000;
		}
		lpszTime=(char*)&sOverTime;
		iSize=sizeof(sOverTime);		
#else
		lpszTime=(char *)&iTimeOut;
		iSize= sizeof(INT32_T);	
#endif
		if(setsockopt(soClient, SOL_SOCKET, SO_SNDTIMEO, lpszTime, iSize ) == SOCKET_ERROR)
			return -1;

		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� SetRecvTimeOut
	//���ܣ�	��������SOCKET���ճ�ʱ
	//��Σ�
	//              soClient:SOCKET���
	//              iTimeOut:��ʱֵ
	//���Σ�          
	//����ֵ:
	//�������:     20090530
	//              0:���óɹ� <0ʧ��
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T SetRecvTimeOut (const SOCKET soClient, const INT32_T iTimeOut)
	{
		char *lpszTime=NULL;
		INT32_T iSize=0;
#ifdef _LINUX_ENV_
		struct timeval sOverTime;
		sOverTime.tv_sec = iTimeOut/1000;
		sOverTime.tv_usec =0;
		if(sOverTime.tv_sec<=0)
		{
			sOverTime.tv_sec=0;
			sOverTime.tv_usec = iTimeOut*1000;
		}
		lpszTime=(char*)&sOverTime;
		iSize=sizeof(sOverTime);		
#else
		lpszTime=(char *)&iTimeOut;
		iSize= sizeof(INT32_T);	
#endif
		if(setsockopt(soClient, SOL_SOCKET, SO_RCVTIMEO,lpszTime, iSize)==SOCKET_ERROR)
			return -1;

		return 0;
	}
	
	static const INT32_T Accept(const SOCKET soClient,struct sockaddr* soAddr,INT32_T *iAddrLen,SOCKET &soAccept)
	{
#ifdef _LINUX_ENV_
		soAccept=accept(soClient,soAddr, (socklen_t*)iAddrLen);
#else
		soAccept=accept(soClient,soAddr, iAddrLen);
#endif	
		if(soAccept==INVALID_SOCKET )
		{
//#ifdef _LINUX_ENV_
//			printf("[%d]accept:%s,err:%d\n",soClient,strerror(errno),errno);
//#endif
			return -1;
		}

		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� BindServer
	//���ܣ�	������SOCKET����
	//��Σ�
	//              iPort:�����˿�
	//              iTimeOut:��ʱֵ
	//              soListenSocket:SOCKET���
	//���Σ�          
	//����ֵ: 
	//�������:     20090530
	//              0:�ɹ� <0ʧ��
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T  BindServer(const INT32_T iPort,const INT32_T iTimeOut,SOCKET &soListenSocket)
	{
		if (iPort<=0)
		{
			return -1;
		}		
		
		sockaddr_in soAddr;
		soAddr.sin_family = AF_INET;
		soAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
		soAddr.sin_port = htons(iPort); 
		soListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if (INVALID_SOCKET == soListenSocket)
		{
			return -2;
		}
		
	//	SetNonBlock(soListenSocket);
	//	INT32_T iReuse = 1;

	//	if (SOCKET_ERROR == setsockopt(soListenSocket, SOL_SOCKET, 
	//		SO_REUSEADDR, (char *)&iReuse, 	sizeof(INT32_T)))
	//	{
	//		CloseSocket(soListenSocket);
	//		return -6;
	//	}

		if (SOCKET_ERROR == bind(soListenSocket, (sockaddr *)&soAddr, sizeof(sockaddr_in)))
		{	
			printf("%ld\n",WSAGetLastError());
			CloseSocket(soListenSocket);	
			return -3;
		}		

	//	SetRecvTimeOut(soListenSocket,iTimeOut);
	//	SetSendTimeOut(soListenSocket,iTimeOut);
	
		if (SOCKET_ERROR == listen(soListenSocket, SOMAXCONN))
		{
			CloseSocket(soListenSocket);			
			return -7;
		}		

		return 0;
	}	

	//////////////////////////////////////////////////////////////////////
	//������     �� CloseSocket
	//���ܣ�	�����ر�socket
	//��Σ�
	//              soSocket:SOCKET���
	//���Σ�          
	//����ֵ: 
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static void CloseSocket(SOCKET &soSocket)
	{
		if (INVALID_SOCKET != soSocket)
		{
#ifdef _LINUX_ENV_
			//shutdown(soSocket,2);
			close(soSocket);
#else			
			closesocket(soSocket);
#endif
			soSocket = INVALID_SOCKET;
		}
	}	

	//////////////////////////////////////////////////////////////////////
	//������     �� Connect
	//���ܣ�	��������socket
	//��Σ�
	//              soSocket:SOCKET���
	//              iPort:�˿�
	//              iTimeOut:��ʱ
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T Connect(const char * const lpszIP,const INT32_T iPort,const INT32_T iTimeOut,SOCKET &soClient)
	{
		if(lpszIP==NULL || iPort<=0)
			return -1;
				
		soClient=socket(AF_INET,SOCK_STREAM,0);
		if(((INT32_T)soClient)<0)
			return -2;
		
		sockaddr_in clientaddr;
		memset(&clientaddr,0,sizeof(clientaddr));
		
		clientaddr.sin_family =AF_INET;
		clientaddr.sin_port =htons(iPort);
		clientaddr.sin_addr.s_addr = inet_addr(lpszIP); 
		
		INT32_T iRet=connect(soClient,(sockaddr*)&clientaddr,sizeof(sockaddr_in));
		
		if(iRet!=0)
		{		
			CloseSocket(soClient);
			return -5;
		}		

		SetRecvTimeOut(soClient,iTimeOut);
		SetSendTimeOut(soClient,5000);

		//INT32_T iOption=1;
		//setsockopt(soClient,SOL_SOCKET,SO_REUSEADDR,(char*)&iOption,sizeof(iOption));
		//
		//struct linger sLinger;
		//sLinger.l_onoff = 1;
		//sLinger.l_linger = 0;
		//if(setsockopt(soClient, SOL_SOCKET, SO_LINGER, (const char*)&sLinger, sizeof(sLinger)) == -1)
		//{
		//	CloseSocket(soClient);
		//	return -3;
		//}

		return 0;
	}
	
	static const char *GetIPStr(const SOCKET sock)
	{
		struct sockaddr cIP;
		socklen_t iIPLen = sizeof(struct sockaddr);

		if (getpeername(sock, &cIP, &iIPLen) != 0)
		{
			return NULL;
		}

		return inet_ntoa(((struct sockaddr_in*)&cIP)->sin_addr);
	}


	//////////////////////////////////////////////////////////////////////
	//������     �� SendData
	//���ܣ�	������������
	//��Σ�
	//              soSocket:SOCKET���
	//              lpszSendBuf:��������
	//              iSendBufLen:�������ݳ���
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T SendData(const SOCKET soSocket,const char * const lpszSendBuf,const INT32_T iSendBufLen)
	{
		if (lpszSendBuf==NULL || iSendBufLen <= 0)
		{
			return -1;
		}
		if (INVALID_SOCKET == soSocket)
		{
			return -2;
		}
		INT32_T iAllLen= send(soSocket,lpszSendBuf,iSendBufLen,0);
		
		if(iAllLen!=iSendBufLen)
		{
			printf("send to %s errno:%ld\n", GetIPStr(soSocket), WSAGetLastError());
			return -3;
		}
		return 0;
	}	
	
	//////////////////////////////////////////////////////////////////////
	//������     �� SendData
	//���ܣ�	������������ֱ�����
	//��Σ�
	//              soSocket:SOCKET���
	//              lpszSendBuf:��������
	//              iSendBufLen:�������ݳ���
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T SendDataEx(const SOCKET soSocket,const char * const lpszSendBuf,const INT32_T iSendBufLen)
	{
		if (lpszSendBuf==NULL || iSendBufLen <= 0)
		{
			return -1;
		}
		if (INVALID_SOCKET == soSocket)
		{
			return -2;
		}
		INT32_T iAllLen=0,iRet=0;
		
		do 
		{
			iRet = send(soSocket,lpszSendBuf+iAllLen,iSendBufLen-iAllLen,0);
			if(iRet > 0)
			{
				iAllLen += iRet;
			}
			
		} while(iRet > 0 && iAllLen < iSendBufLen);
		
		if(iAllLen!=iSendBufLen)
		{
			printf("send to %s errno: %ld\n",GetIPStr(soSocket), WSAGetLastError());
			return -3;
		}		
		return 0;
	}	
	
	//////////////////////////////////////////////////////////////////////
	//������     �� RecvData
	//���ܣ�	������������
	//��Σ�
	//              soSocket:SOCKET���
	//              lpszRecvBuf:��������
	//              iRecvBufLen:�������ݳ���
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T RecvData(const SOCKET soSocket,const char * lpszRecvBuf,const INT32_T iRecvBufLen)
	{
		if (lpszRecvBuf==NULL || iRecvBufLen <= 0)
		{
			return -1;
		}
		if (INVALID_SOCKET == soSocket)
		{
			return -2;
		}
		INT32_T iAllLen= recv(soSocket,(char*)lpszRecvBuf,iRecvBufLen,0);
		
		if(iAllLen!=iRecvBufLen)
		{
			printf("recv ret:%ld\n",WSAGetLastError());
			return -3;
		}		
		return 0;
	}	

	//////////////////////////////////////////////////////////////////////
	//������     �� RecvDataEx
	//���ܣ�	������������ֱ�����
	//��Σ�
	//              soSocket:SOCKET���
	//              lpszRecvBuf:��������
	//              iRecvBufLen:�������ݳ���
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T RecvDataEx(const SOCKET soSocket,const char * lpszRecvBuf,const INT32_T iRecvBufLen)
	{
		if (lpszRecvBuf==NULL || iRecvBufLen <= 0)
		{
			return -1;
		}
		if (INVALID_SOCKET == soSocket)
		{
			return -2;
		}
		
		INT32_T iAllLen=0,iRet=0;
		
		do 
		{
			iRet = recv(soSocket,(char*)(lpszRecvBuf+iAllLen),iRecvBufLen-iAllLen,0);
			if(iRet > 0)
			{
				iAllLen += iRet;
			}
			
		} while(iRet > 0 && iAllLen < iRecvBufLen);
		
		if(iAllLen!=iRecvBufLen)
		{
			printf("recv ret:%ld\n",WSAGetLastError());
			return -3;
		}		
		return 0;
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� SelectSendData
	//���ܣ�	������Select �ٷ�������
	//��Σ�
	//              soSocket:SOCKET���
	//              lpszSendBuf:��������
	//              iSendBufLen:�������ݳ���
	//              iTimeOut:��ʱ
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T SelectSendData(const SOCKET soSocket,const char * const lpszSendBuf, 
		const INT32_T iSendBufLen,const INT32_T iTimeOut)
	{		
		if(soSocket==INVALID_SOCKET ||lpszSendBuf==NULL || iSendBufLen<=0)
		{		
			return -1;
		}
		
		INT32_T iAllLen=0,iRet=0;
		
		iRet = PrepareWrite(soSocket, iTimeOut);
		if (iRet == 0)
		{			
			iAllLen = send(soSocket,lpszSendBuf,iSendBufLen,0);		
		}
		else
		{			
			return -2;
		}	
		
		if(iAllLen!=iSendBufLen)
		{
			return -3;
		}		
		return 0;		
	}
	
	//////////////////////////////////////////////////////////////////////
	//������     �� SelectSendData
	//���ܣ�	������Select �ٷ�������
	//��Σ�
	//              soSocket:SOCKET���
	//              lpszSendBuf:��������
	//              iSendBufLen:�������ݳ���
	//              iTimeOut:��ʱ
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T SelectSendDataEx(const SOCKET soSocket,const char * const lpszSendBuf,
		 const INT32_T iSendBufLen, const INT32_T iTimeOut)
	{
		if(soSocket==INVALID_SOCKET ||lpszSendBuf==NULL || lpszSendBuf[0]==0 || iSendBufLen<=0)
		{		
			return -1;
		}
		
		INT32_T iAllLen=0,iRet=0,iPreRet=0;
		
		do 
		{
			iPreRet = PrepareWrite(soSocket, iTimeOut);
			if (iPreRet == 0)
			{			
				iRet = send(soSocket,lpszSendBuf+iAllLen,iSendBufLen-iAllLen,0);
				if(iRet > 0)
				{
					iAllLen += iRet;
				}
			}
			else
			{			
				return -2;
			}
		} while(iRet > 0 && iAllLen < iSendBufLen);
		
		if(iAllLen!=iSendBufLen)
		{
			return -3;
		}		
		return 0;
	}
	
	//////////////////////////////////////////////////////////////////////
	//������     �� Setopt
	//���ܣ�	��������SOCKETѡ��
	//��Σ�
	//              iName:ѡ����
	//              vpValue:ֵ
	//              iLen:
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T Setopt(const SOCKET soSocket,const INT32_T iName,const void * const vpValue,const INT32_T iLen)
	{
		if (INVALID_SOCKET == soSocket)
		{
			return -1;
		}
		
		return setsockopt(soSocket, SOL_SOCKET, iName, (char *)vpValue, iLen);
	}

	//////////////////////////////////////////////////////////////////////
	//������     �� GetLocalMachineIP
	//���ܣ�	    ȡ����IP
	//��Σ�
	//				iIPListMax					���IP�б�������С
	//���Σ�
	//              szArrIPList:               ���ص�IP�б�
	//����ֵ:  
	//              ���ص�ǰIP����Ŀ��
	//�������:     20090530
	//��ע��    
	//////////////////////////////////////////////////////////////////////
	static const INT32_T GetLocalMachineIP(char szArrIPList[][L_SOCK_PACK_IP_SIZE],const INT32_T iIPListMax)
	{
		if(szArrIPList==NULL || iIPListMax<=0)
			return -1;
	
		const INT32_T L_BUFSIZ=512;
		char szHostName[L_BUFSIZ]; 
		INT32_T iMax=0;
#ifndef _LINUX_ENV_		
		
		INT32_T nStatus = gethostname(szHostName, sizeof(szHostName));
		if (nStatus == SOCKET_ERROR )
		{
			printf("gethostname failed, Error code: %ld\n", WSAGetLastError());
			return -2;
		}
	
		HOSTENT *host = gethostbyname(szHostName);
		if (host != NULL)
		{ 
			for ( INT32_T i=0; i<iIPListMax; i++ )
			{ 
				if(_snprintf(szArrIPList[iMax],L_SOCK_PACK_IP_SIZE-1,"%s",inet_ntoa( *(IN_ADDR*)host->h_addr_list[i]))<=0)
					continue;
				
				iMax++;
				if ( host->h_addr_list[i] + host->h_length >= host->h_name )
					break;
			} 
		}	

#else
		SOCKET s;
		struct ifconf conf;
		struct ifreq *ifr;
		INT32_T i,iNum;

		s = socket(PF_INET, SOCK_DGRAM, 0);
		if(s!=INVALID_SOCKET)
		{
			conf.ifc_len = L_BUFSIZ;
			conf.ifc_buf = szHostName;

			ioctl(s, SIOCGIFCONF, &conf);
			iNum = conf.ifc_len / sizeof(struct ifreq);
			ifr = conf.ifc_req;

			for(i=0;i<iNum && iMax<iIPListMax;i++)
			{
				struct sockaddr_in *sin = (struct sockaddr_in *)(&ifr->ifr_addr);

				ioctl(s, SIOCGIFFLAGS, ifr);
				if(((ifr->ifr_flags & IFF_LOOPBACK) == 0) && (ifr->ifr_flags & IFF_UP))
				{
					if(snprintf(szArrIPList[iMax],L_SOCK_PACK_IP_SIZE-1,"%s",inet_ntoa(sin->sin_addr))<=0)
						continue;
					iMax++;
				}
				ifr++;
			}
			CloseSocket(s);
		}

#endif
		return iMax;
	}


private:
	static const INT32_T  PrepareWrite(const SOCKET soSocket,const INT32_T iTimeOut)
	{
		INT32_T maxfd=0;
		fd_set writefds;	
		INT32_T rtn;
		struct timeval tm;
		if (iTimeOut <= 0)
		{
			return -2;
		}
		tm.tv_sec = iTimeOut/1000;
		tm.tv_usec = (iTimeOut%1000)*1000;
		
		FD_ZERO(&writefds);
		FD_SET(soSocket, &writefds);
		maxfd = maxfd > ((INT32_T)soSocket) ? maxfd : (INT32_T)soSocket;
		rtn = select(maxfd + 1, NULL, &writefds,NULL,&tm);
		if(rtn <= 0)
		{
			if (rtn == 0)
			{
				return -2;
			}
			return -1;
		}
		return 0;		
	}
	
	//////////////////////////////////////////////////////////////////////
	//������     �� SetNonBlock
	//���ܣ�	��������SOCKET������ʽ   ����Ϊ��������ʽ
	//��Σ�
	//              soSocket:SOCKET��� 
	//���Σ�          
	//����ֵ: 
	//              <0:����ʧ��   0:�ɹ�
	//�������:     20090530
	//��ע�� 
	//////////////////////////////////////////////////////////////////////
	static const INT32_T SetNonBlock(const SOCKET soSocket)
	{
#ifdef _LINUX_ENV_	
		INT32_T opt;
		opt = fcntl(soSocket,F_GETFL);
		opt |= O_NONBLOCK;
		fcntl(soSocket,F_SETFL,opt);
#else
		ULINT_T ioctl_opt;
		ioctlsocket(soSocket,FIONBIO,&ioctl_opt);
#endif
		return 0;
	}

	
};

#endif //_SOCKET_PACK_H_

