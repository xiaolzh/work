
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

#ifdef WINDOWS_PLATFORM //是WINDOWS平台
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

//基础Socket操作
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
    //FUNC  设置超时时间
    //PARAM sock：socket
    //      iRecvTimeout：接收超时，单位为毫秒
    //      iSendTimeout：发送超时，单位为毫秒
    //RET   0：设置成功
    //      其他：设置错误
    int SetTimeout(sock_descriptor sock, int iRecvTimeout, int iSendTimeout)
    {
        if (sock == sock_invalid)
        {
            return -1;
        }

#ifdef WINDOWS_PLATFORM
        //设置超时时间
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
    //FUNC  阻塞发送
    //PARAM sock：socket
    //      pSendBuf：待发送数据所在Buf
    //      TotalSendLen：待发送数据长度
    //RET   >=0：成功发送的字节数
    //      其他：参数错误
    //NOTE  返回成功发送的字节数可能小于TotalSendLen
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
    //FUNC  阻塞接收
    //PARAM sock：socket
    //      pRecvBuf：接收Buf
    //      TotalRecvLen：期望接收的数据总长
    //RET   >=0：成功接收的字节数
    //      其他：参数错误
    //NOTE  返回成功接收的字节数可能小于TotalRecvLen
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
    //FUNC  关闭socket
    //PARAM sock：socket
    void Close(sock_descriptor sock)
    {
        if (sock != sock_invalid)
        {
            shutdown(sock, 0x02);   //0x02 means SD_BOTH(windows) or SHUT_RDWR(linux)
            CloseSocket(sock);
        }        
    }
    //FUNC  得到本机的首选IP地址
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
    //FUNC  根据socket，得到对方IP地址
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
    //FUNC  取得连接端IP地址(完成端口模型)
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
    // 得到GetAcceptExSockaddrs的函数指针
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

//TCP服务端
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
    //FUNC  启动服务
    //PARAM u16Port：监听的端口
    //      MaxAcceptQueueLen：Accept队列长度
    //      pIP：监听的IP地址
    //RET   0：启动成功
    //      其他：启动失败，返回错误码
    int Startup(uint16_t u16Port, size_t MaxAcceptQueueLen = 64, char *pIP = NULL)
    {
        //创建监听Socket
        m_sockListen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_sockListen == sock_invalid)
        {
            return -10;
        }

        //形成服务器地址结构
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

        //初始化队列
        iRet = m_AcceptQueue.Init(sizeof(sock_descriptor)+sizeof(sockaddr_in), MaxAcceptQueueLen);
        if (iRet != 0)
        {
            return -40;
        }

        //启动监听线程
        m_bRun = true;
        iRet = m_AcceptThread.StartThread(AcceptThread, this);
        if (iRet != 0)
        {
            return -50;
        }

        return 0;
    }
    //FUNC  停止服务
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
    //FUNC  得到已连接的信息
    //PARAM pClientAddr：已连接的客户地址信息
    //RET   sock_invalid：获取失败
    //      其他：获取成功，为连接对应的socket
    sock_descriptor AcceptConn(struct sockaddr *pClientAddr = NULL)
    {
        char ConnInfo[sizeof(sock_descriptor)+sizeof(struct sockaddr)] = "";
        size_t InfoLen = 0;
        
        //从队列中取
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
        {//需要客户地址信息
            memcpy(pClientAddr, ConnInfo+sizeof(sock_descriptor), sizeof(struct sockaddr));
        }

        return sockConn;
    }
protected:
    //Accept线程：用于将连接信息加入队列
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

            //得到了一个连接信息
            char ConnInfo[sizeof(sock_descriptor)+sizeof(sClientAddr)] = "";
            *(sock_descriptor*)ConnInfo = sockConn;
            memcpy(ConnInfo+sizeof(sock_descriptor), &sClientAddr, sizeof(sClientAddr));

            //入队
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
    sock_descriptor m_sockListen;               //监听socket
    bool m_bRun;                                //线程运行标志
    nsWFThread::CThread m_AcceptThread;         //Accpet线程
    nsWFQueue::CBlockListQueue m_AcceptQueue;   //Accept队列
};

//TCP客户端
class CTcpClient : public CSock
{
public:
    //FUNC  连接服务器
    //PARAM ulINETAddr：IP地址的数字表示
    //      usPort：端口
    //      uimsTimeout：连接的超时时间。单位为毫秒。默认不会主动超时。
    //RET   sock_invalid：连接失败
    //      其他：连接成功，返回连接对应的socket
    sock_descriptor ConnServer(unsigned long ulINETAddr, unsigned short usPort, unsigned int uimsTimeout = 0)
    {
        //创建socket
        sock_descriptor sockConn = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (sockConn == sock_invalid)
        {
            return sock_invalid;
        }

        //初始化服务器地址
        struct sockaddr_in sServerAddr;
        memset(&sServerAddr, 0, sizeof(sockaddr_in));
        sServerAddr.sin_family = AF_INET;
        sServerAddr.sin_addr.s_addr = ulINETAddr;
        sServerAddr.sin_port = htons(usPort);

        //连接服务器
        if (uimsTimeout == 0)
        {//默认使用阻塞方式
            int iRet = connect(sockConn, (struct sockaddr *)&sServerAddr, sizeof(sServerAddr));
            if (iRet == sock_error)
            {//连不上
                Close(sockConn);
                return sock_invalid;
            }
            else
            {
                return sockConn;
            }
        }
        else
        {//使用非阻塞方式
            timeval sTimeOut;
            sTimeOut.tv_sec = uimsTimeout/1000;
            sTimeOut.tv_usec = (uimsTimeout%1000)*1000;
            fd_set fdWrite;
            unsigned long ul = 1;
            ioctlsocket(sockConn, FIONBIO, &ul); //设置为非阻塞模式
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
                ioctlsocket(sockConn, FIONBIO, &ul); //设置为阻塞模式
                return sockConn;
            }
            else
            {
                Close(sockConn);
                return sock_invalid;
            }            
        }
    }
    //FUNC  连接服务器
    //PARAM pIP：IP地址的“点分十进制”字符串
    //      usPort：端口
    //      uimsTimeout：连接的超时时间。单位为毫秒。默认不会主动超时。
    //RET   sock_invalid：连接失败
    //      其他：连接成功，返回连接对应的socket
    sock_descriptor ConnServer(const char *pIP, unsigned short usPort, unsigned int uimsTimeout = 0)
    {
        return ConnServer(inet_addr(pIP), usPort, uimsTimeout);
    }    
protected:
private:
};

}   // end of namespace

#endif  // end of #ifndef

