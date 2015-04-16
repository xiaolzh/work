
#ifndef WF_SOCKET_H
#define WF_SOCKET_H

#include "WFInt.h"
#include "WFPub.h"
#include "WFThread.h"
#include "WFQueue.h"

#include <limits.h>

#ifdef WIN32
#define WINDOWS_PLATFORM
#endif

#ifdef WIN64
#define WINDOWS_PLATFORM
#endif

#ifdef WINDOWS_PLATFORM //��WINDOWSƽ̨
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib")
    #include "mswsock.h"
    typedef SOCKET sock_descriptor;
    typedef int socklen_t;
    #define sock_invalid INVALID_SOCKET
    #define sock_error SOCKET_ERROR
    #define CloseSocket(sock) closesocket(sock)
    typedef void (PASCAL FAR *GETACCEPTEXSOCKADDRS)(PVOID, DWORD, DWORD, DWORD, LPSOCKADDR*, LPINT, LPSOCKADDR*, LPINT);
#else
    #include <unistd.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <sys/ioctl.h>
    #include <signal.h>
    typedef int sock_descriptor;
    #define sock_invalid (-1)
    #define sock_error (-1)
    #define CloseSocket(sock) close(sock)
    #define ioctlsocket ioctl
#endif

namespace nsWFSocket
{

//����Socket����
class CSock
{
public:
    CSock()
    {
#ifdef WINDOWS_PLATFORM
        WSADATA WsaData;
        WSAStartup(MAKEWORD(2,2), &WsaData);
#else
        signal(SIGPIPE,SIG_IGN);    //ignore signal SIGPIPE
#endif
    }
    ~CSock()
    {
#ifdef WINDOWS_PLATFORM
        WSACleanup();
#endif
    }
    //FUNC  ���ó�ʱʱ��
    //PARAM sock��socket
    //      iRecvTimeout�����ճ�ʱ����λΪ����
    //      iSendTimeout�����ͳ�ʱ����λΪ����
    //RET   0�����óɹ�
    //      ���������ô���
    int SetTimeout(sock_descriptor sock, int iRecvTimeout, int iSendTimeout)
    {
        if (sock == sock_invalid)
        {
            return -1;
        }

#ifdef WINDOWS_PLATFORM
        //���ó�ʱʱ��
        int iRet = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&iRecvTimeout, sizeof(int));
        if (iRet == sock_error)
        {
            return -10;
        }

        iRet = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char *)&iSendTimeout, sizeof(int));
        if (iRet == sock_error)
        {
            return -20;
        }
#else
        struct timeval sOverTime;
		sOverTime.tv_sec = iRecvTimeout/1000;
		sOverTime.tv_usec = (iRecvTimeout%1000)*1000;
		int iRet = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&sOverTime, sizeof(sOverTime));
        if (iRet == sock_error)
        {
            return -10;
        }

		sOverTime.tv_sec = iSendTimeout/1000;
		sOverTime.tv_usec = (iSendTimeout%1000)*1000;
		iRet = setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&sOverTime, sizeof(sOverTime));
        if (iRet == sock_error)
        {
            return -20;
        }

