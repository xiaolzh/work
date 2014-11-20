//init config file
//init gather class
//do monitor work
#include "Gather.h"

var_4 main(var_4 argc, var_1 *argv[])
{
	CONFIG_INFO cfg;
	var_4 iRet;

	iRet = cfg.init("config.ini");

	if (iRet < 0)
	{
		printf("Main::init config file failed! Errno:%d\n", iRet);
		return -1;
	}

#ifdef DEBUG
	printf("recv: %d\ncomm: %d\n", cfg.recv_port, cfg.comm_port);
#endif
	

	Gather gath;
	if (argc >= 2 && memcmp(argv[1], "load", 4) == 0)
	{
		iRet = gath.Init(&cfg, 1);
	}
	else
	{
		iRet = gath.Init(&cfg);
	}

	if (iRet < 0)
	{
		printf("init Gather class failed!Errno:%d\n",iRet);
		return -1;
	}

	SOCKET   soClient,soMonitor;
	iRet = CSocketPack::BindServer(cfg.watch_port,2000,soMonitor);
	if (iRet != 0)
	{
		printf("bind watch port %d failed,ret=%ld\n", cfg.watch_port, WSAGetLastError());
		return -1;
	}

	struct sockaddr  sSockAddr;
	var_4 iAddrSize = sizeof(struct sockaddr);

	enum ErrorType {
        TYPE_OK = 0,        //work well
        TYPE_MONITOR,       //monitor error
        TYPE_NETWORK,       //network error
        TYPE_SERVICE,       //service error
        TYPE_OTHER,         //other error
    };  

    enum ErrorLevel {
        LEVEL_A = 1,        //crash
        LEVEL_B,            //important
        LEVEL_C,            //normal
        LEVEL_D,            //debug
        LEVEL_E             //nothing
    };  

    char szInfoBuf[1024];
    memcpy(szInfoBuf,"MonitorP1 ",10);

    while(1)
    {    
        if(CSocketPack::Accept (soMonitor, &sSockAddr, &iAddrSize,soClient)!=0)
        {
            cp_sleep(10);
            continue;
        }

        CSocketPack::SetRecvTimeOut(soClient, cfg.recv_overtime * 1000);
        CSocketPack::SetSendTimeOut(soClient, cfg.send_overtime * 1000);

        char *lpszStr = szInfoBuf + 10;

        iRet = CSocketPack::RecvData (soClient,lpszStr,4);

        if (iRet !=0 )
        {
            *(INT32_T*)lpszStr = 4;
            lpszStr += 4;

            *(INT32_T*)lpszStr = TYPE_NETWORK;
            lpszStr += 4;
        }
        else
        {
            lpszStr += 4;

            char *lpszType = lpszStr;
            lpszStr += 4;

            char *lpszLevel = lpszStr;
            lpszStr += 4;

            iRet = gath.GetThreadState(cfg.watch_overtime);

            if(iRet>0)
            {
                *(INT32_T*)lpszType=TYPE_SERVICE;
                *(INT32_T*)lpszLevel=LEVEL_A;
            }
            else
            {
                *(INT32_T*)lpszType=TYPE_OK;
                *(INT32_T*)lpszLevel=LEVEL_E;
            }

            *(INT32_T*)(szInfoBuf+10)=lpszStr-szInfoBuf-14;
        }

        iRet=CSocketPack::SendDataEx(soClient,szInfoBuf,lpszStr-szInfoBuf);
        CSocketPack::CloseSocket(soClient);

        printf("monitor send: ret:%d,len:%d\n",iRet,lpszStr-szInfoBuf);
    }
	return 0;
}