#endif
		return 0;
	}
    //FUNC  ��������
    //PARAM sock��socket
    //      pSendBuf����������������Buf
    //      TotalSendLen�����������ݳ���
    //RET   >=0���ɹ����͵��ֽ���
    //      ��������������
    //NOTE  ���سɹ����͵��ֽ�������С��TotalSendLen
    int BlockingSend(sock_descriptor sock, const char *pSendBuf, size_t TotalSendLen)
    {
        if (sock == sock_invalid)
        {
            return -1;
        }

        size_t HadSendLen = 0;
        int iSendLen = 0;
        do 
        {
            iSendLen = send(sock, pSendBuf+HadSendLen, TotalSendLen-HadSendLen, 0);
            if (iSendLen > 0)
            {
                HadSendLen += iSendLen;
            }
            else
            {
                printf("iSendLen=%d, LastError=%d\n", iSendLen, nsWFPub::LastError());
            }
        } while(iSendLen > 0 && HadSendLen < HadSendLen);

        return HadSendLen;
    }
    //FUNC  ��������
    //PARAM sock��socket
    //      pRecvBuf������Buf
    //      TotalRecvLen���������յ������ܳ�
    //RET   >=0���ɹ����յ��ֽ���
    //      ��������������
    //NOTE  ���سɹ����յ��ֽ�������С��TotalRecvLen
    int BlockingRecv(sock_descriptor sock, char *pRecvBuf, size_t TotalRecvLen)
    {
        if (sock == sock_invalid)
        {
            return -1;
        }
        
        int iLoopCount = 0;

        size_t HadRecvLen = 0;
        int iRecvLen = 0;
        do 
        {
            iRecvLen = recv(sock, pRecvBuf+HadRecvLen, TotalRecvLen-HadRecvLen, 0);
            if (iRecvLen > 0)
            {
                HadRecvLen += iRecvLen;
            }
#ifndef WINDOWS_PLATFORM
            else if (iRecvLen < 0)
            {
                int iError = nsWFPub::LastError();
                if (iError == EINTR  || iError == EWOULDBLOCK || iError == EAGAIN)
                {
                    if (++iLoopCount == 100)
                    {
                        printf("BlockingRecv::iRecvLen=%d, LastError=%d\n", iRecvLen, iError);
                        iLoopCount = 0;
                    }
                    continue;
                }                
            }
#endif
            else
            {
                //printf("iRecvLen=%d, LastError=%d\n", iRecvLen, nsWFPub::LastError());
                break;
            }
        } while(HadRecvLen < TotalRecvLen);
        
        return HadRecvLen;
    }
    //FUNC  �ر�socket
    //PARAM sock��socket
    void Close(sock_descriptor sock)
    {
        if (sock != sock_invalid)
        {
            shutdown(sock, 0x02);   //0x02 means SD_BOTH(windows) or SHUT_RDWR(linux)
            CloseSocket(sock);
        }        
    }
    //FUNC  �õ���������ѡIP��ַ
    const char *GetIPStr()
    {
        char szHostName[1024] = {0};
        gethostname(szHostName, 255);

        struct hostent *pLocalHost = gethostbyname(szHostName);
        if (pLocalHost == NULL)
        {
            return NULL;
        }

        int i = 0;
        while (pLocalHost->h_addr_list[i] != NULL)
        {
            char *pIP = inet_ntoa(*((struct in_addr *)pLocalHost->h_addr_list[i]));
            if (memcmp(pIP, "127.", 4) != 0)
            {//isn't 127.*.*.*
                return pIP;
            }
            else
            {
                ++i;
            }
        }

        return NULL;
    }
    //FUNC  ����socket���õ��Է�IP��ַ
    const char *GetIPStr(sock_descriptor sock)
    {
        struct sockaddr cIP;
        socklen_t iIPLen = sizeof(struct sockaddr);

        if (getpeername(sock, &cIP, &iIPLen) != 0)
        {
            return NULL;
        }

        return inet_ntoa(((struct sockaddr_in*)&cIP)->sin_addr);
    }
#ifdef WINDOWS_PLATFORM 
    //FUNC  ȡ�����Ӷ�IP��ַ(��ɶ˿�ģ��)
    const char* GetIPStr(SOCKET in_sListen, char *buffer, GETACCEPTEXSOCKADDRS fpFunPoint = NULL)
    {
        int locallen, remotelen;
        SOCKADDR *pLocal = 0, *pRemote = 0;
		
        if (fpFunPoint == NULL)
        {
            GETACCEPTEXSOCKADDRS out_fpFunPoint = NULL;
            GetGetAcceptExSockaddrs(in_sListen, fpFunPoint);
        }

        fpFunPoint(buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &pLocal, &locallen, &pRemote, &remotelen);
		
        SOCKADDR_IN addr;
        memcpy(&addr, pRemote, sizeof(sockaddr_in));
		
        return inet_ntoa(addr.sin_addr);
    }
#endif
protected:
private:
#ifdef WINDOWS_PLATFORM
    // �õ�GetAcceptExSockaddrs�ĺ���ָ��
    long GetGetAcceptExSockaddrs(SOCKET in_sListen, GETACCEPTEXSOCKADDRS& out_fpFunPoint)
    {

        DWORD dwRetVal = 0;
        GUID GUIDName = WSAID_GETACCEPTEXSOCKADDRS;
        if (WSAIoctl(in_sListen, SIO_GET_EXTENSION_FUNCTION_POINTER, &GUIDName, sizeof(GUIDName), &out_fpFunPoint, sizeof(out_fpFunPoint), &dwRetVal, NULL, NULL))
        {
            return -1;
        }

        return 0;
    }
#endif
};

//TCP�����
class CTcpServer : public CSock
{
public:
    CTcpServer()
    {
        m_sockListen = sock_invalid;
        m_bRun = false;
    }
    ~CTcpServer()
    {
        Cleanup();
    }
    //FUNC  ��������
    //PARAM u16Port�������Ķ˿�
    //      MaxAcceptQueueLen��Accept���г���
    //      pIP��������IP��ַ
    //RET   0�������ɹ�
    //      ����������ʧ�ܣ����ش�����
    int Startup(uint16_t u16Port, size_t MaxAcceptQueueLen = 64, char *pIP = NULL)
    {
        //��������Socket
        m_sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_sockListen == sock_invalid)
        {
            return -10;
        }

        //�γɷ�������ַ�ṹ
        struct sockaddr_in sServerAddr;
        memset(&sServerAddr, 0, sizeof(sockaddr_in));
        sServerAddr.sin_family = AF_INET;
        if (pIP != NULL)
        {
            sServerAddr.sin_addr.s_addr = inet_addr(pIP);
        }
        else
        {
            sServerAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
        }            
        sServerAddr.sin_port = htons(u16Port);

        //bind
        int iRet = bind(m_sockListen, (struct sockaddr *)&sServerAddr,  sizeof(sServerAddr));
        if (iRet == sock_error)
        {
            return -20;
        }

        //listen
        iRet = listen(m_sockListen, INT_MAX);
        if (iRet == sock_error)
        {
            return -30;
        }

        //��ʼ������
        iRet = m_AcceptQueue.Init(sizeof(sock_descriptor)+sizeof(sockaddr_in), MaxAcceptQueueLen);
        if (iRet != 0)
        {
            return -40;
        }

        //���������߳�
        m_bRun = true;
        iRet = m_AcceptThread.StartThread(AcceptThread, this);
        if (iRet != 0)
        {
            return -50;
        }

        return 0;
    }
    //FUNC  ֹͣ����
    int Cleanup()
    {
        m_bRun = false;
        m_AcceptThread.WaitForQuit();

        if (m_sockListen != sock_invalid)
        {
            Close(m_sockListen);
            m_sockListen = sock_invalid;
        }
        return 0;
    }
    //FUNC  �õ������ӵ���Ϣ
    //PARAM pClientAddr�������ӵĿͻ���ַ��Ϣ
    //RET   sock_invalid����ȡʧ��
    //      ��������ȡ�ɹ���Ϊ���Ӷ�Ӧ��socket
    sock_descriptor AcceptConn(struct sockaddr *pClientAddr = NULL)
    {
        char ConnInfo[sizeof(sock_descriptor)+sizeof(struct sockaddr)] = "";
        size_t InfoLen = 0;
        
        //�Ӷ�����ȡ
        int iRet = m_AcceptQueue.DeQueue(ConnInfo, InfoLen);
        if (iRet != 0)
        {
            return sock_invalid;
        }
        
		if (InfoLen != sizeof(sock_descriptor)+sizeof(struct sockaddr))
		{
			return -2;
		}

        sock_descriptor sockConn = *(sock_descriptor*)ConnInfo;
        if (pClientAddr != NULL)
        {//��Ҫ�ͻ���ַ��Ϣ
            memcpy(pClientAddr, ConnInfo+sizeof(sock_descriptor), sizeof(struct sockaddr));
        }

        return sockConn;
    }
protected:
    //Accept�̣߳����ڽ�������Ϣ�������
    static void AcceptThread(void *pParam)
    {
        CTcpServer *pThis = (CTcpServer *)pParam;

        sock_descriptor sockConn;
        struct sockaddr sClientAddr;
        fd_set fdRead;
        while (pThis->m_bRun)
        {
            FD_ZERO(&fdRead);
            FD_SET(pThis->m_sockListen, &fdRead);
            struct timeval sTimeOut = {1, 0};
            //select on fdRead
            if (select(pThis->m_sockListen+1, &fdRead, NULL, NULL, &sTimeOut) <= 0) 
            {
                nsWFPub::SleepMilliSecond(1);
                continue;
            }

            if (!FD_ISSET(pThis->m_sockListen, &fdRead))
            {
                nsWFPub::SleepMilliSecond(1);
                continue;
            }

            socklen_t iAddrSize = sizeof(sClientAddr);
            sockConn = accept(pThis->m_sockListen, &sClientAddr, &iAddrSize);
            if (sockConn == sock_invalid)
            {
                nsWFPub::SleepMilliSecond(1);
                continue;
            }

            //�õ���һ��������Ϣ
            char ConnInfo[sizeof(sock_descriptor)+sizeof(sClientAddr)] = "";
            *(sock_descriptor*)ConnInfo = sockConn;
            memcpy(ConnInfo+sizeof(sock_descriptor), &sClientAddr, sizeof(sClientAddr));

            //���
            while (pThis->m_bRun)
            {
                int iRet = pThis->m_AcceptQueue.EnQueue(ConnInfo, sizeof(sock_descriptor)+sizeof(sClientAddr));
                if (iRet != 0)
                {
                    nsWFPub::SleepMilliSecond(1);
                }
                else
                {
                    break;
                }
            }
        }
    }
private:
    sock_descriptor m_sockListen;               //����socket
    bool m_bRun;                                //�߳����б�־
    nsWFThread::CThread m_AcceptThread;         //Accpet�߳�
    nsWFQueue::CBlockListQueue m_AcceptQueue;   //Accept����
};

//TCP�ͻ���
class CTcpClient : public CSock
{
public:
    //FUNC  ���ӷ�����
    //PARAM ulINETAddr��IP��ַ�����ֱ�ʾ
    //      usPort���˿�
    //      uimsTimeout�����ӵĳ�ʱʱ�䡣��λΪ���롣Ĭ�ϲ���������ʱ��
    //RET   sock_invalid������ʧ��
    //      ���������ӳɹ����������Ӷ�Ӧ��socket
    sock_descriptor ConnServer(unsigned long ulINETAddr, unsigned short usPort, unsigned int uimsTimeout = 0)
    {
        //����socket
        sock_descriptor sockConn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockConn == sock_invalid)
        {
            return sock_invalid;
        }

        //��ʼ����������ַ
        struct sockaddr_in sServerAddr;
        memset(&sServerAddr, 0, sizeof(sockaddr_in));
        sServerAddr.sin_family = AF_INET;
        sServerAddr.sin_addr.s_addr = ulINETAddr;
        sServerAddr.sin_port = htons(usPort);

        //���ӷ�����
        if (uimsTimeout == 0)
        {//Ĭ��ʹ��������ʽ
            int iRet = connect(sockConn, (struct sockaddr *)&sServerAddr, sizeof(sServerAddr));
            if (iRet == sock_error)
            {//������
                Close(sockConn);
                return sock_invalid;
            }
            else
            {
                return sockConn;
            }
        }
        else
        {//ʹ�÷�������ʽ
            timeval sTimeOut;
            sTimeOut.tv_sec = uimsTimeout/1000;
            sTimeOut.tv_usec = (uimsTimeout%1000)*1000;
            fd_set fdWrite;
            unsigned long ul = 1;
            ioctlsocket(sockConn, FIONBIO, &ul); //����Ϊ������ģʽ
            bool bSucc = false;
            if (connect(sockConn, (struct sockaddr *)&sServerAddr, sizeof(sServerAddr)) == sock_error)
            {
                FD_ZERO(&fdWrite);
                FD_SET(sockConn, &fdWrite);                    
                if (select(sockConn+1, NULL, &fdWrite, NULL, &sTimeOut) > 0)
                {
                    int error = -1;
                    socklen_t len = sizeof(int);
                    getsockopt(sockConn, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
                    if (error == 0)
                    {
                        bSucc = true;
                    }
                    else
                    {
                        bSucc = false;
                    }
                }
                else
                {
                    bSucc = false;
                }
            }
            else
            {
                bSucc = true;
            }

            if (bSucc)
            {
                ul = 0;
                ioctlsocket(sockConn, FIONBIO, &ul); //����Ϊ����ģʽ
                return sockConn;
            }
            else
            {
                Close(sockConn);
                return sock_invalid;
            }            
        }
    }
    //FUNC  ���ӷ�����
    //PARAM pIP��IP��ַ�ġ����ʮ���ơ��ַ���
    //      usPort���˿�
    //      uimsTimeout�����ӵĳ�ʱʱ�䡣��λΪ���롣Ĭ�ϲ���������ʱ��
    //RET   sock_invalid������ʧ��
    //      ���������ӳɹ����������Ӷ�Ӧ��socket
    sock_descriptor ConnServer(const char *pIP, unsigned short usPort, unsigned int uimsTimeout = 0)
    {
        return ConnServer(inet_addr(pIP), usPort, uimsTimeout);
    }    
protected:
private:
};

}   // end of namespace

#endif  // end of #ifndef

