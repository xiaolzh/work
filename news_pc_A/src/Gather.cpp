#include "Gather.h"

Gather::Gather()
{
	m_cfg = NULL;
	e_log = NULL;
	m_pGroupBase = NULL;
	m_pDataBase = NULL;
	bm_groupStatus = NULL;
	bm_modify = NULL;

	m_pDocInfo = NULL;
	m_pFingerGroup = NULL;

	m_iThreadMax = 0;

	m_lpszThreadErr = NULL;
	m_spThreadInfo = NULL;

	m_data_store = NULL;
	m_bExit = false;
	memset(m_push_flag, 0, sizeof(m_push_flag));
}

Gather::~Gather()
{
	FreeObj(m_cfg);
	FreeObj(e_log);
	FreeObj(m_pGroupBase);
	FreeObj(m_pDataBase);
	FreeObj(bm_groupStatus);
	FreeObj(bm_modify);
	FreeObj(m_pDocInfo);
	FreeObj(m_pFingerGroup);
	FreeObj(m_lpszThreadErr);
	FreeObj(m_spThreadInfo);
	FreeObj(m_data_store);
	m_bExit = true;
}

//init function, restore data and init all work threads
var_4 Gather::Init(CONFIG_INFO* cfg, var_4 loadBinData)
{
	var_4 iRet = 0;

	if (cfg == NULL)
		return -1;
	m_cfg = cfg;

	if(CSocketPack::Startup() != 0)
    {
        e_log->writelog("%s:%d CSocketPack::Startup error!\n", FL, LN);
        return -2;
    }

#ifdef _LINUX_ENV_
    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGPIPE);
    if(pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) != 0)
    {
        e_log->writelog("%s:%d Block sigpipe error.\n", FL, LN);
        return -3;
    }
#endif

    CreateDir(m_cfg->log_path);
	// 初始化错误日志
	e_log = new Log_file();
	if (e_log == NULL)
	{
        printf("%s:%d New log error\n", FL, LN);
        return -4;
	}

    iRet = e_log->Init(m_cfg->log_path, (var_1 *)"ERROR", 10, 100<<20, 86400);
    if (iRet != 0)
    {
        printf("%s:%d InitLog error!\n", FL, LN);
        return -5;
    }
	
	// 初始化流程日志
	p_log = new Log_file();
	if (p_log == NULL)
	{
        printf("%s:%d New log error\n", FL, LN);
        return -4;
	}

    iRet = p_log->Init(m_cfg->log_path, (var_1 *)"PROCESS", 10, 100<<20, 86400);
    if (iRet != 0)
    {
        printf("%s:%d InitLog error!\n", FL, LN);
        return -5;
	}
	p_log->writelog("%s:%d Create send data directory first.\n", FL, LN);

	var_1 indexDataPath[MAX_FILE_PATH];
	// for example:
	// 		  Group NO. 	1
	// 		Machine NO.		2
	// corresponding the (1 * 256 + 2 )th thread.
	var_4 distribution[MAX_INDEX_GROUP * MAX_INDEX_IN_EACH_GROUP];
	var_4 total_machine = 0;
	for (var_4 i = 0; i < m_cfg->index_group_num; i++)
	{
		for (var_4 j = 0; j < m_cfg->index_num[i]; j++)
		{
			SPRINT(indexDataPath, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d%s", m_cfg->send_path, SLASH, i, SLASH, j, SLASH);
			
			//  /data/send/group1/machine1
			if (FileExists(indexDataPath) == false) // file not exist
			{
				e_log->writelog("%s:%d Path %s not exist, create it first!\n", FL, LN, indexDataPath);
				if (CreateDir(indexDataPath) != 0)
				{
					e_log->writelog("%s:%d Mkdir %s error! \n", FL, LN, indexDataPath);
					return -6;
				}
			}
			distribution[total_machine++] = 256 *i +j;
		}
	}
	// check if recv path existed
	if (FileExists(m_cfg->recv_path) == false) // inc data not existed
	{
		e_log->writelog("%s:%d Path %s not exist, create it first!\n", FL, LN, m_cfg->recv_path);
		if (CreateDir(m_cfg->recv_path) != 0)
		{
			e_log->writelog("%s:%d Create dir %s error.\n", FL, LN, m_cfg->recv_path);
			return -7;
		}
	}

	m_data_store = new UT_Persistent_KeyValue<var_u8>();
	if (m_data_store == NULL)
	{
        e_log->writelog("%s:%d New Data Storage error!\n", FL, LN);
		return -8;
	}

	iRet = m_data_store->init(m_cfg->data_store_config_file, loadBinData);
	if (iRet != 0)
	{
        e_log->writelog("%s:%d Init Data Storage error!Errno:%d\n", FL, LN, iRet);
		return -9;
	}

	var_1* pMemory = (var_1*)calloc(1,ALLOC_SIZE<<3);
	iRet = Recover(pMemory, loadBinData);
	FreeObj(pMemory);
	if (iRet < 0)
	{
		DeleteObj(m_pGroupAlloc);
		DeleteObj(m_pDataAlloc);
        DeleteObj(m_pDocInfo);
		DeleteObj(m_pFingerGroup);
		e_log->writelog("%s:%d Recover error!Errno:%d\n", FL, LN, iRet);
        return -10;
	}
    m_iThreadMax = m_cfg->thread_num_comm + m_cfg->thread_num_recv + m_cfg->thread_num_query + total_machine + 5;

    m_spThreadInfo = new THREAD_INFO[m_iThreadMax];
    if(m_spThreadInfo == NULL)
    {
        e_log->writelog("%s:%d New thread info error!\n", FL, LN);
        return -11;
    }

    m_lpszThreadErr = new var_1[m_iThreadMax * 20];
    if(m_lpszThreadErr == NULL)
    {
        e_log->writelog("%s:%d New thread err buffer error!\n", FL, LN);
        return -12;
    }

	THREAD_INFO *spThreadInfo = m_spThreadInfo;

	//recv data thread
	iRet = CSocketPack::BindServer(m_cfg->recv_port, 5000, m_addServer);
	if (iRet != 0)
	{
		e_log->writelog("%s:%d Listen port:%d error!Errno:%ld\n", FL, LN, m_cfg->recv_port, WSAGetLastError());
		return -13;
	}

	for(var_4 i = 0; i < m_cfg->thread_num_recv; i++)
	{
		spThreadInfo[i].vpInfo = this;
		CThreadManage::StartThread(ThreadRecvData, spThreadInfo + i);
	}
	spThreadInfo += m_cfg->thread_num_recv;

	// del data thread
	iRet = CSocketPack::BindServer(m_cfg->del_port, 5000, m_delServer);
	if (iRet != 0)
	{
		e_log->writelog("%s:%d Listen port:%d error!Errno:%ld\n", FL, LN, m_cfg->del_port, WSAGetLastError());
		return -14;
	}
	spThreadInfo->vpInfo = this;
	CThreadManage::StartThread(ThreadDelData, spThreadInfo);
	spThreadInfo += 1;

	// add data thread
	spThreadInfo->vpInfo = this;
	CThreadManage::StartThread(ThreadAddData, spThreadInfo);
	spThreadInfo += 1;

	//comm thread
	iRet = CSocketPack::BindServer(m_cfg->comm_port, 5000, m_commServer);
	if (iRet != 0)
	{
		e_log->writelog("%s:%d Listen port:%d error!Errno:%ld\n", FL, LN, m_cfg->comm_port, WSAGetLastError());
		return -15;
	}

	for(var_4 i = 0; i < m_cfg->thread_num_comm; i++)
	{
		spThreadInfo[i].vpInfo = this;
		CThreadManage::StartThread(ThreadComm, spThreadInfo + i);
	}
	spThreadInfo += m_cfg->thread_num_comm;

	// query thread
	iRet = CSocketPack::BindServer(m_cfg->query_port, 5000, m_queryServer);
	if (iRet != 0)
	{
		e_log->writelog("%s:%d Listen port:%d error!Errno:%ld\n", FL, LN, m_cfg->query_port, WSAGetLastError());
		return -16;
	}

	for (var_4 i = 0; i < m_cfg->thread_num_query; i++)
	{
		spThreadInfo[i].vpInfo = this;
		CThreadManage::StartThread(ThreadQuery, spThreadInfo + i);
	}
	spThreadInfo += m_cfg->thread_num_query;

	//generate push data thread
	spThreadInfo->vpInfo = this;
	CThreadManage::StartThread(ThreadGenPushData, spThreadInfo);
	spThreadInfo += 1;

	//push data thread
	for(var_4 i = 0; i < total_machine; i++)
	{
		spThreadInfo[i].vpInfo = this;
		spThreadInfo[i].iBufSize = distribution[i];
		// temp use for storage machine group and machine number  groupid * 256 + machineid
		CThreadManage::StartThread(ThreadPushData, spThreadInfo + i);
	}
	spThreadInfo += total_machine;

	//push all data thread
	for(var_4 i = 0; i < m_cfg->index_group_num; i++)
	{
		spThreadInfo[i].vpInfo = this;
		spThreadInfo[i].iBufSize = i;//temp use for storage machine group
		CThreadManage::StartThread(ThreadPushAllData, spThreadInfo + i);
	}
	spThreadInfo += m_cfg->index_group_num;

	//save data and backup data thread
	spThreadInfo->vpInfo = this;
	CThreadManage::StartThread(ThreadSave, spThreadInfo);
	spThreadInfo += 1;

	p_log->writelog("%s:%d Gather init finished!\n", FL, LN);
	return 0;
}

THREADFUNTYPE Gather::ThreadComm(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	assert(pThis);

	var_1 *pDocBuf = NULL;
	var_4 i, iRet, dataLen, cmdType, docsNum, searchType;
	var_4 processLen = 0, bufLen =0, iLen = 0, results; 
	// iLen: the length of pDocBuf used, results: the number of docs successfully processed
	var_u8 searchid, docid;
	SOCKET commClient;
	struct sockaddr sAddr;
	var_4 iAddrSize = sizeof(struct sockaddr);
	var_1 *szRecvBuf= (var_1*)calloc(1, MAX_RECV_PACK_LEN);
	var_1 *pMemory  = (var_1*)calloc(1, 1<<30);
	assert(szRecvBuf && pMemory);

	pDocBuf = (var_1*)pMemory;
	//add detail comm info here, you can use pThis->m_cfg to get configs
	while(!pThis->m_bExit)
	{
		if(CSocketPack::Accept (pThis->m_commServer, &sAddr, &iAddrSize, commClient) != 0)
			continue;
		CSocketPack::SetRecvTimeOut(commClient, 5000);
		CSocketPack::SetSendTimeOut(commClient, 5000);
		iRet = CSocketPack::RecvData (commClient, szRecvBuf, 12);
		if (iRet != 0)
		{
			pThis->SendErrorPack(commClient, -1);
			continue;
		}
		// check header here
		if (strncmp(szRecvBuf, CHECK_HEADER, 8) != 0)
		{
			pThis->e_log->writelog("%s:%d Check header error.(%s)\n", FL, LN, szRecvBuf);
			pThis->SendErrorPack(commClient, -2);
			continue;
		}
		dataLen = *(var_4*)(szRecvBuf + 8);
		if (dataLen < 0 || dataLen >= ALLOC_SIZE)
		{
			pThis->e_log->writelog("%s:%d Data length %d invalid.\n", FL, LN, dataLen);
			pThis->SendErrorPack(commClient, -3);
			continue;
		}
		iRet = CSocketPack::RecvDataEx (commClient, szRecvBuf + 12, dataLen);
		if (iRet != 0)
		{
			pThis->SendErrorPack(commClient, -4);
			continue;
		}
		cmdType = *(var_4*)(szRecvBuf + 12);
		processLen = 16;
		strncpy(pDocBuf, CHECK_HEADER, 8);
		bufLen = 1<<25;
		// Multiple (most 64) data query
		if (cmdType == query_command)
		{
			iLen = 16; 								// 8bytes: CHECK_HEADER, 4bytes: total length, 4bytes: return number of docs
			results = 0;
			docsNum = *(var_4*)(szRecvBuf + 16);
			processLen += 4;
			for (i = 0; i < docsNum && processLen - 20 <= dataLen; i++)
			{
				docid = *(var_u8*)(szRecvBuf + processLen);
				iRet = pThis->GetDocXml(docid, pDocBuf + iLen + 4, bufLen, pMemory, 1<<6);
				if (iRet > 0 && iRet + iLen < (ALLOC_SIZE<<7))
				{
					memcpy(pDocBuf + iLen, &iRet, 4);
					iLen += iRet + 4;
					results++;
				}
				processLen += 8;
			}
			*(var_4*)(pDocBuf + 8)  = iLen - 12;
			*(var_4*)(pDocBuf + 12) = results;
		}

		else if(cmdType == similar_docs)
 		{
   			var_4 startPos, endPos, totalNum, resultNum;
			iLen = 16;// 8bytes: CHECK_HEADER, 4bytes: total length, 4bytes: return number of docs
			// 获取相似新闻
			// searchType = 0 按docid查询
			// searchType = 1 按finger查询
			searchType  = *(var_4*)(szRecvBuf + 16);
			searchid    = *(var_u8*)(szRecvBuf + 20);
			startPos    = *(var_4*)(szRecvBuf + 28);
			endPos      = *(var_4*)(szRecvBuf + 32);
			iRet = pThis->GetSimilarDocs_test(searchid, searchType, startPos, endPos, pDocBuf + 16, bufLen, totalNum, resultNum);
			if (iRet <= 0)
			{
				pThis->e_log->writelog("%s:%d GetSimilarDocs_test(%lu) error, searchType is %d. Errno is %d.\n", FL, LN, searchid, searchType, iRet);
				*(var_4*)(pDocBuf + 8) = 4;
				*(var_4*)(pDocBuf + 12)= iRet;
			}
			else
			{
				*(var_4*)(pDocBuf + 8) = iRet + 4;
				*(var_4*)(pDocBuf + 12)= resultNum;
				iLen += iRet;
			}
		}
		else if (cmdType == capacity_status)
		{
			var_4 dataNum, groupNum;
			iLen = 20;
			pThis->GetStatus(dataNum, groupNum, 1);
			*(var_4*)(pDocBuf + 8) = 8;
			*(var_4*)(pDocBuf + 12)= dataNum;
			*(var_4*)(pDocBuf + 16)= groupNum;
		}
		// Binary data import
		else if (cmdType == binary_import)
		{
			iLen = 16;
			iRet = pThis->LoadBinData(pMemory, 1<<7);
			if (iRet < 0)
			{
				pThis->e_log->writelog("%s:%d[ERRNO:%d] LoadBinData error.\n", FL, LN, iRet);
				continue;
			}
			*(var_4*)(pDocBuf + 8)  = 4;
			*(var_4*)(pDocBuf + 12) = (iRet <0 ? -1:1);
		}
		// Binary data export
		else if (cmdType == binary_export)
		{
			iLen = 16;
			iRet = pThis->DumpBinData(pMemory, 1<<7);
			if (iRet < 0)
			{
				pThis->e_log->writelog("%s:%d[ERRNO:%d] DumpBinData error.\n", FL, LN, iRet);
				continue;
			}
			*(var_4*)(pDocBuf + 8)  = 4;
			*(var_4*)(pDocBuf + 12) = (iRet <0 ? -1:1);
		}
		// The online global data push
		else if (cmdType == global_push)
		{
			var_4 machine_id;
			iLen = 16;
			machine_id = *(var_4*)(szRecvBuf + 16);
			*(var_4*)(pDocBuf + 8) = 4;
			if (machine_id <= pThis->m_cfg->index_group_num)
			{
				pThis->m_push_flag[machine_id] = 1;
				*(var_4*)(pDocBuf + 12) = 1;
			}
			else
			{
				*(var_4*)(pDocBuf + 12) = -1;
			}
		}
		else
		{
			pThis->e_log->writelog("%s:%d Command(%d) is unrecognizable.\n", FL, LN, cmdType);
		}

		iRet = CSocketPack::SendData(commClient, pDocBuf, iLen);
		if (iRet != 0)
		{
			pThis->e_log->writelog("%s:%d[ERRNO:%d] Send data to client error.\n", FL, LN, iRet);
		}
		CSocketPack::CloseSocket(commClient);
	}
	FreeObj(szRecvBuf);
	FreeObj(pMemory);
	return NULL;
}

THREADFUNTYPE Gather::ThreadQuery(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	assert(pThis);

	SOCKET queryClient;
	struct sockaddr sAddr;
	var_4 iAddrSize = sizeof(struct sockaddr);
	var_1 *szRecvBuf = NULL;
	var_1 *szSendBuf = NULL;
	var_1 *pMemory = NULL;
	var_1 headBuf[32] = {0};
	var_1 timeBuf[11] = {0};
	var_4 iRet, optType, reqMethod, startPos, endPos;
	var_4 idLen, txtLen, leftLen, queryLen, queryTime, srcLen, dataLen, sendLen;
	var_4 resultCnt=0;
	var_u8 docid;

	szSendBuf = (var_1*)calloc(1, 2*ALLOC_SIZE);
	szRecvBuf = (var_1*)calloc(1, 1<<12);
	pMemory   = (var_1*)calloc(1, ALLOC_SIZE<<3);
	
	assert(szRecvBuf && szRecvBuf && pMemory);

	while (!pThis->m_bExit)
	{
		if (CSocketPack::Accept (pThis->m_queryServer, &sAddr, &iAddrSize, queryClient) != 0)
			continue;
		CSocketPack::SetRecvTimeOut(queryClient, 5000);
		CSocketPack::SetSendTimeOut(queryClient, 5000);
		iRet = CSocketPack::RecvData(queryClient, szRecvBuf, 72);
		if (iRet != 0)
		{
			pThis->SendErrorPack(queryClient, -1);
			continue;
		}
		if (strncmp(szRecvBuf + 10, "3333", 4) != 0)
		{
			pThis->e_log->writelog("%s:%d Check header error.\n", FL, LN);
			pThis->SendErrorPack(queryClient, -2);
			continue;
		}
		memcpy(headBuf, szRecvBuf, 10);
		dataLen = atol(headBuf);
		memcpy(headBuf, szRecvBuf + 14, 4);
		optType = atol(headBuf);
		switch(optType)
		{
		case 1301:
		case 1401:
		case 1501:
		case 1302:
		case 1402:
		case 1502:
		case 1303:
		case 1403:
		case 1503:
			reqMethod = optType;
			break;
		default:
			reqMethod = -1;
			break;
		}
		if (dataLen <= 72 || dataLen >= 2048 || reqMethod == -1)
		{
			pThis->e_log->writelog("%s:%d Option type(%d) invalid.\n", FL, LN, optType);
			pThis->SendErrorPack(queryClient, -3);
			continue;
		}
		iRet = CSocketPack::RecvDataEx(queryClient, szRecvBuf + 72, dataLen - 62);
		if (iRet != 0)
		{
			pThis->SendErrorPack(queryClient, -4);
			continue;
		}
		// now parse request body
		startPos = atol(szRecvBuf + 42);
		endPos   = atol(szRecvBuf + 52);
		idLen    = atol(szRecvBuf + 62);
		queryLen = atol(szRecvBuf + 72);
		// time 
		SPRINT(timeBuf, 11, "%s", szRecvBuf + 82);
		queryTime= atol(timeBuf);
		srcLen   = atol(szRecvBuf + 92);
		// check parameters
		if (idLen <0|| queryLen <0|| srcLen < 0|| startPos<0||startPos >= endPos ||endPos -startPos>=1024)
		{
			pThis->e_log->writelog("%s:%d Parameters check failed.\n", FL, LN);
			pThis->SendErrorPack(queryClient, -5);
			continue;
		}
		docid = strtoull(szRecvBuf + 102 + srcLen + queryLen, 0, 10);
		sendLen = 58;
		leftLen = 2*ALLOC_SIZE - 58;
		SPRINT(szSendBuf + 10, 9, "3333%d", reqMethod);
		
		switch(reqMethod)
		{
		case 1301:
		case 1302:
			iRet = pThis->GetSameNews(docid, reqMethod, startPos, endPos, szSendBuf + sendLen, leftLen, resultCnt, txtLen, pMemory, 0);
			if (iRet > 0)
			{
				sendLen += iRet;
				// feed back parameters
				SPRINT(szSendBuf + 18, 10, "%d", endPos - startPos);
				SPRINT(szSendBuf + 28, 10, "%d", resultCnt);
				SPRINT(szSendBuf + 38, 10, "%d", iRet);
				SPRINT(szSendBuf + 48, 10, "%d", txtLen);
			}
			else
			{
				SPRINT(szSendBuf + 18, 10, "%d", endPos - startPos);
				SPRINT(szSendBuf + 28, 10, "%d", 0);
				SPRINT(szSendBuf + 38, 10, "%d", 0);
				SPRINT(szSendBuf + 48, 10, "%d", 0);
			}
			break;
		case 1401:
		case 1402:
			iRet = pThis->GetRelativeNews(docid, reqMethod, szSendBuf + sendLen, leftLen, resultCnt, txtLen, pMemory, 0);
			if (iRet > 0)
			{
				sendLen += iRet;
				// feed back parameters
				SPRINT(szSendBuf + 18, 10, "%d", endPos - startPos);
				SPRINT(szSendBuf + 28, 10, "%d", resultCnt);
				SPRINT(szSendBuf + 38, 10, "%d", iRet);
				SPRINT(szSendBuf + 48, 10, "%d", txtLen);
			}
			else
			{
				SPRINT(szSendBuf + 18, 10, "%d", endPos - startPos);
				SPRINT(szSendBuf + 28, 10, "%d", 0);
				SPRINT(szSendBuf + 38, 10, "%d", 0);
				SPRINT(szSendBuf + 48, 10, "%d", 0);
			}
			break;
		case 1501:
		case 1502:
			break;
		default:
			pThis->e_log->writelog("%s:%d Invalid request method[%d].\n", FL, LN, reqMethod);
		}
		SPRINT(szSendBuf, 10, "%d", sendLen - 10); // 第10位为'\0'，最大到999999999
		// send data to client		
		iRet = CSocketPack::SendData(queryClient, szSendBuf, sendLen);
		if (iRet != 0)
		{
			pThis->e_log->writelog("%s:%d[ERRNO:%d] Send data to query client error.\n ", FL, LN, iRet);
		}
		CSocketPack::CloseSocket(queryClient);
	}
	FreeObj(pMemory);
	return NULL;
}

#ifdef OLD_DATA
THREADFUNTYPE Gather::ThreadRecvData(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	assert(pThis);

	var_4 iRet, dataLen, alignBufLen;
	SOCKET addClient;
	struct sockaddr sAddr;
	var_4 iAddrSize = sizeof(struct sockaddr);
	var_1 *szRecvBuf = (var_1*)calloc(1, MAX_RECV_PACK_LEN);
	var_1 recvFileName[MAX_FILE_PATH] = {0};
	var_1 singleDocBufHead[16] = {0};

	while (!pThis->m_bExit)
	{
		if (CSocketPack::Accept (pThis->m_addServer, &sAddr, &iAddrSize, addClient) != 0)
			continue;
		CSocketPack::SetRecvTimeOut(addClient, 5000);
		CSocketPack::SetSendTimeOut(addClient, 5000);

		iRet = CSocketPack::RecvData (addClient, szRecvBuf, 8);
		if (iRet != 0)
		{
			pThis->SendErrorPack(addClient, -1);
			continue;
		}
		// check header here, get dataLen from header
		if (*(var_u4*)szRecvBuf != 0xFDFDFDFD)
		{
			pThis->e_log->writelog("%s:%d Check header error.\n", FL, LN);
			pThis->SendErrorPack(addClient, -2);
			continue;
		}
		dataLen = *(var_4*)(szRecvBuf + 4);
		if (dataLen < 0 || dataLen + 8 >= ALLOC_SIZE)
		{
			pThis->e_log->writelog("%s:%d Data length %d invalid.\n", FL, LN, dataLen);
			pThis->SendErrorPack(addClient, -3);
			continue;
		}

		iRet = CSocketPack::RecvDataEx (addClient, szRecvBuf + 8, dataLen);
		if (iRet != 0)
		{
			pThis->SendErrorPack(addClient, -4);
			continue;
		}
		// send information to client
		iRet = CSocketPack::SendData(addClient, "RecvSuccess", 11);
		if (iRet != 0)
		{
			pThis->e_log->writelog("%s:%d[ERRNO:%d] Send data to client error.\n", FL, LN, iRet);
		}
		CSocketPack::CloseSocket(addClient);

		// lock then save data
		pThis->recv_data_lock.lock();
		SPRINT(recvFileName, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->recv_path, SLASH, "recv.dat");
		FILE *fp = fopen(recvFileName, "ab");
		if (fp == NULL)
		{
			pThis->recv_data_lock.unlock();
			pThis->e_log->writelog("%s:%d Open recv.dat error.\n", FL, LN);
			continue;
		}
		strncpy(singleDocBufHead, DATA_HEADER, 4);
		*(var_4*)(singleDocBufHead + 4) = add_command;
		*(var_4*)(singleDocBufHead + 8) = dataLen;
		iRet = fwrite(singleDocBufHead, 12, 1, fp);
		if (iRet != 1)
		{
			CloseFile(fp);
			pThis->recv_data_lock.unlock();
			continue;
		}
		alignBufLen = (dataLen + 3) & 0xFFFFFFFC;  // 四字节向上对齐
		iRet = fwrite(szRecvBuf + 8, alignBufLen, 1, fp);
		if (iRet != 1)
		{
			CloseFile(fp);
			pThis->recv_data_lock.unlock();
			continue;
		}

		CloseFile(fp);
		pThis->recv_data_lock.unlock();
	}
}

#else
THREADFUNTYPE Gather::ThreadRecvData(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	assert(pThis);

	var_4 iRet, dataLen, processLen, num, singleLen, alignBufLen;
	SOCKET addClient;
	struct sockaddr  sAddr;
	var_4 iAddrSize = sizeof(struct sockaddr);
	var_1 *szRecvBuf = (var_1*)calloc(1, MAX_RECV_PACK_LEN);
	var_1 recvFileName[MAX_FILE_PATH] = {0};
	var_1 szSendBuf[16] = {0};
	var_1 singleDocBufHead[16] = {0};

	while(!pThis->m_bExit)
	{
		if(CSocketPack::Accept (pThis->m_addServer, &sAddr, &iAddrSize, addClient) != 0)
			continue;
		CSocketPack::SetRecvTimeOut(addClient, 5000);
		CSocketPack::SetSendTimeOut(addClient, 5000);

		iRet = CSocketPack::RecvData (addClient, szRecvBuf, 12);
		if(iRet  != 0 )
		{
			pThis->SendErrorPack(addClient, -1);
			continue;
		}

		// check header here, get dataLen from header
		if (strncmp(szRecvBuf, CHECK_HEADER, 4))
		{
			pThis->e_log->writelog("%s:%d Check header error.\n", FL, LN);
			pThis->SendErrorPack(addClient, -2);
			continue;
		}
		dataLen = *(var_4*)(szRecvBuf + 8);
		if (dataLen < 0 || dataLen + 12 >= ALLOC_SIZE)
		{
			pThis->e_log->writelog("%s:%d Data length %d invalid.\n", FL, LN, dataLen);
			pThis->SendErrorPack(addClient, -3);
			continue;
		}

		iRet = CSocketPack::RecvDataEx (addClient, szRecvBuf + 12, dataLen);
		if(iRet  != 0 )
		{
			pThis->SendErrorPack(addClient, -4);
			continue;
		}
		// send information to client
		strncpy(szSendBuf, CHECK_HEADER, 8);
		*(var_4*)(szSendBuf + 8) = 0;
		iRet = CSocketPack::SendData(addClient, szSendBuf, 12);
		if (iRet != 0)
		{
			pThis->e_log->writelog("%s:%d[ERRNO:%d] Send data to client error.\n", FL, LN, iRet);
		}
		CSocketPack::CloseSocket(addClient);

		// lock then save data
		pThis->recv_data_lock.lock();
		if (SPRINT(recvFileName, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->recv_path, SLASH, "recv.dat")<=0)
		{
            continue;
		}
		FILE *fp = fopen(recvFileName, "ab");
		if (fp == NULL)
		{
			pThis->recv_data_lock.unlock();
			pThis->e_log->writelog("%s:%d Open recv.dat error.\n", FL, LN);
			continue;
		}
		num = *(var_4*)(szRecvBuf + 12);
		processLen = 16;
		for (var_4 i = 0; i < num; i++)
		{
			if (processLen - 12 >= dataLen)
			{
				break;
			}
			singleLen = *(var_4*)(szRecvBuf + processLen);
			strncpy(singleDocBufHead, DATA_HEADER, 4);
			*(var_4*)(singleDocBufHead + 4) = add_command;
			*(var_4*)(singleDocBufHead + 8) = singleLen;
			iRet = fwrite(singleDocBufHead, 12, 1, fp);
			if (iRet != 1)
			{
				continue;
			}
			alignBufLen = (singleLen + 3) & 0xFFFFFFFC; // 四字节向上对齐
			iRet = fwrite(szRecvBuf + processLen + 4, alignBufLen, 1, fp);
			if (iRet != 1)
			{
				continue;
			}
			processLen = processLen + singleLen + 4;
		}

		CloseFile(fp);
		pThis->recv_data_lock.unlock();
		// save end
	}
	FreeObj(szRecvBuf);
	return NULL;
}
#endif

//#ifdef OLD_DATA
//THREADFUNTYPE Gather::ThreadDelData(var_vd* vpInfo)
//{
//	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
//	Gather *pThis = (Gather*)spThread->vpInfo;
//	assert(pThis);
//	
//	var_4 i, docnum, iRet, dataLen;
//	var_u8 docid;
//	SOCKET delClient;
//	struct sockaddr sAddr;
//	var_4 iAddrSize = sizeof(struct sockaddr);
//	var_1 *szRecvBuf = (var_1*)calloc(1, MAX_RECV_PACK_LEN);
//    var_1 recvFileName[MAX_FILE_PATH] = {0};
//	var_1 singleDocBuf[16] = {0};
//	var_1 *curpos = NULL;
//	
//	while (!pThis->m_bExit)
//	{
//		if (CSocketPack::Accept (pThis->m_delServer, &sAddr, &iAddrSize, delClient)!=0)
//			continue;
//		CSocketPack::SetRecvTimeOut(delClient, 5000);
//		CSocketPack::SetSendTimeOut(delClient, 5000);
//
//		iRet = CSocketPack::RecvData(delClient, szRecvBuf, 10);
//		if (iRet != 0)
//		{
//			pThis->SendErrorPack(delClient, -1);
//			continue;
//		}
//		dataLen = atol(szRecvBuf);
//		if (dataLen <0 || dataLen + 8 >= MAX_RECV_PACK_LEN || dataLen % 30!= 0)
//		{
//			pThis->e_log->writelog("%s:%d Data length %d invalid.\n", FL, LN, dataLen);
//			pThis->SendErrorPack(delClient, -3);
//			continue;
//		}
//
//		iRet = CSocketPack::RecvDataEx(delClient, szRecvBuf + 10, dataLen);
//		if (iRet != 0)
//		{
//			pThis->SendErrorPack(delClient, -4);
//			continue;
//		}
//		// send information to client
//		iRet = CSocketPack::SendData(delClient, "DELETE OK!", 10);
//		if (iRet != 0)
//		{
//			pThis->e_log->writelog("%s:%d[ERRNO:%d] Send data to client error.\n", FL, LN, iRet);
//		}
//		CSocketPack::CloseSocket(delClient);
//
//		// process received buffer, write to file
//		docnum = dataLen / 30;
//		curpos = szRecvBuf + 10;
//		// lock then save data
//		pThis->recv_data_lock.lock();
//
//		SPRINT(recvFileName, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->recv_path, SLASH, "recv.dat");
//		FILE *fp = fopen(recvFileName, "ab");
//		if (fp == NULL)
//		{
//			pThis->recv_data_lock.unlock();
//			pThis->e_log->writelog("%s:%d Open recv.dat error.\n", FL, LN);
//			continue;
//		}
//
//		for (i = 0; i < docnum; ++i)
//		{
//			strncpy(singleDocBuf, DATA_HEADER, 4);
//			*(var_4*)(singleDocBuf + 4) = del_command;
//			*(var_4*)(singleDocBuf + 8) = 0;
//			docid = strtoul(curpos, 0, 10);
//			*(var_u8*)(singleDocBuf + 12) = docid;
//			iRet = fwrite(singleDocBuf, 20, 1, fp);
//			if (iRet != 1)
//			{
//				pThis->e_log->writelog("%s:%d[ERRNO:%d] fwrite delete doc[%lu] error.\n", FL, LN, iRet, docid);	
//			}
//		}
//		CloseFile(fp);
//		pThis->recv_data_lock.unlock();
//	}
//	FreeObj(szRecvBuf);
//	return NULL;
//}

//#else
THREADFUNTYPE Gather::ThreadDelData(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather*)spThread->vpInfo;
	assert(pThis);

	var_4 iRet = 0, dataLen = 0, delType = 0;
	var_u8 docid;
	SOCKET delClient;
	struct sockaddr sAddr;
	var_4 iAddrSize = sizeof(struct sockaddr);
	var_1 *szRecvBuf = (var_1*)calloc(1, MAX_RECV_PACK_LEN);
	var_1 szSendBuf[16] = {0};
    var_1 singleDocBuf[32]={0};	// only 16bytes used
    var_1 recvFileName[MAX_FILE_PATH] = {0};
	while (!pThis->m_bExit)
	{
		if (CSocketPack::Accept (pThis->m_delServer, &sAddr, &iAddrSize, delClient)!=0)
			continue;
		CSocketPack::SetRecvTimeOut(delClient, 5000);
		CSocketPack::SetSendTimeOut(delClient, 5000);

		iRet = CSocketPack::RecvData(delClient, szRecvBuf, 12);
		if (iRet != 0)
		{
			pThis->SendErrorPack(delClient, -1);
			continue;
		}
		// check head here
		if (strncmp(szRecvBuf, CHECK_HEADER, 8) != 0)
		{
			pThis->e_log->writelog("%s:%d Check header error.\n", FL, LN);
			pThis->SendErrorPack(delClient, -2);
			continue;
		}

		dataLen = *(var_4*)(szRecvBuf + 8);
		if (dataLen < 0 || dataLen + 12 >= ALLOC_SIZE)
		{
			pThis->e_log->writelog("%s:%d Data length %d invalid.\n", FL, LN, dataLen);
			pThis->SendErrorPack(delClient, -3);
			continue;
		}

		iRet = CSocketPack::RecvDataEx(delClient, szRecvBuf + 12, dataLen);
		if (iRet != 0)
		{
			pThis->SendErrorPack(delClient, -4);
			continue;
		}
		// send information to client
		strncpy(szSendBuf, CHECK_HEADER, 8);
		*(var_4*)(szSendBuf + 8) = 0;
		iRet = CSocketPack::SendData(delClient, szSendBuf, 12);
		if (iRet != 0)
		{
			pThis->e_log->writelog("%s:%d[ERRNO:%d] Send data to client error.\n", FL, LN, iRet);
		}
		CSocketPack::CloseSocket(delClient);

		// process received buffer, write to file
		var_4 dataNum = 0, processLen = 12;
		delType = *(var_4*)(szRecvBuf + 12);
		dataNum = *(var_4*)(szRecvBuf + 16);
		processLen += 8;
		// lock then save data
		pThis->recv_data_lock.lock();

		SPRINT(recvFileName, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->recv_path, SLASH, "recv.dat");
		
		FILE *fp = fopen(recvFileName, "ab");
		if (fp == NULL)
		{
			pThis->recv_data_lock.unlock();
			pThis->e_log->writelog("%s:%d Open recv.dat error.\n", FL, LN);
			continue;
		}

		for (var_4 i = 0; i < dataNum; i++)
		{
			strncpy(singleDocBuf, DATA_HEADER, 4);
			*(var_4*)(singleDocBuf + 4) = del_command;
			*(var_4*)(singleDocBuf + 8) = delType;
			docid = *(var_u8*)(szRecvBuf + processLen); 
			*(var_u8*)(singleDocBuf + 12) = docid;
			processLen = processLen + 8;
			iRet = fwrite(singleDocBuf, 20, 1, fp);
			if (iRet != 1)
			{
				pThis->e_log->writelog("%s:%d[ERRNO:%d] fwrite delete doc[%lu] error.\n", FL, LN, iRet, docid);
			}
		}
		CloseFile(fp);
		pThis->recv_data_lock.unlock();
		// save end
	}
	FreeObj(szRecvBuf);
	return NULL;
}
//#endif

THREADFUNTYPE Gather::ThreadAddData(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	assert(pThis);
	//include add new data, update data, refresh data
	//move recv.dat away and do the add work

	//1. check whether recv.dat exists
	//2. get lock
	//3. rename file
	//4. release lock
	//5. do add work, here will use update_data_lock

	// alloc buffers
	DOC_BUF *pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
	var_1 *pMemory = (var_1*)calloc(1, ALLOC_SIZE<<3);
	var_1 *pAddBuf = NULL;
	var_1 *pIncBuf = NULL;
    assert(pDoc != NULL || pMemory != NULL);

	var_1  recvFileName[MAX_FILE_PATH] = {0};
	var_1  addFileName[MAX_FILE_PATH]  = {0};
	var_1  incFileName[MAX_FILE_PATH]  = {0};
    var_1  recvDataHead[16] = {0};
	var_4  iRet = 0, cmdType = 0, delType = 0, bufLen = 0;
	var_u8 deleteid, docid, groupid, fileSize = 0;
	// recv.dat
	SPRINT(recvFileName, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->recv_path, SLASH, "recv.dat");
	SPRINT(addFileName,  MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->recv_path, SLASH, "add.dat");
	while(!pThis->m_bExit)
	{
   		pAddBuf = (var_1*)(pMemory);
    	pIncBuf = (var_1*)(pMemory + ALLOC_SIZE);
		// add.dat not exist
		if (FileExists(addFileName) == false)
		{
			// recv.dat exist
			if (FileExists(recvFileName))
			{
				pThis->recv_data_lock.lock();
				RenameFile(recvFileName, addFileName);
				pThis->recv_data_lock.unlock();
			}
			// recv.dat not exist
			else
			{
				sleep(2);
				continue;
			}
		}
		FILE *fp = fopen(addFileName, "rb");
		if (fp == NULL)
		{
			pThis->e_log->writelog("%s:%d Open %s error.\n", FL, LN, addFileName);
			sleep(2);
			continue;
		}
		// check header
		var_4 readBytes = fread(recvDataHead, 4, 1, fp);
		if (readBytes < 0 || memcmp(recvDataHead, DATA_HEADER, 4) != 0)
		{
			pThis->e_log->writelog("%s:%d Recv data header error!(%s)\n", FL, LN, recvDataHead);
			pThis->GotoNextRecord(fp);
		}
        // save receive log
        SPRINT(incFileName, MAX_FILE_PATH, "%s%srecv.inc", pThis->m_cfg->recv_path, SLASH);
        FILE *fpInc = NULL;
        fpInc = fopen(incFileName, "ab");
        if (fpInc == NULL)
        {
            pThis->e_log->writelog("%s:%d Open recv.inc error.\n", FL, LN);
            sleep(2);
            continue;
        }
        fileSize = GetFileSize(fp);
		while (fileSize != ftell(fp))
		{
		    readBytes = fread(&cmdType, 4, 1, fp);
		    if (readBytes <= 0)
		    {
		        pThis->e_log->writelog("%s:%d Invalid command type %d.\n", FL, LN, cmdType);
                pThis->GotoNextRecord(fp);
                continue;
		    }

			if (cmdType == add_command)
			{
                readBytes = fread(&bufLen, sizeof(var_4), 1, fp);
                if (readBytes <= 0 || bufLen < 0 || (bufLen + 12) > ALLOC_SIZE)
                {
                    pThis->e_log->writelog("%s:%d Invalid buffer length %d.\n", FL, LN, bufLen);
                    pThis->GotoNextRecord(fp);
                    continue;
                }

                readBytes = fread(pAddBuf, (bufLen + 3) & 0xFFFFFFFC, 1, fp);
                if (readBytes < 0)
                {
                    pThis->e_log->writelog("%s:%d Read body error.\n", FL, LN);
                    pThis->GotoNextRecord(fp);
                    continue;
                }
				// process buffer
				iRet = ProcessBufToDoc(pAddBuf, bufLen, pDoc);
				if (iRet != 0)
				{
					pThis->e_log->writelog("%s:%d[ERRNO:%d] Process body error!\n", FL, LN, iRet);
					pThis->GotoNextRecord(fp);
					continue;
				}

				if (pThis->m_cfg->noduplicate == 1)
				{
					pThis->AddDoc_noduplicate(pDoc, pDoc->force, pMemory, 2);
				}
				else
				{
					pThis->AddDoc(pDoc, pDoc->force, pMemory, 2);
				}
                // write to receive log
                memcpy(pIncBuf, DATA_HEADER,  4);
                memcpy(pIncBuf + 4, &cmdType, 4);
                memcpy(pIncBuf + 8, &bufLen, 4);
                memcpy(pIncBuf + 12, pAddBuf, bufLen);
                fwrite(pIncBuf, (bufLen + 15)&0xFFFFFFFC, 1, fpInc);
			}

			else if (cmdType == del_command)
			{
				readBytes = fread(&delType, 4, 1, fp);
				if (readBytes <= 0)
				{
                    pThis->e_log->writelog("%s:%d Invalid delete type %d.\n", FL, LN, delType);
                    pThis->GotoNextRecord(fp);
                    continue;
                }
				
                readBytes = fread(&deleteid, 8, 1, fp);
				if (readBytes <= 0 || deleteid <= 0)
                {
                    pThis->e_log->writelog("%s:%d Invalid id %lu.DelType is %d.\n", FL, LN, deleteid, delType);
                    pThis->GotoNextRecord(fp);
                    continue;
                }
				// delType: 0 按docid删除文档
				// delType: 1 按groupid删除文档
				pThis->DelDoc(deleteid, delType);
				memcpy(pIncBuf, DATA_HEADER, 4);
				memcpy(pIncBuf + 4,  &cmdType, 4);
				memcpy(pIncBuf + 8,  &delType, 4);
				memcpy(pIncBuf + 12, &deleteid,  8);
				fwrite(pIncBuf, 20, 1, fpInc);
			}

			else if ( cmdType == update_command)
			{
				
			}
			else
			{
				pThis->e_log->writelog("%s:%d Command(%d) is unrecognizable.\n", FL, LN, cmdType);
			}
			// read the next record header
			readBytes = fread(recvDataHead, 4, 1, fp);
			if (readBytes < 0 || memcmp(recvDataHead, DATA_HEADER, 4) != 0)
			{
				pThis->e_log->writelog("%s:%d Read data header error!\n", FL, LN);
				pThis->GotoNextRecord(fp);
			}
		}
		CloseFile(fpInc);
		CloseFile(fp);
		RemoveFile(addFileName);
	}
	pThis->p_log->writelog("%s:%d Gather::ThreadAddData exit.\n", FL, LN);
	FreeObj(pMemory);
	FreeObj(pDoc);
    return NULL;
}

THREADFUNTYPE Gather::ThreadGenPushData(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	assert(pThis);
	var_4 iRet, fileNum = 0, outBufLen = ALLOC_SIZE, need_update = 0;
    var_4 i, j, iLen;
	var_u4 total, current, alloced, maxid;
	var_1 foldPath[MAX_FILE_PATH] = {0};
    var_1 fname[MAX_FILE_PATH]    = {0};
	var_1 file_name_new[MAX_FILE_PATH] = {0};
    var_1 fid = 0; // the ith file handle
	uLong tmpBufLen;
    FILE **fpAdds = NULL;
    FILE **fpDels = NULL;
	var_1 *pMemory = NULL;
    var_1 *tmpBuf = NULL;
	var_1 *outBuf = NULL;
    DOC_BUF *pDoc = NULL;

	pMemory = (var_1*)calloc(1, ALLOC_SIZE<<2);
    assert(pMemory);
	for (var_4 i = 0; i < pThis->m_cfg->index_group_num; i++)
    {
        fileNum += pThis->m_cfg->index_num[i];
    }
    // init files
    fpDels = (FILE**)calloc(fileNum + 1, sizeof(FILE*));
    if (fpDels == NULL)
    {
        pThis->e_log->writelog("%s:%d Alloc delete fps error.\n", FL, LN);
        return NULL;
    }
    fpAdds = (FILE**)calloc(fileNum + 1, sizeof(FILE*));
    if (fpAdds == NULL)
    {
        pThis->e_log->writelog("%s:%d Alloc add fps error.\n", FL, LN);
    	return NULL;
	}
    tmpBuf = (var_1*)pMemory;
    outBuf = (var_1*)(pMemory + ALLOC_SIZE);
	pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
	assert (pDoc && fpAdds && fpDels);
	
	while (!pThis->m_bExit)
	{

		// generate xml file for updated groupid, here will use update_data_lock

		// lock
		pThis->update_data_lock.lock();
		pThis->m_pGroupAlloc->stat(total, current, alloced, maxid);

		need_update = 0;
		for (i = 1; i < maxid; i++)
		{
			if (pThis->bm_modify[i] == 1)
			{
				need_update = 1;
				break;
			}
		}
		if (!need_update)
		{
			pThis->update_data_lock.unlock();
			sleep(pThis->m_cfg->update_interval);
			continue;
		}

        for (i = 0, fid = 0; i < pThis->m_cfg->index_group_num; i++)
        {
            for (j = 0; j < pThis->m_cfg->index_num[i]; j++)
            {
                SPRINT(foldPath, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d", pThis->m_cfg->send_path, SLASH, i, SLASH, j);
                if (IsFolderEmpty(foldPath))
                {
                    pThis->FileSerial[i][j] = 1;
                }
                SPRINT(fname, MAX_FILE_PATH, "%s%sDelxml_%04d.dat.tmp", foldPath, SLASH, pThis->FileSerial[i][j]);
                fpDels[fid] = fopen(fname, "wb");
                if (fpDels[fid] == NULL)
                {
                    pThis->e_log->writelog("%s:%d open file:%s error.\n", FL, LN, fname);
					goto update_end;
                }
                SPRINT(fname, MAX_FILE_PATH, "%s%sAddxml_%04d.dat.tmp", foldPath, SLASH, pThis->FileSerial[i][j]);
                fpAdds[fid] = fopen(fname, "wb");
                if (fpAdds[fid] == NULL)
                {
                    pThis->e_log->writelog("%s:%d open file:%s error.\n", FL, LN, fname);
					goto update_end;
                }

				if ((++pThis->FileSerial[i][j]) % 10000 == 0)
				{
					pThis->FileSerial[i][j]++;
				}
				fid++;
            }
        }
		for (i = 1; i < maxid; i++)
		{
			if (pThis->bm_modify[i] == 0)
                continue;
            var_4 machine_num = 0; // processed machines
            // write to each group
			// del xml
			if (pThis->bm_groupStatus[i] != 0)
			{
            	for (j = 0; j < pThis->m_cfg->index_group_num; j++)
           	 	{
               		var_4 mod_idx = pThis->bm_groupStatus[i] % pThis->m_cfg->index_num[j];
                	if (fprintf(fpDels[mod_idx + machine_num], "%lu\n", pThis->bm_groupStatus[i]) > 0)
                	{
                    	// del success
               	 	}
                	machine_num += pThis->m_cfg->index_num[j];
            	}
			}
            // add xml
            if (pThis->m_pGroupBase[i].leadid != 0 && pThis->m_pGroupBase[i].number > 0)
            {
				tmpBufLen = ALLOC_SIZE;
                iRet = pThis->GetDocData(pThis->m_pGroupBase[i].leadid, tmpBuf, &tmpBufLen, pMemory, 2);
                if (iRet <0)
                {
                    pThis->e_log->writelog("%s:%d[ERRNO:%d] Get data of doc error.\n", FL, LN, iRet);
                    continue;
                }
                iRet = ProcessBufToDoc(tmpBuf, tmpBufLen, pDoc);
                if (iRet < 0)
                {
                    pThis->e_log->writelog("%s:%d[ERRNO:%d] Process buffer to doc error.\n", FL, LN, iRet);
                    continue;
                }
                iRet = GenerateAddXml(pDoc, pThis->m_pGroupBase[i].number, outBuf, outBufLen);
                if (iRet <= 0)
				{
                    continue;
                }
                var_4 machine_num = 0; // processed machines
                // write to each group
                for (j = 0; j < pThis->m_cfg->index_group_num; j++)
                {
                    var_u4 mod_idx = pThis->m_pGroupBase[i].leadid % pThis->m_cfg->index_num[j];
                    if (fwrite(&iRet, 4, 1, fpAdds[mod_idx + machine_num]) == 1 &&
                            fwrite(outBuf, iRet, 1, fpAdds[mod_idx + machine_num]) == 1)
                    {
                        // add success
                    }
                    machine_num += pThis->m_cfg->index_num[j];
                }
            }
            pThis->bm_modify[i] = 0;
            pThis->bm_groupStatus[i] = pThis->m_pGroupBase[i].leadid;
		}
update_end:
		//close file
    	for (i = 0; i < fileNum; i++)
    	{
       		CloseFile(fpDels[i]);
       		CloseFile(fpAdds[i]);
    	}
		// rename XXXX.dat.tmp to XXXX.dat
		struct dirent *direntp;
		DIR *dirp;
		for (i = 0; i < pThis->m_cfg->index_group_num; i++)
		{
			for (j=0; j<pThis->m_cfg->index_num[i];j++)
			{
				SPRINT(foldPath, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d", pThis->m_cfg->send_path, SLASH, i, SLASH, j);
				if (!(dirp = opendir(foldPath)))
				{
					pThis->e_log->writelog("%s:%d Open dir %s error.\n", FL, LN);
					continue;
				}
				while (direntp = readdir(dirp))
				{
					if (direntp->d_type == 8)
					{
						iLen = strlen(direntp->d_name);
						// 先rename Addxml文件
						if (iLen < 10 || strncmp(direntp->d_name + iLen - 4, ".tmp", 4))
						{
							continue;
						}
						 
						if (strncmp(direntp->d_name, "Add", 3))
						{
							continue;
						}

						SPRINT(fname, MAX_FILE_PATH, "%s%s%s", foldPath, SLASH, direntp->d_name);
						SPRINT(file_name_new, strlen(fname) - 3, "%s", fname);
						RenameFile(fname, file_name_new);

						// rename Delxml文件
						SPRINT(fname, MAX_FILE_PATH, "%s%sDel%s", foldPath, SLASH, direntp->d_name + 3);
						SPRINT(file_name_new, strlen(fname) - 3, "%s", fname);
						RenameFile(fname, file_name_new);
					}
				}
				closedir(dirp);
			}
		}
		iRet = msync(pThis->bm_groupBuf, pThis->bm_groupSize,MS_SYNC);
		if (iRet != 0)
		{
			pThis->e_log->writelog("%s:%d[ERRNO:%d] Msync bm_groupStatus error.\n", FL, LN, iRet);
		}
		pThis->update_data_lock.unlock();
		sleep(pThis->m_cfg->update_interval);
	}
    FreeObj(pMemory);
    FreeObj(fpAdds);
	FreeObj(fpDels);
	FreeObj(pDoc);
	return NULL;
}

THREADFUNTYPE Gather::ThreadPushData(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	var_4 groupNum = spThread->iBufSize / 256;
	var_4 machineNum = spThread->iBufSize % 256;
	assert(pThis);

	var_4 iRet = 0, docLen;
	var_4 file_index = 1;
	var_1 send_folder[MAX_FILE_PATH] = {0};
	struct stat st_add, st_del;
	FILE *fp_add = NULL;
	FILE *fp_del = NULL;
	var_1 *pSendAddbuf = NULL;
	var_1 *pSendDelbuf = NULL;
	var_4 read_add_flag;
	var_4 read_del_flag;
	var_8 addLen, delLen, iLen;

	var_1 *ip = pThis->m_cfg->index_ip[groupNum][machineNum];
	var_1 *pMemory  = (var_1*)calloc(1, ALLOC_SIZE<<2);
	var_1 *pMemory1 = (var_1*)calloc(1, 1u<<31);
	var_1 *pMemory2 = (var_1*)calloc(1, 1u<<31);
	assert(pMemory && pMemory1 && pMemory2);

	var_4 port = pThis->m_cfg->index_port[groupNum][machineNum];
	iRet = _snprintf(send_folder, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d%s", pThis->m_cfg->send_path, SLASH, groupNum, SLASH, machineNum, SLASH);
	assert(iRet > 0);

	while(!pThis->m_bExit)
	{
		var_1 file_send_add[MAX_FILE_PATH] = {0};
		var_1 file_send_del[MAX_FILE_PATH] = {0};
		
		if (!pThis->m_push_flag[groupNum])
		{
			pThis->MergeSmallFiles(send_folder, pMemory);
			//get first data
			iRet = pThis->GetSendFiles(send_folder, (var_1 *)"Delxml_", file_send_add, file_send_del);
			if (iRet < 0)
			{
				sleep(2);
				continue;
			}
			// 正在生成全局数据，不允许发送
			if (strstr(file_send_add, "Addxml_0000")!=NULL)
			{
				sleep(60);
				continue;
			}

			if (strstr(file_send_del, "Delxml_0000")!=NULL)
			{
				sleep(60);
				continue;
			}
			//send data
			if(stat(file_send_add, &st_add) == 0 && stat(file_send_del, &st_del) == 0)
			{
				addLen = st_add.st_size + IDXHEADLEN;
				delLen = st_del.st_size + IDXHEADLEN;
				// 确定文件大小不再变化
				while (true)
				{
					sleep(1);
					stat(file_send_add, &st_add);
					stat(file_send_del, &st_del);
					if (addLen == st_add.st_size + IDXHEADLEN && delLen == st_del.st_size + IDXHEADLEN)
					{
						break;
					}
					addLen = st_add.st_size + IDXHEADLEN;
					delLen = st_del.st_size + IDXHEADLEN;
				}
				
				if(addLen >= 1u<<31 || delLen >= 1u<<31)
				{
					sleep(60);
					continue;
				}

				pSendAddbuf = (var_1 *)(pMemory1);
				pSendDelbuf = (var_1 *)(pMemory2);

				fp_add = fopen(file_send_add, "rb");
				fp_del = fopen(file_send_del, "rb");
				if (fp_add == NULL || fp_del == NULL)
				{
					CloseFile(fp_add);
					CloseFile(fp_del);
					pThis->e_log->writelog("Open file in ThreadPushData error, add_file is %s, del_file is %s\n", file_send_add, file_send_del);
					sleep(1);
					continue;
				}

				read_add_flag = 0;
				read_del_flag = 0;

				if (st_add.st_size == 0)
				{
					read_add_flag = 1;
				}
				else
				{
					read_add_flag = fread(pSendAddbuf + IDXHEADLEN, st_add.st_size, 1, fp_add);
				}
				// 如果Addxml不为空，做强校验保证pSendAddbuf的正确性
				iLen = IDXHEADLEN;
				while (iLen < addLen)
				{
					docLen = *(var_4*)(pSendAddbuf + iLen);
					iLen = iLen + docLen + 4;
					if (iLen > addLen) // 有半条数据
					{
						pThis->e_log->writelog("%s:%d Incomplete data.file %s\n", FL, LN, file_send_add);
						assert(0);
					}
				}

				if (st_del.st_size == 0)
				{
					read_del_flag = 1;
				}
				else
				{
					read_del_flag = fread(pSendDelbuf + IDXHEADLEN, st_del.st_size, 1, fp_del);
				}
				CloseFile(fp_add);
				CloseFile(fp_del);
				if (read_add_flag == 1 && read_del_flag == 1)
				{
					memcpy(pSendAddbuf, "SJ_INDEX",8);
					*(int *)(pSendAddbuf + 8) = 0;
					*(int *)(pSendAddbuf + 12) = addLen - IDXHEADLEN;
					memcpy(pSendDelbuf, "SJ_INDEX",8);
					*(int *)(pSendDelbuf + 8) = 1;
					*(int *)(pSendDelbuf + 12) = delLen - IDXHEADLEN;
ReSend:
					//must first send del file
					iRet = pThis->SendDataToIndex(pSendDelbuf, delLen, ip, port);
					if (iRet < 0)
					{
						sleep(10);
						goto ReSend;
					}

					iRet = pThis->SendDataToIndex(pSendAddbuf, addLen, ip, port);
					if (iRet < 0)
					{
						sleep(10);
						goto ReSend;
					}
					//remove data
					RemoveFile(file_send_add);
					RemoveFile(file_send_del);
				}
				else
				{
					pThis->e_log->writelog("Read data from file in ThreadPushData error, add_file is %s, del_file is %s\n", file_send_add, file_send_del);
				}
			}
		}
		else
		{
			sleep(30);
		}
	}
	FreeObj(pMemory);
	FreeObj(pMemory1);
	FreeObj(pMemory2);
	return NULL;
}

THREADFUNTYPE Gather::ThreadPushAllData(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	var_4 groupNum = spThread->iBufSize;
	assert(pThis);

	bool break_flag = false;
	var_4 i, iRet, mod_idx,record_count=0;
	FILE **fpAdds = NULL;
	FILE **fpDels = NULL;
	DOC_BUF *pDoc = NULL;
	var_1 *tmpBuf = NULL;
	var_1 *outBuf = NULL;
	var_1 *pMemory = NULL;
	uLong tmpBufLen = ALLOC_SIZE;
	var_4 outBufLen = ALLOC_SIZE;
	GROUP_INFO *groupBaseBak = NULL;
	var_1 foldername[MAX_FILE_PATH] = {0};
	var_1 filename[MAX_FILE_PATH] = {0};
	var_1 file_name_new[MAX_FILE_PATH] = {0};
	var_4 fileNum = pThis->m_cfg->index_num[groupNum];
	fpDels = (FILE**)calloc(fileNum + 1, sizeof(FILE*));
	fpAdds = (FILE**)calloc(fileNum + 1, sizeof(FILE*));
	pMemory = (var_1*)calloc(1, ALLOC_SIZE<<2);
	pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
	assert(pDoc && fpDels && fpAdds && pMemory);

	tmpBuf = (var_1*)pMemory;
	outBuf = (var_1*)(pMemory + ALLOC_SIZE);
	while (!pThis->m_bExit)
	{
		//check whether need to push all data
		if (pThis->m_push_flag[groupNum])
		{
			groupBaseBak = (GROUP_INFO*)calloc(pThis->m_cfg->max_group_num, sizeof(GROUP_INFO));
			if (groupBaseBak == NULL)
			{
				pThis->e_log->writelog("%s:%d malloc memory failed in ThreadPushAllData.\n", FL, LN);
				sleep(5);
				continue;
			}
			break_flag = false;
			//get lock
			pThis->update_data_lock.lock();
			//copy index
			memcpy(groupBaseBak, pThis->m_pGroupBase, pThis->m_cfg->max_group_num * sizeof(GROUP_INFO));
			//release lock
			pThis->update_data_lock.unlock();
			//generate files to send folder
			for (i = 0; i < fileNum; i++)
			{
				iRet = _snprintf(foldername, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d%s", pThis->m_cfg->send_path, SLASH, groupNum, SLASH, i, SLASH);
				if (iRet <= 0)
				{
					break_flag = true;
					break;
				}

				while(!ClearFolder(foldername))
				{
					sleep(1);
				}

				//del file
				iRet = _snprintf(filename, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d%sDelxml_0000.dat", pThis->m_cfg->send_path, SLASH, groupNum, SLASH, i, SLASH);
				if (iRet <= 0)
				{
					break_flag = true;
					break;
				}
				fpDels[i] = fopen(filename, "wb");
				if (fpDels[i] == NULL)
				{
					break_flag = true;
					break;
				}
				//add file
				iRet = _snprintf(filename, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d%sAddxml_0000.dat", pThis->m_cfg->send_path, SLASH, groupNum, SLASH, i, SLASH);
				if (iRet <= 0)
				{
					break_flag = true;
					break;
				}
				fpAdds[i] = fopen(filename, "wb");
				if (fpAdds[i] == NULL)
				{
					break_flag = true;
					break;
				}
			}
			if (!break_flag)
			{
				pThis->p_log->writelog("Start generate global data ...\n");
				for(i = 0; i < pThis->m_cfg->max_group_num; i++)
				{
					if (groupBaseBak[i].leadid != 0 && groupBaseBak[i].number > 0)
					{
						tmpBufLen = ALLOC_SIZE;
						iRet = pThis->GetDocData(groupBaseBak[i].leadid, tmpBuf, &tmpBufLen, pMemory, 2);
						if (iRet < 0)
						{
						    pThis->e_log->writelog("%s:%d[ERRNO:%d] Get data of doc error.\n", FL, LN, iRet);
                            continue;
						}
                        iRet = ProcessBufToDoc(tmpBuf, tmpBufLen, pDoc);
                        if (iRet < 0)
						{
						    pThis->e_log->writelog("%s:%d[ERRNO:%d] Process buffer to doc error.\n", FL, LN, iRet);
						    continue;
						}
                        iRet = GenerateAddXml(pDoc, groupBaseBak[i].number, outBuf, outBufLen);
                        if (iRet <= 0)
                        {
                            continue;
                        }
                        mod_idx = groupBaseBak[i].leadid % fileNum;
                        if (fwrite(&iRet, 4, 1, fpAdds[mod_idx]) == 1 && fwrite(outBuf, iRet, 1, fpAdds[mod_idx]) == 1)
                        {
                            //add success
						}
						if (record_count % 100000 == 0)
						{
							pThis->p_log->writelog("ThreadPushAllData generate %u records.\n", record_count);
						}
						record_count++;
					}
				}
				pThis->m_push_flag[groupNum] = 0;
				pThis->p_log->writelog("Total generate %u records.\n", record_count);
			}
			for (i = 0; i < fileNum; i++)
			{
				CloseFile(fpAdds[i]);
				CloseFile(fpDels[i]);
			}
			FreeObj(groupBaseBak);
		}
		sleep(60);
	}
	FreeObj(fpDels);
	FreeObj(fpAdds);
	FreeObj(tmpBuf);
	FreeObj(outBuf);
	FreeObj(pDoc);
	return NULL;
}

THREADFUNTYPE Gather::ThreadSave(var_vd* vpInfo)
{
	THREAD_INFO *spThread = (THREAD_INFO*)vpInfo;
	Gather *pThis = (Gather *)spThread->vpInfo;
	assert(pThis);
	//add detail save info here, you can use pThis->m_cfg to get configs
	//will use update _data_lock to block data add and data updat
	DIR *dirp;
	struct dirent *direntp;
	var_1 file_name_old[MAX_FILE_PATH] = {0};
	var_1 file_name_new[MAX_FILE_PATH] = {0};

	var_1 file_idx[MAX_FILE_PATH]	     = {0};
	var_1 file_data[MAX_FILE_PATH]	     = {0};
	var_1 file_idx_alloc[MAX_FILE_PATH]	 = {0};
	var_1 file_data_alloc[MAX_FILE_PATH] = {0};
	var_1 file_doc_map[MAX_FILE_PATH]	 = {0};
	var_1 file_finger_map[MAX_FILE_PATH] = {0};
	var_1 file_save_flag[MAX_FILE_PATH]  = {0};
    var_1 backupDataPath[MAX_FILE_PATH]  = {0};
    var_1 backupIdxPath[MAX_FILE_PATH]   = {0};
	// save to *.bak file, then rename it
	if (SPRINT(file_idx,       MAX_FILE_PATH, "%s%s%s.bak", pThis->m_cfg->data_path, SLASH, pThis->m_cfg->file_idx)       <= 0 ||
		SPRINT(file_data,      MAX_FILE_PATH, "%s%s%s.bak", pThis->m_cfg->data_path, SLASH, pThis->m_cfg->file_data)      <= 0 ||
		SPRINT(file_idx_alloc, MAX_FILE_PATH, "%s%s%s.bak", pThis->m_cfg->data_path, SLASH, pThis->m_cfg->file_idx_alloc) <= 0 ||
		SPRINT(file_data_alloc,MAX_FILE_PATH, "%s%s%s.bak", pThis->m_cfg->data_path, SLASH, pThis->m_cfg->file_data_alloc)<= 0 ||
		SPRINT(file_doc_map,   MAX_FILE_PATH, "%s%s%s.bak", pThis->m_cfg->data_path, SLASH, pThis->m_cfg->file_doc_map)   <= 0 ||
		SPRINT(file_finger_map,MAX_FILE_PATH, "%s%s%s.bak", pThis->m_cfg->data_path, SLASH, pThis->m_cfg->file_finger_map)<= 0 ||
		SPRINT(file_save_flag, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->data_path, SLASH, pThis->m_cfg->file_save_flag) < 0)
	{
		pThis->e_log->writelog("%s:%d Get file name error.\n", FL, LN);
		return NULL;
	}
	var_4 iRet = 0, sleepCount = 1, backupCount = 1,  dataNum = 0, groupNum = 0;
	var_4 iLen = 0;
	while (!pThis->m_bExit)
	{
		if (sleepCount > (pThis->m_cfg->save_interval / 60))
		{
			pThis->save_data_lock.lock();
			if (backupCount == pThis->m_cfg->backup_count)
			{
				SPRINT(backupDataPath, MAX_FILE_PATH, "%s%sdata%s", pThis->m_cfg->store_backup_path, SLASH, SLASH);
				SPRINT(backupIdxPath,  MAX_FILE_PATH, "%s%sidx%s",  pThis->m_cfg->store_backup_path, SLASH, SLASH);
				pThis->m_data_store->backup_prepare();
				if (pThis->m_data_store->save_idx())
				{
					pThis->e_log->writelog("%s:%d Failed to call save_idx.\n", FL, LN);
					pThis->save_data_lock.unlock();
					sleep(300);
					continue;
				}
				if (pThis->m_data_store->backup(backupDataPath, backupIdxPath))
				{
					pThis->e_log->writelog("%s:%d Failed to call backup.\n", FL, LN);
					pThis->save_data_lock.unlock();
					sleep(300);
					continue;
				}
				pThis->m_data_store->backup_finish();
			}
			else
			{
				if (pThis->m_data_store->save_idx())
				{
					pThis->e_log->writelog("%s:%d Failed to call save_idx.\n", FL, LN);
					pThis->save_data_lock.unlock();
					sleep(300);
					continue;
				}
			}
			pThis->update_data_lock.lock();
			if ((iRet = SaveFile(file_idx, pThis->m_pGroupBase, pThis->m_cfg->max_group_num, 1)) < 0)
			{
				pThis->e_log->writelog("%s:%d Save file %s error.Errno %d.\n", FL, LN, file_idx, iRet);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;
			}
			if ((iRet = SaveFile(file_data, pThis->m_pDataBase, pThis->m_cfg->max_data_num, 1)) < 0)
			{
				pThis->e_log->writelog("%s:%d Save file %s error.Errno %d.\n", FL, LN, file_data, iRet);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;
			}
			if ((iRet = pThis->m_pGroupAlloc->save(file_idx_alloc)) < 0)
			{
				pThis->e_log->writelog("%s:%d Save file %s error.Errno %d.\n", FL, LN, file_idx_alloc, iRet);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;
			}
			if ((iRet = pThis->m_pDataAlloc->save(file_data_alloc)) < 0)
			{
				pThis->e_log->writelog("%s:%d Save file %s error.Errno %d.\n", FL, LN, file_data_alloc, iRet);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;
			}
			if ((iRet = pThis->m_pFingerGroup->Save(file_finger_map)) < 0)
			{
				pThis->e_log->writelog("%s:%d Save file %s error.Errno %d.\n", FL, LN, file_finger_map, iRet);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;
			}
			if ((iRet = pThis->m_pDocInfo->Save(file_doc_map)) < 0)
			{
				pThis->e_log->writelog("%s:%d Save file %s error.Errno %d.\n", FL, LN, file_doc_map, iRet);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;
			}
			iRet = msync(pThis->bm_groupBuf, pThis->bm_groupSize,MS_SYNC);
			if (iRet != 0)
			{
				pThis->e_log->writelog("%s:%d Msync bm_groupStatus error.\n", FL, LN);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;
			}
			if (!(dirp = opendir(pThis->m_cfg->data_path)))
			{
				pThis->e_log->writelog("%s:%d Open directory %s error.\n", pThis->m_cfg->data_path);
				pThis->save_data_lock.unlock();
				pThis->update_data_lock.unlock();
				sleep(300);
				continue;

			}
			// save backup finish
			CreateFile(file_save_flag);
			while (direntp = readdir(dirp))
			{
				if (direntp->d_type == 8)
				{
					SPRINT(file_name_old, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->data_path, SLASH, direntp->d_name);
					iLen = strlen(file_name_old);
					if (!strncmp(file_name_old + iLen - 4, ".bak", 4))
					{
						SPRINT(file_name_new, iLen - 3, "%s", file_name_old);
						RenameFile(file_name_old, file_name_new);
					}
				}
			}
			closedir(dirp);
			RemoveFile(file_save_flag);
			if (backupCount == pThis->m_cfg->backup_count)
			{
				//backup
				if (!(dirp = opendir(pThis->m_cfg->data_path)))
				{
					pThis->e_log->writelog("%s:%d Open directory %s error.\n", pThis->m_cfg->backup_path);
				}
				else
				{
					while ((direntp = readdir(dirp)))
					{
						if (direntp->d_type == 8)
						{
							SPRINT(file_name_old, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->data_path, SLASH, direntp->d_name);
							SPRINT(file_name_new, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->backup_path, SLASH, direntp->d_name);
							CopyFile(file_name_old, file_name_new);
						}
					}
				}
				closedir(dirp);
			}
			sleepCount = 1;
			backupCount++;
			if(backupCount > pThis->m_cfg->backup_count)
			{
				backupCount = 1;
			}
			// remove recv.inc
			var_1 incFile[MAX_FILE_PATH] = {0};
			SPRINT(incFile, MAX_FILE_PATH, "%s%s%s", pThis->m_cfg->recv_path, SLASH, "recv.inc");
			RemoveFile(incFile);
			pThis->save_data_lock.unlock();
			pThis->update_data_lock.unlock();
			// 记录当前的数据量
			var_4 dataNum, groupNum;
			pThis->GetStatus(dataNum, groupNum, 0);
			pThis->p_log->writelog("%s:%d Now data num is %d and group num is %d.\n", FL, LN, dataNum, groupNum);
		}
		else
		{
			sleepCount++;
			sleep(60);
		}
	}
	return NULL;
}

// delete doc
var_4 Gather::DelDoc(var_u8 deleteid, var_4 delType)
{
	DOC_INFO docu;
	var_u4 groupid = 0, dataid = 0;
	GROUP_INFO *pGroupInfo    = NULL;
	DATA_INFO  *pDataInfo     = NULL;
	DATA_INFO  *pTempDataInfo = NULL;

	if (delType != 0 && delType != 1)
	{
		e_log->writelog("%s:%d DelDoc delType invalid.\n", FL, LN);
		return -10;
	}

	update_data_lock.lock();
	// 按docid删除文档
	if (delType == 0)
	{
		// get group id by docid
		if (m_pDocInfo->Search(deleteid, docu) <= 0)
		{
			e_log->writelog("%s:%d Can not found docid %lu\n", FL, LN, deleteid);
			update_data_lock.unlock();
			return -1;
		}
		groupid = docu.groupid;
		pGroupInfo = m_pGroupBase + groupid;
		dataid = pGroupInfo->next;

		// find the doc
		if (dataid == docu.dataid)
		{
			pDataInfo = m_pDataBase + dataid;
			pGroupInfo->next = pDataInfo->next;
			pGroupInfo->number--;
			memset(pDataInfo, 0, sizeof(DATA_INFO));
		}
		else
		{
			while (dataid)
			{
				pDataInfo = m_pDataBase + dataid;
				// find the doc
				if (pDataInfo->next == docu.dataid)
				{
					pTempDataInfo = m_pDataBase + docu.dataid;
					pDataInfo->next = pTempDataInfo->next;
					pGroupInfo->number--;
					memset(pTempDataInfo, 0, sizeof(DATA_INFO));
					break;
				}
				dataid = pDataInfo->next;
			}
		}
		// erase data and map
		m_pDataAlloc->freeID(docu.dataid);
		m_pDocInfo->Del(deleteid);
		m_data_store->del(deleteid);
		// 重新选leadid
		if (pGroupInfo->leadid == deleteid && pGroupInfo->number > 0)
		{
			dataid = pGroupInfo->next;
			pDataInfo = m_pDataBase + dataid;
			pGroupInfo->leadid = pDataInfo->docid;
			pGroupInfo->weight = pDataInfo->weight;
			pGroupInfo->time = pDataInfo->time;
			dataid = pDataInfo->next;
			while (dataid)
			{
				pDataInfo = m_pDataBase + dataid;
				if ((pDataInfo->weight + 2)/3 > (pGroupInfo->weight+2)/3 ||
						((pDataInfo->weight + 2) /3 == (pGroupInfo->weight +2)/3 && pDataInfo->time < pGroupInfo->time))
				{
					pGroupInfo->leadid = pDataInfo->docid;
					pGroupInfo->weight = pDataInfo->weight;
					pGroupInfo->time = pDataInfo->time;
				}
				dataid = pDataInfo->next;
			}
		}
		bm_modify[groupid] = 1;
	}
	// 按finger删除文档
	if (delType == 1)
	{
		if (m_pFingerGroup->Search(deleteid, groupid) <= 0)
		{
			e_log->writelog("%s:%d Can not found finger %lu\n", FL, LN, deleteid);
			update_data_lock.unlock();
			return -2;
		}
		pGroupInfo = m_pGroupBase + groupid;
		dataid = pGroupInfo->next;
		while (dataid)
		{
			pDataInfo = m_pDataBase + dataid;	
			m_pDataAlloc->freeID(dataid);
			m_pDocInfo->Del(pDataInfo->docid);
			m_data_store->del(pDataInfo->docid);
			dataid = pDataInfo->next;
		}
		pGroupInfo->number = 0;
	}
	// if just one node, free the group
	if (pGroupInfo->number == 0)
	{
		m_pFingerGroup->Del(pGroupInfo->finger);
		memset(pGroupInfo, 0, sizeof(GROUP_INFO));
		m_pGroupAlloc->freeID(groupid);
		bm_modify[groupid] = 1;
	}
	update_data_lock.unlock();
	e_log->writelog("%s:%d Delete(delType:%d) %lu successfully.\n", FL, LN, delType, deleteid);
	return 0;
}

var_4 Gather::Recover(var_1* pMemory, var_4 loadBinData)
{
	//recover data and run inc file again to restore all data, if there is no data, init new one
	DIR *dirp;
    struct dirent *direntp;
	struct stat st;
	FILE* fp = NULL;
	FILE* fpBak = NULL;
	var_1* docBuf = NULL;
	var_4 i, j, maxid, iRet, iLen;
	var_8 modify_time, file_size;
	
	var_1 file_path[MAX_FILE_PATH]		 = {0};
	var_1 file_name[MAX_FILE_PATH]		 = {0};
    var_1 file_name_new[MAX_FILE_PATH] 	 = {0};
    var_1 file_name_add[MAX_FILE_PATH] 	 = {0};
    var_1 file_name_del[MAX_FILE_PATH] 	 = {0};
	var_1 file_name_bak[MAX_FILE_PATH]   = {0};
  
	var_1 file_idx[MAX_FILE_PATH]        = {0};
	var_1 file_data[MAX_FILE_PATH]       = {0};
	var_1 file_idx_alloc[MAX_FILE_PATH]  = {0};
	var_1 file_data_alloc[MAX_FILE_PATH] = {0};
	var_1 file_doc_map[MAX_FILE_PATH]    = {0};
	var_1 file_finger_map[MAX_FILE_PATH] = {0};
	var_1 file_group_status[MAX_FILE_PATH] = {0};
	var_1 file_save_flag[MAX_FILE_PATH]	 = {0};

	if (m_cfg->max_group_num <= 0 || m_cfg->max_data_num <= 0 /*some other conditions need be checked.*/)
	{
		e_log->writelog("%s:%d Check parameters error.\n", FL, LN);
		return -1;
	}

	if (FileExists(m_cfg->data_path) == true)
	{
		if (CreateDir(m_cfg->data_path) != 0)
		{
			e_log->writelog("%s:%d Create data path '%s' error!\n", FL, LN, m_cfg->data_path);
			return -2;
		}
	}
	if (FileExists(m_cfg->backup_path) == false)
	{
		if (CreateDir(m_cfg->backup_path) != 0)
		{
			e_log->writelog("%s:%d Create backup path '%s' error!\n", FL, LN, m_cfg->backup_path);
			return -3;
		}
	}
    if (SPRINT(file_idx,       MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_idx)       <= 0 ||
		SPRINT(file_data,      MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_data)      <= 0 ||
		SPRINT(file_idx_alloc, MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_idx_alloc) <= 0 ||
		SPRINT(file_data_alloc,MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_data_alloc)<= 0 ||
		SPRINT(file_doc_map,   MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_doc_map)   <= 0 ||
		SPRINT(file_finger_map,MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_finger_map)<= 0 ||
		SPRINT(file_group_status, MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_group_status) <= 0 ||
		SPRINT(file_save_flag, MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, m_cfg->file_save_flag) <= 0 )
	{
		e_log->writelog("%s:%d SPRINT idx/data filename error.\n", FL, LN);
		return -4;
	}
	// create IDAlloc object
	m_pGroupAlloc = new IDAlloc();
	m_pDataAlloc  = new IDAlloc();
	if (NULL == m_pGroupAlloc || NULL == m_pDataAlloc)
	{
		e_log->writelog("%s:%d Create IDAlloc object error.\n", FL, LN);
		return -5;
	}
	// init maps
	m_pDocInfo = new Fixsizehash<var_u8, DOC_INFO>(m_cfg->max_data_num, m_cfg->max_data_num - 1);
	m_pFingerGroup = new Fixsizehash<var_u8, var_u4>(m_cfg->max_group_num, m_cfg->max_group_num - 1);
	if (m_pDocInfo == NULL || m_pFingerGroup == NULL)
	{
		e_log->writelog("%s:%d Create Fixsizehash error.\n", FL, LN);
		return -6;
	}
	if (m_pDocInfo->Init(e_log) <= 0 || m_pFingerGroup->Init(e_log) <= 0) 
	{
		e_log->writelog("%s:%d Fixsizehash Init error.\n", FL, LN);
		return -7;
	}
	//
	if (!(dirp = opendir(m_cfg->data_path)))
	{
		e_log->writelog("%s:%d Open dir %s error.\n", FL, LN, m_cfg->data_path);
		return -8;
	}
	if (FileExists(file_save_flag))
	{
		// if *.bak exist, rename it first
		while (direntp = readdir(dirp))
        {
        	if (direntp->d_type == 8)
            {
				SPRINT(file_name, MAX_FILE_PATH, "%s%s%s", m_cfg->data_path, SLASH, direntp->d_name);
                iLen = strlen(file_name);
                if (strncmp(file_name + iLen - 4, ".bak", 4) == 0)
                {
                    SPRINT(file_name_new, iLen - 3, "%s", file_name);
                    RenameFile(file_name, file_name_new);
                }
            }
        }
		RemoveFile(file_save_flag);
	}
	closedir(dirp);
	for (i =0; i <m_cfg->index_group_num; i++)
	{
		for (j=0; j<m_cfg->index_num[i]; j++)
		{
			maxid = 0, modify_time = 0;
			SPRINT(file_path, MAX_FILE_PATH, "%s%sgroup%02d%smachine%02d", m_cfg->send_path, SLASH, i, SLASH, j);
			if (!(dirp = opendir(file_path)))
			{
				e_log->writelog("%s:%d Open dir %s error.\n", FL, LN, file_path);
				return -9;
			}
			var_1 file_name_id[5] = {0};
			while (direntp = readdir(dirp))
			{
				if (direntp->d_type == 8)
				{
					SPRINT(file_name_bak, MAX_FILE_PATH, "%s%s%s", file_path, SLASH, direntp->d_name);
					if (stat(file_name_bak, &st) == -1)
					{
						continue;
					}

					// 合并小文件时会生成Addxml_xxxx.bak和Delxml_xxxx.bak文件
					iLen = strlen(direntp->d_name);
					if (!strncmp(direntp->d_name + iLen - 4, ".bak", 4))
					{
						RemoveFile(file_name_bak);
						continue;
					}
					if (!strncmp(direntp->d_name, "Add", 3) && !strncmp(direntp->d_name + iLen - 4, ".tmp", 4))
					{
						continue;
					}
					// 删除ThreadGenPushData生成的.tmp文件
					// 如果Delxml_0001.dat.tmp不存在, Addxml_0001.dat.tmp肯定不存在
					// 如果Delxml_0001.dat.tmp存在，Addxml_0001.dat.tmp不一定存在
					SPRINT(file_name_bak, MAX_FILE_PATH, "%s%s%s", file_path, SLASH, direntp->d_name);
					iLen = strlen(file_name_bak);
					// Delxml_0001.dat.tmp
					if (!strncmp(direntp->d_name, "Del", 3))
					{
						SPRINT(file_name_add, MAX_FILE_PATH, "%s%sAdd%s", file_path, SLASH, direntp->d_name + 3);
						if (!strncmp(file_name_bak + iLen - 4, ".tmp", 4))
						{
							RemoveFile(file_name_bak);
							// 如果Addxml_0001.dat.tmp存在，删除它
							if (FileExists(file_name_add))
							{
								RemoveFile(file_name_add);
							}
							// 如果Addxml_0001.dat存在，删除它
							file_name_add[strlen(file_name_add) - 4] = '\0';
							if (FileExists(file_name_add))
							{
								RemoveFile(file_name_add);
							}
						}	
						// 如果只有Delxml_0001.dat，则删除它
						// Addxml_0001.dat可以单独存在
						else
						{
							if (stat(file_name_add, &st) == -1)
							{
								RemoveFile(file_name_bak);
							}
						}
						continue;
					}
				
					// 获取当前目录中Addxml文件最大编号，例如Addxml_0017.dat中的0017
					strncpy(file_name_id, direntp->d_name + 7, 4);
					maxid = (atoi(file_name_id) > maxid ? atoi(file_name_id):maxid);

					// get the last modified file
					SPRINT(file_name_add, MAX_FILE_PATH, "%s%s%s", file_path, SLASH, direntp->d_name);
					stat(file_name_add, &st);
					if (modify_time < st.st_mtime)
					{
						modify_time = st.st_mtime;
						SPRINT(file_name, MAX_FILE_PATH, "%s", file_name_add);
					}
				}
			}
			closedir(dirp);
			FileSerial[i][j] = maxid + 1;
			if (stat(file_name, &st) == -1)
			{    
				continue;
			} 
			docBuf = (var_1*)pMemory;
			
			fp =  fopen(file_name, "rb");
			if (fp == NULL)
			{
				e_log->writelog("%s:%d Open file %s error.\n", FL, LN, file_name);
				return -10;
			}
			SPRINT(file_name_bak, MAX_FILE_PATH, "%s.bak", file_name);
			fpBak = fopen(file_name_bak, "wb");
			if (fpBak == NULL)
			{
				CloseFile(fp);
				e_log->writelog("%s:%d Open file %s error.\n", FL, LN, file_name_bak);
				return -11;
			}
			var_1 end_flag = 0;
			file_size = GetFileSize(file_name);
			while (file_size != ftell(fp))
			{
				iRet = fread(&iLen, 4, 1, fp);
				if (iRet <= 0)
				{
					break;
				}
				iRet = fread(docBuf, iLen, 1, fp);
				if (iRet <= 0)
				{
					break;
				}
				iRet = fwrite(&iLen, 4, 1, fpBak);
				if (iRet <= 0)
				{
					break;
				}
				iRet = fwrite(docBuf, iLen, 1, fpBak);
				if (iRet <= 0)
				{
					e_log->writelog("%s:%d Check %s failed.Restart or delete it.\n", FL, LN, file_name);
					assert(0);
				}
			}
			CloseFile(fp);
			CloseFile(fpBak);
			RenameFile(file_name_bak, file_name);
		}
	}
	// m_pIdxBuf, m_pDataBuf need not to release
    m_pIdxBuf = (var_1*)malloc(m_cfg->max_group_num*sizeof(GROUP_INFO) + 4);
    m_pDataBuf= (var_1*)malloc(m_cfg->max_data_num*sizeof(DATA_INFO) + 4);
    bm_modify = (var_1*)calloc(m_cfg->max_group_num, 1);
	// init data
	if ((FileExists(file_idx_alloc) || FileExists(file_data_alloc) ||
          FileExists(file_idx) || FileExists(file_data) ||
          FileExists(file_doc_map) || FileExists(file_finger_map)) && loadBinData == 0)
    {
        // recovery
        p_log->writelog("%s:%d Start recovery data.\n", FL, LN);
        if (m_pGroupAlloc->restore(file_idx_alloc) <= 0)
		{
			e_log->writelog("%s:%d Restore file %s error.\n", FL, LN, file_idx_alloc);
			return -13;
		}

		if (m_pDataAlloc->restore(file_data_alloc) <= 0)
		{
			e_log->writelog("%s:%d Restore file %s error!\n", FL, LN, file_data_alloc);
			return -14;
		}

		if (LoadFile(file_idx, &m_pIdxBuf) < 0)
		{
			e_log->writelog("%s:%d Load file %s error.\n", FL, LN, file_idx);
			return -15;
		}
		m_pGroupBase = (GROUP_INFO*)(m_pIdxBuf + 4); // 4bytes: number of groups

		if (LoadFile(file_data, &m_pDataBuf) < 0)
		{
			e_log->writelog("%s:%d Load file %s error.\n", FL, LN, file_data);
			return -16;
		}
		m_pDataBase = (DATA_INFO*)(m_pDataBuf + 4); // 4bytes: data num

		if (m_pDocInfo->Load(file_doc_map) < 0)
		{
			e_log->writelog("%s:%d Load file %s error.\n", FL, LN, file_doc_map);
			return -17;
		}
		if (m_pFingerGroup->Load(file_finger_map) < 0)
		{
			e_log->writelog("%s:%d Load file %s error.\n", FL, LN, file_finger_map);
			return -18;
		}

		// mmap bm_groupStatus

		if (LoadFile(file_group_status, &bm_groupBuf, bm_groupFile, bm_groupSize) < 0)
		{
			FreeFiles();
			e_log->writelog("%s:%d Load file %s error.\n", FL, LN, file_group_status);
			return -20;
		}
		bm_groupStatus = (var_u8*)bm_groupBuf;
    }
	else
	{
	    p_log->writelog("%s:%d Start new data.\n", FL, LN);
		if (m_pGroupAlloc->init(m_cfg->max_group_num) <= 0)
		{
			FreeFiles();
			e_log->writelog("%s:%d Init group info error!\n", FL, LN);
			return -21;
		}
		if (m_pDataAlloc->init(m_cfg->max_data_num) <= 0)
		{
			FreeFiles();
			e_log->writelog("%s:%d Init data info error.\n", FL, LN);
			return -22;
		}

		if (CreateFile(file_idx, &m_pIdxBuf,  m_cfg->max_group_num*sizeof(GROUP_INFO) + 4, sizeof(GROUP_INFO)) < 0)
		{
			e_log->writelog("%s:%d Create file %s error.\n", FL, LN, file_idx);
			return -23;
		}
		*(var_4*)m_pIdxBuf = m_cfg->max_group_num;
		m_pGroupBase = (GROUP_INFO*)(m_pIdxBuf + 4);

		if (CreateFile(file_data, &m_pDataBuf, m_cfg->max_data_num*sizeof(DATA_INFO), sizeof(DATA_INFO)) < 0)
		{
			e_log->writelog("%s:%d Create file %s error.\n", FL, LN, file_data);
			return -24;
		}
        *(var_4*)m_pDataBuf = m_cfg->max_data_num;
        m_pDataBase = (DATA_INFO*)(m_pDataBuf + 4);

		bm_groupSize = m_cfg->max_group_num * sizeof(var_u8);
		if (CreateFile(file_group_status, &bm_groupBuf, bm_groupFile, bm_groupSize, sizeof(var_u8)) < 0)
		{
			e_log->writelog("%s:%d Create file %s error.\n", FL, LN, file_group_status);
			return -26;
		}
		bm_groupStatus = (var_u8*)bm_groupBuf;

		if (loadBinData == 1)
		{
			LoadBinData(pMemory, 0);
		}
	}
	if (ReloadIncData(pMemory, 0) < 0)
	{
		e_log->writelog("%s:%d Reload inc data error.\n", FL, LN);
		return -27;
	}
	var_4 dataNum, groupNum;
	GetStatus(dataNum, groupNum, 0);
	p_log->writelog("%s:%d Now data num is %d and group num is %d.\n", FL, LN, dataNum, groupNum);
	p_log->writelog("%s:%d Index service started successfully!\n", FL, LN);
	return 0;
}

var_4 Gather::ReloadIncData(var_1* pMemory, var_4 ith)
{
	DIR *dirp;
	struct dirent *direntp;
	var_1 filename[MAX_FILE_PATH];
	DOC_BUF *pDoc = NULL;
	var_1   *pAddBuf = NULL;

	// check if inc data exists
	var_4 iRet, optType, delType, bufLen;
	var_u8 deleteid = 0, fileSize = 0;
	if (!(dirp = opendir(m_cfg->recv_path)))
	{
		p_log->writelog("%s:%d Open dir %s error.\n", FL, LN, m_cfg->send_path);
		return -1;
	}
	// alloc buffers
	pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
	pAddBuf = (var_1*)(pMemory + ith * ALLOC_SIZE);

	while (direntp = readdir(dirp))
	{
		if (direntp->d_type != 8)
		{
			continue;
		}
		if (!(strncmp(direntp->d_name, "recv.inc", MAX_FILE_PATH)))
		{
			SPRINT(filename, MAX_FILE_PATH, "%s%s%s", m_cfg->recv_path, SLASH, "recv.inc");
			FILE* fp = fopen(filename, "rb");
			if (fp == NULL)
			{
				e_log->writelog("%s:%d Open file %s error.\n", FL, LN, filename);
				closedir(dirp);
				FreeObj(pDoc);
				return -1;
			}
			// check data header
			var_1 dataHeader[16] = {0};
			iRet = fread(dataHeader, 4, 1, fp);
			if (iRet != 1 || strncmp(dataHeader, DATA_HEADER, 4) != 0)
			{
				e_log->writelog("%s:%d Receive file header error.\n", FL, LN);
				GotoNextRecord(fp);
			}
			fileSize = GetFileSize(fp);
			while (fileSize != ftell(fp))
			{
				iRet = fread(&optType, sizeof(var_4), 1, fp);
				if (iRet != 1)
				{
					e_log->writelog("%s:%d Option type is %d error.\n", FL, LN, optType);
					GotoNextRecord(fp);
					continue;
				}
				if (optType == add_command)
				{
					iRet = fread(&bufLen, sizeof(var_4), 1, fp);
					if (iRet != 1 || bufLen < 0 || (bufLen + 8) > ALLOC_SIZE)
					{
						e_log->writelog("%s:%d Doc length is %d error.\n", FL, LN, bufLen);
						GotoNextRecord(fp);
						continue;
					}
					// read doc body buffer
					iRet = fread(pAddBuf, (bufLen+3)&0xFFFFFFFC, 1, fp);
					if (iRet != 1)
					{
						e_log->writelog("%s:%d Read doc body error.\n", FL, LN);
						GotoNextRecord(fp);
						continue;
					}
					iRet = ProcessBufToDoc(pAddBuf, bufLen, pDoc);
					if (iRet < 0)
					{
						e_log->writelog("%s:%d[ERRNO:%d] Process body error.\n", FL, LN, iRet);
						GotoNextRecord(fp);
						continue;
					}
					if (m_cfg->noduplicate == 1)
					{
						iRet = AddDoc_noduplicate(pDoc, pDoc->force, pMemory, ith + 1);
					}
					else
					{
						iRet = AddDoc(pDoc, pDoc->force, pMemory, ith + 1);
					}
					if (iRet < 0)
					{
						e_log->writelog("%s:%d Add doc %lu error.Errno is %d.\n", FL, LN, pDoc->docid, iRet);
						GotoNextRecord(fp);
						continue;
					}
				}
				else if(optType == del_command)
				{
					iRet = fread(&delType, 4, 1, fp);
					if (iRet != 1)
					{
						e_log->writelog("%s:%d Invalid delete type %d.\n", FL, LN, delType);
						GotoNextRecord(fp);
						continue;
					}
					iRet = fread(&deleteid, 8, 1, fp);
					if (iRet != 1)
					{
						e_log->writelog("%s:%d Invalid id %lu. DelType is %d.\n", FL, LN, deleteid, delType);
						GotoNextRecord(fp);
						continue;
					}
					DelDoc(deleteid, delType);
				}
				else if (optType ==update_command)
				{
				
				}
				else
				{
					e_log->writelog("%s:%d Command(%d) is unrecognizable.\n", FL, LN, optType);
				}
				// check the header of next record
				iRet = fread(dataHeader, 4, 1, fp);
				if (iRet < 0 || strncmp(dataHeader, DATA_HEADER, 4) != 0)
				{
					e_log->writelog("%s:%d Read data header error.\n", FL, LN);
					GotoNextRecord(fp);
					continue;
				}
			}
			CloseFile(fp);
			// find recv.inc, so break here
			break;
		}
	}
	closedir(dirp);
	FreeObj(pDoc);
	return 0;
}

var_vd Gather::FreeFiles()
{
	// do some clean job here
	return;
}

var_4 Gather::AddDoc(DOC_BUF *pDoc, var_4 forceRefresh, var_1* pMemory, var_4 ith)
{
	GROUP_INFO *pGroupInfo = NULL;
	DATA_INFO  *pDataInfo  = NULL;
	DOC_BUF *poldDoc = NULL; 
	DOC_BUF *pdstDoc = NULL;
	DOC_INFO docu;
	uLong zlibBufLen, dstBufLen;
	var_1 *dstBuf = NULL;
	var_1 *srcBuf = NULL;
	var_u8 docid, finger;
	var_u4 time = 0, groupid = 0, dataid = 0;
	var_4  iRet = 0, datBufLen = 0, srcBufLen;

    if (pDoc == NULL)
	{
		return -1;
	}
	docid  = pDoc->docid;
	finger = pDoc->finger;
	time   = pDoc->time;
	if (docid == 0 || finger == 0 || time == 0)
	{
		e_log->writelog("%s:%d Discard data, docid is %lu,finger is %lu,time is %u.\n ", docid, finger, time);
		return -2;
	}
    var_1 *zlibBuf = (var_1*)(pMemory + ith * ALLOC_SIZE);
    var_1 *datBuf  = (var_1*)(pMemory + (ith + 1) * ALLOC_SIZE);

	iRet = m_pDocInfo->Search(docid, docu);
	if (iRet <= 0)
	{
		// compress doc to binary
		zlibBufLen = ALLOC_SIZE;
		iRet = compress((Bytef*)zlibBuf, &zlibBufLen, (const Bytef*)pDoc->buff, (uLong)pDoc->buflen);
		if (iRet != Z_OK)
		{
			e_log->writelog("%s:%d Compress data error,docid is %lu,finger is %lu.\n", FL, LN, docid, finger);
			return -5;
		}
		iRet = m_data_store->add(docid, zlibBuf, zlibBufLen);
		if (iRet < 0)
		{
			e_log->writelog("%s:%d Save docid  %lu error.\n", FL, LN, docid);
			return -6;
		}

		// set rwlock
		update_data_lock.lock();
		// alloc data
		iRet = m_pDataAlloc->getID(dataid);
		if (iRet <= 0)
		{
			update_data_lock.unlock();
			e_log->writelog("%s:%d Alloc dataid error,docid is %lu.\n", FL, LN, docid);
			return -7;
		}
		// set store data
		pDataInfo = m_pDataBase + dataid;
		pDataInfo->docid = docid;
		pDataInfo->time = time;
		pDataInfo->next = 0;
		// get doc weight
		pDataInfo->weight = GetDocWeight(pDoc);
	
		// alloc group
		iRet = m_pGroupAlloc->getID(groupid);
		if (iRet <= 0)
		{
			update_data_lock.unlock();
			e_log->writelog("%s:%d Alloc groupid error,docid is %lu.\n", FL, LN, docid);
			return -8;
		}
		// set store group
		pGroupInfo = m_pGroupBase + groupid;
		pGroupInfo->leadid = docid;
		pGroupInfo->finger = finger;
		pGroupInfo->number = 1;
		pGroupInfo->time = time;
		pGroupInfo->next = dataid;
		// get leader weight
		pGroupInfo->weight = pDataInfo->weight;
		// set maps
		docu.groupid = groupid;
		docu.dataid  = dataid;
		m_pDocInfo->Add(docid, &docu);
		m_pFingerGroup->Add(finger, &groupid);
		// modify	
		bm_modify[groupid] = 1;
		// release lock
		update_data_lock.unlock();
	}
	else if (iRet == 1)
	{
		if (forceRefresh == 1)
		{
			// replace doc
			DelDoc(docid, 0);
			AddDoc(pDoc, 1, pMemory, ith); // Certainly there is no original doc, DelDoc beforehand
		}
		else
		{
			// merge tags
			srcBuf = (var_1*)(pMemory + (ith + 2) * ALLOC_SIZE);
			dstBuf = (var_1*)(pMemory + (ith + 3) * ALLOC_SIZE);
			poldDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
			pdstDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
			srcBufLen = ALLOC_SIZE;
			dstBufLen = ALLOC_SIZE;
			iRet = m_data_store->query(docid, srcBuf, srcBufLen);
			if (iRet < 0)
			{
				e_log->writelog("%s:%d m_data_store query docid %lu error.\n", FL, LN, docid);
				FreeObj(poldDoc);
				FreeObj(pdstDoc);
				return -9;
			}
			iRet = uncompress((Bytef *)dstBuf, &dstBufLen, (const Bytef *)srcBuf, srcBufLen); 
			if (iRet != Z_OK)
			{
				e_log->writelog("%s:%d Merge docid %lu error.\n", FL, LN, docid);
				FreeObj(poldDoc);
				FreeObj(pdstDoc);
				return -10;
			}
			iRet = ProcessBufToDoc(dstBuf, dstBufLen, poldDoc);
			if (iRet < 0)
			{
				e_log->writelog("%s:%d[ERRNO:%d] process buffer to doc(%lu) error.\n", FL, LN, iRet, docid);
				FreeObj(poldDoc);
				FreeObj(pdstDoc);
				return -11;
			}
			iRet = MergeDoc(poldDoc, pDoc, pdstDoc, pMemory, ith + 4);
			FreeObj(poldDoc);
			if (iRet <0)
			{
				e_log->writelog("%s:%d[ERRNO:%d] merge doc error.\n", FL, LN, iRet);
				FreeObj(pdstDoc);
				return -12;
			}
			// compress doc to binary
			zlibBufLen = ALLOC_SIZE;
			iRet = compress((Bytef*)zlibBuf, &zlibBufLen, (const Bytef*)pdstDoc->buff, (uLong)pdstDoc->buflen);
			FreeObj(pdstDoc);
			if (iRet != Z_OK)
			{
				e_log->writelog("%s:%d Compress data error,docid is %lu,finger is %lu.\n", FL, LN, docid, finger);
				return -13;
			}// save doc
			iRet = m_data_store->add(docid, zlibBuf, zlibBufLen);
			if (iRet < 0)
			{
				e_log->writelog("%s:%d Save doc error,docid is %lu.\n", FL, LN, docid);
				return -14;
			}
			update_data_lock.lock();
			bm_modify[docu.groupid] = 1;
			update_data_lock.unlock();
		}
	}
	e_log->writelog("%s:%d Add docid %lu finished.\n", FL, LN, docid);
	return 0;
}

var_4 Gather::AddDoc_noduplicate(DOC_BUF *pDoc, var_4 forceRefresh, var_1* pMemory, var_4 ith)
{
	GROUP_INFO *pGroupInfo = NULL;
	DATA_INFO  *pDataInfo  = NULL;
	DOC_BUF *poldDoc = NULL;
	DOC_BUF *pdstDoc = NULL;
	uLong zlibBufLen, dstBufLen;
	DOC_INFO docu;
	var_1 *dstBuf = NULL;
	var_1 *srcBuf = NULL;
	var_u8 docid, finger, time;
	var_u4 groupid, dataid;
	var_4 iRet, iRet1, iRet2, datBufLen =0, srcBufLen;
	if (pDoc == NULL)
	{
	    return -1;
	}

	docid  = pDoc->docid;
	finger = pDoc->finger;
	time   = pDoc->time;

	if (time == 0 || docid == 0 || finger == 0)
	{
		e_log->writelog("%s:%d Discard data docid is %lu,finger is %lu.\n", FL, LN, docid, finger);
		return -2;
	}
	var_1* datBuf  = (var_1*)(pMemory + ith * ALLOC_SIZE);
	var_1* zlibBuf = (var_1*)(pMemory + (ith + 1) * ALLOC_SIZE);
	
	// check docid and finger
	iRet1 = m_pDocInfo->Search(docid, docu); 			// map: docid->DOC_INFO
	iRet2 = m_pFingerGroup->Search(finger, groupid); 	// map: finger->groupid
	// no such docid
	if (iRet1 <= 0)
	{
		update_data_lock.lock();
		zlibBufLen = ALLOC_SIZE;
		// compress doc to binary
		iRet = compress((Bytef*)zlibBuf, &zlibBufLen, (const Bytef*)pDoc->buff, (uLong)pDoc->buflen);
		if (iRet != Z_OK)
		{
			update_data_lock.unlock();
			e_log->writelog("%s:%d Compress data error,docid is %lu,finger is %lu.\n", FL, LN, docid, finger);
			return -5;
		}
		// save doc
		iRet = m_data_store->add(docid, zlibBuf, zlibBufLen);
		if (iRet < 0)
		{
			update_data_lock.unlock();
			e_log->writelog("%s:%d Save doc error,docid is %lu.\n", FL, LN, docid);
			return -6;
		}
		// alloc data
		iRet = m_pDataAlloc->getID(dataid);
		if (iRet <= 0)
		{
			update_data_lock.unlock();
			e_log->writelog("%s:%d Alloc dataid error,docid is %lu,finger is %lu.\n", FL, LN, docid, finger);
			return -7;
		}
		// set store data
		pDataInfo = m_pDataBase + dataid;
		pDataInfo->docid = docid;
		pDataInfo->time  = time;
		pDataInfo->next  = 0;
		// get doc weight
		pDataInfo->weight = GetDocWeight(pDoc);

		// finger not exists, add a new group
		if (iRet2 <= 0)
		{
			// alloc group
			iRet = m_pGroupAlloc->getID(groupid);
			if (iRet <= 0)
			{
				update_data_lock.unlock();
				e_log->writelog("%s:%d Alloc groupid error,docid is %lu,finger is %lu.\n", FL, LN, docid, finger);
				return -8;
			}
			// set store group
			pGroupInfo = m_pGroupBase + groupid;
			pGroupInfo->leadid = docid;
			pGroupInfo->finger = finger;
			pGroupInfo->number = 1;
			// get leader weight
			pGroupInfo->weight = pDataInfo->weight;
			pGroupInfo->time   = time;
			pGroupInfo->next   = dataid;
			m_pFingerGroup->Add(finger, &groupid);
		}
		// finger exists, add doc to corresponding group
		else
		{
			// attach to group
			AttachDataToGroup(groupid, dataid);
		}
		// set maps
		docu.groupid = groupid;
		docu.dataid  = dataid;
		m_pDocInfo->Add(docid, &docu);

		bm_modify[groupid] = 1;

		update_data_lock.unlock();
	}
	// docid exists
	else
	{
		// force refresh or finger not exists
		if (forceRefresh == 1 || iRet2 <= 0 || (iRet2 > 0 && docu.groupid != groupid))
		{
			DelDoc(docid, 0);
			AddDoc_noduplicate(pDoc, forceRefresh, pMemory, ith);
		}
		else
		{
			//merge tags
			srcBuf = (var_1*)(pMemory + (ith + 2) * ALLOC_SIZE);
			dstBuf = (var_1*)(pMemory + (ith + 3) * ALLOC_SIZE);
			poldDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
			pdstDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
			srcBufLen = ALLOC_SIZE;
			dstBufLen = ALLOC_SIZE;
			iRet = m_data_store->query(docid, srcBuf, srcBufLen);
			if (iRet < 0)
			{
				FreeObj(poldDoc);
				FreeObj(pdstDoc);
				e_log->writelog("%s:%d m_data_store query docid %lu error.\n", FL, LN, docid);
				return -9;
			}
			iRet = uncompress((Bytef *)dstBuf, &dstBufLen, (const Bytef *)srcBuf, srcBufLen); 
			if (iRet != Z_OK)
			{
				FreeObj(poldDoc);
				FreeObj(pdstDoc);
				e_log->writelog("%s:%d Merge docid %lu error.\n", FL, LN, docid);
				return -10;
			}
			iRet = ProcessBufToDoc(dstBuf, dstBufLen, poldDoc);
			if (iRet < 0)
			{
				FreeObj(poldDoc);
				FreeObj(pdstDoc);
				e_log->writelog("%s:%d[ERRNO:%d] process buffer to doc(%lu) error.\n", FL, LN, iRet, docid);
				return -11;
			}
			iRet = MergeDoc(poldDoc, pDoc, pdstDoc, pMemory, ith + 4);
			FreeObj(poldDoc);
			if (iRet <0)
			{
				e_log->writelog("%s:%d[ERRNO:%d] merge doc error.\n", FL, LN, iRet);
				FreeObj(pdstDoc);
				return -12;
			}
			// compress doc to binary
			zlibBufLen = ALLOC_SIZE;
			iRet = compress((Bytef*)zlibBuf, &zlibBufLen, (const Bytef*)pdstDoc->buff, (uLong)pdstDoc->buflen);
			FreeObj(pdstDoc);
			if (iRet != Z_OK)
			{
				e_log->writelog("%s:%d Compress data error,docid is %lu,finger is %lu.\n", FL, LN, docid, finger);
				return -13;
			}// save doc
			iRet = m_data_store->add(docid, zlibBuf, zlibBufLen);
			if (iRet < 0)
			{
				e_log->writelog("%s:%d Save doc error,docid is %lu.\n", FL, LN, docid);
				return -14;
			}
			update_data_lock.lock();
			bm_modify[docu.groupid] = 1;
			update_data_lock.unlock();
		}
	}
	e_log->writelog("%s:%d Add docid %lu finished.\n", FL, LN, docid);
	return 0;
}

var_4 Gather::AttachDataToGroup(var_u4 groupid, var_u4 dataid)
{
	GROUP_INFO *pGroupInfo = m_pGroupBase + groupid;
	DATA_INFO  *pDataInfo  = m_pDataBase  + dataid;
	// 判断是否要换leader
	ChooseLeader(pDataInfo, pGroupInfo);
	
	pDataInfo->next = pGroupInfo->next;
	pGroupInfo->next = dataid;
	pGroupInfo->number++;
	return 0;
}
#ifdef OLD_DATA

var_vd Gather::SendErrorPack(SOCKET soClient, var_4 iErr)
{
	var_1 szPack[20] = {0};
	snprintf(szPack, 12, "%s%d", "RecvError", -1);
	CSocketPack::SendDataEx(soClient, szPack, 11);
	CSocketPack::CloseSocket(soClient);
}

#else
var_vd Gather::SendErrorPack(SOCKET soClient, var_4 iErr)
{
	var_1 szPack[20] = {0};
    strncpy(szPack, CHECK_HEADER, 8);
    *(var_4*)(szPack + 8) = iErr;
	CSocketPack::SendDataEx(soClient, szPack, 12);
	CSocketPack::CloseSocket(soClient);
}
#endif
//get thread status for monitor system
var_4 Gather::GetThreadState(var_4 iTimeOut)
{
    var_1 *lpszBuf = m_lpszThreadErr, *lpszBufEnd = m_lpszThreadErr + m_iThreadMax * 20;
    var_4 iErr = 0, iRet = 0, iErrLen = 0;
    for(var_4 i = 0; i < m_iThreadMax; i++)
    {
        iRet = m_spThreadInfo[i].sState.GetStateInfo(iTimeOut, lpszBuf, lpszBufEnd-lpszBuf, iErrLen);
        if(iRet <= 0)
            continue;
        if(iErrLen > 0)
            lpszBuf += iErrLen;
    }
    return iErr;
}

// support two kinds of query conditions, through docid or finger
// give priority to docid
var_4 Gather::GetSimilarDocs_test(var_u8 searchid, var_4 searchType, var_4 startPos, var_4 endPos, var_1* outBuf, var_4 outBufLen, var_4 &totalSimilarNum, var_4 &resultSimilarNum)
{
	var_1 *pMemory = NULL;
	var_1 *tmpBuf = NULL;
	uLong tmpBufLen;
	var_4 processedLen = 0, iRet, i = 0, j = 0, cnt = 0; // for counting
	var_u4 groupid, dataid;
	var_u8 getDocIds[64] = {0}; // the max number is 64
	DOC_INFO docu;
	DOC_BUF *pDoc = NULL;
	if ( startPos < 0 || endPos < startPos || endPos - startPos >= 64)
	{
		e_log->writelog("%s:%d Invalid in parameters.\n", FL, LN);
		return -1;
	}
	if (searchType != 0 && searchType != 1)
	{
		e_log->writelog("%s:%d DelDoc searchType invalid.\n", FL, LN);
		return -10;
	}
	pMemory = (var_1*)calloc(1, ALLOC_SIZE << 2);
	if (pMemory == NULL)
	{
		e_log->writelog("%s:%d Allocate memory error.\n", FL, LN);
		return -2;
	}
	// 按docid查询
	if (searchType == 0)
	{
		iRet = m_pDocInfo->Search(searchid, docu);
		if (iRet <= 0)
		{
			e_log->writelog("%s:%d Docid %lu not found.\n", FL, LN, searchid);
			return -3;
		}
		groupid = docu.groupid;
	}
	// 按groupid查询
	if (searchType == 1)
	{
		iRet = m_pFingerGroup->Search(searchid, groupid);
		if (iRet <= 0)
		{
			e_log->writelog("%s:%d Finger %lu not found.\n",FL, LN, searchid);
			return -4;
		}
	}

	GROUP_INFO *groupInfo;
	DATA_INFO *dataInfo;
	groupInfo = m_pGroupBase + groupid;
	totalSimilarNum = groupInfo->number;
	if (totalSimilarNum < startPos + 1)
	{
		return -5;
	}
	dataid = groupInfo->next;
	while (dataid && cnt < startPos)
	{
		dataInfo = m_pDataBase + dataid;
		dataid = dataInfo->next;
		cnt++;
	}
	// get next (endPos - startPos) ids
	while (dataid && cnt <= endPos)
	{
		dataInfo = m_pDataBase + dataid;
		// remove itself
		if (searchid != dataInfo->docid)
		{
			getDocIds[i++] = dataInfo->docid;
			cnt++;
		}
		dataid = dataInfo->next;
	}
	//get doc data
	if (i > 0)
	{
		pDoc = new DOC_BUF();
		tmpBuf = (var_1*)pMemory;
		if (pDoc == NULL)
		{
			FreeObj(pDoc);
			return -6;
		}
		resultSimilarNum = 0;
		for (j = 0; j < i; ++j)
		{
			// get the ith doc
			tmpBufLen = ALLOC_SIZE;
			iRet = GetDocData(getDocIds[j], tmpBuf, &tmpBufLen, pMemory, 2);
			if (iRet < 0)
			{
			    e_log->writelog("%s:%d Get data of docid %lu error.\n", FL, LN, getDocIds[j]);
			    continue;
			}
            iRet = ProcessBufToDoc(tmpBuf, tmpBufLen, pDoc);
            if (iRet < 0)
            {
                e_log->writelog("%s:%d Process buffer to docid %lu error.\n", FL, LN, getDocIds[j]);
                continue;
            }
            iRet = GenerateAddXml(pDoc, totalSimilarNum, outBuf + processedLen + 4, outBufLen - processedLen - 4);
            if (iRet <= 0)
            {
				e_log->writelog("%s:%d GenerateAddXml[docid %lu, errno %d].\n", FL, LN, getDocIds[j], iRet);
                continue;
            }
            // copy data length
            memcpy(outBuf + processedLen, &iRet, 4); // iRet: generated xml length
            processedLen += iRet + 4;
			resultSimilarNum++;
		}
		// free memory
		FreeObj(pDoc);
	}
	return processedLen;
}

var_4 Gather::GetSameNews(var_u8 docid, var_4 requestType, var_4 startPos, var_4 endPos, var_1* dstBuf, var_4 dstBufLen, var_4& totalNum, var_4& txtLen, var_1* pMemory, var_4 ith)
{
	var_4 iRet, i=0, j=0, processLen=0, isSelf=0;
	var_u4 groupid, dataid;
	var_1 *tmpBuf = NULL;
	var_8 docIDs[1024]={0};
	DOC_INFO docu;
	DOC_BUF *pDoc = NULL;
	DATA_INFO *pDataInfo = NULL;
	GROUP_INFO *pGroupInfo = NULL;
	uLong tmpBufLen = ALLOC_SIZE;
	// remember set totalNum 0
	totalNum = 0;
	
	iRet = m_pDocInfo->Search(docid, docu);
	if (iRet <= 0)
	{
		e_log->writelog("%s:%d Doc[%lu] not found.\n", FL, LN, docid);
		return -1;
	}
	groupid = docu.groupid;
	update_data_lock.lock();
	pGroupInfo = m_pGroupBase + groupid;
	// startPos start with 0
	if (pGroupInfo->number <= startPos)
	{
		update_data_lock.unlock();
		e_log->writelog("%s:%d StartPos[%d] larger than number[%d].\n", FL, LN, startPos, pGroupInfo->number);
		return -2;
	}
	dataid = pGroupInfo->next;
	// go through docs before startPos
	while (dataid && i < startPos)
	{
		pDataInfo = m_pDataBase + dataid;
		dataid = pDataInfo->next;
		i++;
	}
	// get docids between startPos and endPos
	while (dataid && i<pGroupInfo->number && j<(endPos- startPos))
	{
		pDataInfo = m_pDataBase + dataid;
		docIDs[j] = pDataInfo->docid;
		dataid = pDataInfo->next;
		i++;
		j++;
	}
	// unlock
	update_data_lock.unlock();

	if (j >0)
	{
		pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
		tmpBuf = (var_1*)(pMemory + ith*ALLOC_SIZE);
		if (pDoc == NULL)
		{
			FreeObj(pDoc);
			return -3;
		}
		for (i =0, isSelf=0; i<j; ++i)
		{
			iRet = GetDocData(docIDs[i], tmpBuf, &tmpBufLen, pMemory, ith + 1);
			if (iRet < 0)
			{
				e_log->writelog("%s:%d Get doc[%lu] data error.\n", FL, LN, docIDs[i]);
				FreeObj(pDoc);
				continue;
			}
			iRet = ProcessBufToDoc(tmpBuf, tmpBufLen, pDoc);
			if (iRet < 0)
			{
				e_log->writelog("%s:%d Process buffer to doc[%lu] error.\n", FL, LN, docIDs[i]);
				FreeObj(pDoc);
				continue;
			}
			if (docid == docIDs[i])
			{
				isSelf = 1;
				iRet = DigestXml(pDoc, dstBuf + processLen, dstBufLen, requestType, 1);
			}
			else
			{
				iRet = DigestXml(pDoc, dstBuf + processLen, dstBufLen, requestType, 0);
			}
			if (iRet > 0)
			{
				totalNum++;
				processLen += iRet;
			}
			else
			{
				e_log->writelog("%s:%d Get doc[%lu] digest xml error.\n",FL, LN, docIDs[i] );
				continue;
			}
		}
		if (isSelf >0)
		{
			iRet = GetDocData(docid, tmpBuf, &tmpBufLen, pMemory, ith + 2);
			if (iRet >= 0)
			{
				iRet = ProcessBufToDoc(tmpBuf, tmpBufLen, pDoc);
				if (iRet >= 0)
				{
					iRet =  DigestXml(pDoc, dstBuf + processLen, dstBufLen, requestType, 2);
					if (iRet >= 0)
					{
						txtLen = iRet - 1; // 减掉末尾的0x07
						processLen += iRet;
					}
					else
					{
						e_log->writelog("%s:%d Get doc[%lu] digest xml error.\n", FL, LN, docid);
					}
				}
				else
				{
					e_log->writelog("%s:%d Process buffer to doc[%lu] error.\n", FL, LN, docid);
				}
			}
			else
			{
				e_log->writelog("%s:%d Get doc[%lu] data error.\n", FL, LN, docid);
			}
		}
	}
	return processLen;
}

var_4 Gather::GetDocData(var_u8 docid, var_1* dstBuf, uLong *dstBufLen, var_1* pMemory, var_4 ith)
{
	var_4 iRet;
	var_1* srcBuf = (var_1*)(pMemory + ith *ALLOC_SIZE);
	var_4 srcBufLen = ALLOC_SIZE;
	iRet = m_data_store->query(docid, srcBuf, srcBufLen);
	if (iRet < 0)
	{
		e_log->writelog("%s:%d m_data_store query error.\n", FL, LN);
		return -2;
	}
	// dstBufLen为传入传出参数，先传入为ALLOC_SIZE，传出值为buffer实际长度
	*dstBufLen = ALLOC_SIZE;
	iRet = uncompress((Bytef *)dstBuf, dstBufLen, (const Bytef *)srcBuf, srcBufLen);
	if (iRet == Z_OK)
	{
		return 0;
	}
	return -3;
}
var_vd Gather::ChooseLeader(DATA_INFO* pDataInfo, GROUP_INFO* pGroupInfo)
{
    if ((pDataInfo->weight + 2) / 3 > (pGroupInfo->weight + 2) / 3 ||
			((pDataInfo->weight + 2) / 3 == (pGroupInfo->weight + 2) / 3 && pDataInfo->time < pGroupInfo->time))
	{
		pGroupInfo->leadid = pDataInfo->docid;
		pGroupInfo->weight = pDataInfo->weight;
		pGroupInfo->time = pDataInfo->time;
	}
    return;
}

var_4 Gather::DelOldData(var_u4 overTime)
{
	// record status of data id alloc
	var_u4 total;		// alloc memory size
	var_u4 current;		// cycle using memory, like circular queue, current point
	var_u4 alloced;		// represent the number of memory block used
	var_u4 maxid;		// the max number of memory available
	var_4 delNum = 0;

	m_pDataAlloc->stat(total, current, alloced, maxid);
	for (var_4 i = 0; i <= maxid; ++i)
	{
		if (m_pDataAlloc->is_set(i))
		{
			if (m_pDataBase[i].docid != 0 && m_pDataBase[i].time < overTime)
			{
				DelDoc(m_pDataBase[i].docid, 0);
				delNum++;
			}
		}
	}
	m_pDataAlloc->stat(total, current, alloced, maxid);
	p_log->writelog("%s:%d Delete over time docs %d\n", FL, LN, delNum);
	return 0;
}

var_4 Gather::GetDocXml(var_u8 docid, var_1* outBuf, var_4 outBufLen, var_1 *pMemory, var_4 ith)
{
	var_u4 groupid;
	var_4 iRet, sameDocsNum; // the number of documents that have the same finger
    uLong tmpBufLen = ALLOC_SIZE;
	var_1 *tmpBuf = NULL;
	DOC_INFO docu;
	GROUP_INFO *pGroupInfo = NULL;
	DOC_BUF *pDoc = NULL;
	if (outBuf == NULL || outBufLen < ALLOC_SIZE)
	{
		e_log->writelog("%s:%d Bad parameters.\n", FL, LN);
		return -1;
	}
	iRet = m_pDocInfo->Search(docid, docu);
	if (iRet <= 0)
	{
		e_log->writelog("%s:%d No docid %lu searched.\n", FL, LN, docid);
		return -2;
	}
	pGroupInfo = m_pGroupBase + docu.groupid;
	sameDocsNum = pGroupInfo->number; 
	tmpBuf = (var_1*)(pMemory + ith * ALLOC_SIZE);
	pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
	if (pDoc == NULL)
	{
		FreeObj(pDoc);
		e_log->writelog("%s:%d Alloc buffer error.\n", FL, LN);
		return -3;
	}
	iRet = GetDocData(docid, tmpBuf, &tmpBufLen, pMemory, ith + 1);
	if (iRet < 0)
	{
		FreeObj(pDoc);
	    e_log->writelog("%s:%d GetDocData error.docid %lu.\n", FL, LN, docid);
	    return -4;
	}
    iRet = ProcessBufToDoc(tmpBuf, tmpBufLen, pDoc);
    if (iRet < 0)
    {
		FreeObj(pDoc);
        e_log->writelog("%s:%d ProcessBufToDoc error.docid = %lu.\n", FL, LN, docid);
        return -5;
    }
    iRet = GenerateAddXml(pDoc, sameDocsNum, outBuf, outBufLen);
    if (iRet <= 0)
	{
		FreeObj(pDoc);
		e_log->writelog("%s:%d GenerateAddXml[docid %lu, errno %d].\n", FL, LN, docid, iRet);
        return -6;
    }
	FreeObj(pDoc);
	return iRet;
}

var_4 Gather::DumpBinData(var_1* pMemory, var_4 ith)
{
	var_u4 total, current, alloced, maxid;
	var_4 i, iRet, fileid=0, addlen=0, addnum=0;
	uLong doclen = ALLOC_SIZE;
	var_u8 docid = 0;
	var_1 fname[MAX_FILE_PATH] = {0};
	var_1 *pDocBuf = NULL;
	FILE* fp = NULL;
	DOC_INFO docu;
	DATA_INFO *pDataInfo = NULL;
	pDocBuf = (var_1*)(pMemory + ith * ALLOC_SIZE);
	
	SPRINT(fname, MAX_FILE_PATH, "%s%sDump_%03d.bin", m_cfg->dump_path, SLASH, fileid);
	fp = fopen(fname, "wb");
	if (fp == NULL)
	{
		e_log->writelog("%s%d Open file %s error.\n", FL, LN, fname);
		return -3;
	}
	// lock
	m_pDataAlloc->stat(total, current, alloced, maxid);
	p_log->writelog("%s:%d Start dump bin data...\n", FL, LN);
	p_log->writelog("%s:%d Total data number is %d.\n", FL, LN, m_pDocInfo->Count());

	for (i = 1; i <= maxid; i++)
	{
		if (m_pDataAlloc->is_set(i))
		{
			pDataInfo = m_pDataBase + i;
			docid = pDataInfo->docid;
			iRet = m_pDocInfo->Search(docid, docu);
			if (iRet <= 0)
			{
				continue;
			}
			iRet = GetDocData(docid, pDocBuf, &doclen, pMemory, ith + 1);
			if (iRet < 0)
			{
				e_log->writelog("%s:%d GetDocData error. docid is %lu.\n", FL, LN, docid);
				continue;
			}
			if (fwrite(&doclen, sizeof(uLong), 1, fp) == 1 && fwrite(pDocBuf, doclen, 1, fp) == 1 )
			{
				 addlen += doclen + sizeof(uLong);
				 addnum++;
			}
			if (addlen >= (1<<30))
			{
				CloseFile(fp);
				fileid +=1;
				addlen = 0;
				SPRINT(fname, MAX_FILE_PATH, "%s%sDump_%03d.bin", m_cfg->dump_path, SLASH, fileid);
				fp = fopen(fname, "wb");
				if (fp == NULL)
				{
					e_log->writelog("%s:%d Open file %s error.\n", FL, LN, fname);
					return -4;
				}
			}
			if (addnum % 100000 == 0)
			{
				p_log->writelog("%s:%d Dump data number is %d.\n", FL, LN, addnum);
			}
		}
	}
	CloseFile(fp);
	update_data_lock.unlock();
	p_log->writelog("%s:%d Dump data number is %d.\n", FL, LN, addnum);
	p_log->writelog("%s:%d Dump bin data finished.\n", FL, LN);
	return 0;
}

var_4 Gather::LoadBinData(var_1* pMemory, var_4 ith)
{
	DIR *dirp;
	struct dirent *direntp;
	FILE *fp = NULL;
	DOC_BUF* pDoc  = NULL;
	var_1 *pDocBuf = NULL;
	var_1 fname[MAX_FILE_PATH] = {0};
	uLong doclen   = ALLOC_SIZE;
	var_4 iRet, curnum=0;
	var_8 fileSize = 0;
	pDocBuf = (var_1*)(pMemory + ith * ALLOC_SIZE);
	pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
	if (pDoc == NULL)
	{
		FreeObj(pDoc);
		p_log->writelog("%s:%d Allocate memory error.\n", FL, LN);
		return -1;
	}
	if (!(dirp = opendir(m_cfg->dump_path)))
	{
		p_log->writelog("%s:%d Open dir %s error.\n", m_cfg->dump_path);
		FreeObj(pDoc);
		return -2;
	}
	while (direntp = readdir(dirp))
	{
		if (direntp->d_type == 8)
		{
			SPRINT(fname, MAX_FILE_PATH, "%s%s%s", m_cfg->dump_path, SLASH, direntp->d_name);
			fp = fopen(fname, "rb");
			if (fp == NULL)
			{
				p_log->writelog("%s:%d Open file %s error.\n", FL, LN, fname);
				continue;
			}
			fileSize = GetFileSize(fp);
			while (fileSize != ftell(fp))
			{
				iRet = fread(&doclen, sizeof(uLong), 1, fp);
				if (iRet == 1 && doclen < ALLOC_SIZE)
				{
					iRet = fread(pDocBuf, doclen, 1, fp);
					if (iRet != 1)
					{
						p_log->writelog("%s:%d Read doc[%lu] error.\n", FL, LN, *(var_8*)pDocBuf);
						continue;
					}
					iRet = ProcessBufToDoc(pDocBuf, doclen, pDoc);
					if (iRet < 0)
					{
						e_log->writelog("%s:%d[ERRNO:%d] Process buf to doc[%lu] error.\n", FL, LN, iRet, *(var_8*)pDocBuf);
						continue;
					}
					if (m_cfg->noduplicate == 1)
					{
						AddDoc_noduplicate(pDoc, 0, pMemory, ith + 1);
					}
					else
					{
						AddDoc(pDoc, 0, pMemory, ith + 1);
					}
					curnum++;
					if (curnum % 100000 == 0)
					{
						p_log->writelog("%s:%d Load bin data number is %d.Filename is %s.\n", FL, LN, curnum, fname);
					}
				}
				else
				{
					break;
				}
			}
			CloseFile(fp);
		}
	}
	p_log->writelog("%s:%d Load bin data number is %d.\n", FL, LN, curnum);
	FreeObj(pDoc);
	closedir(dirp);
	p_log->writelog("%s:%d LoadBinData finished.\n", FL, LN);
	return 0;
}

var_4 Gather::SendDataToIndex(var_1 *buf, var_4 bufLen, var_1 *ip, var_4 port)
{
	var_4 iRet, sendLen;
	var_1 recvbuf[20] = {0};
	SOCKET soClient;
	iRet = CSocketPack::Connect(ip, port, 30000, soClient);
	if (iRet != 0)
	{
		return -1;
	}

	iRet = CSocketPack::SendDataEx(soClient, buf, bufLen);
	if (iRet != 0)
	{
		CSocketPack::CloseSocket(soClient);
		return -2;
	}
	sendLen = *(var_4*)(buf + 12);
	iRet = CSocketPack::RecvDataEx(soClient, recvbuf, 16);
	if (iRet != 0)
	{
		CSocketPack::CloseSocket(soClient);
		return -3;
	}
	CSocketPack::CloseSocket(soClient);
	
	if (sendLen != *(var_4*)(recvbuf + 12))
	{
		return -4;
	}	
	return 0;
}

var_4 Gather::GetSendFiles(var_1 *folder, var_1* ident, var_1 *file_add, var_1 *file_del)
{
	var_4 iRet, oldIndex = INT_MAX, index, find = 0;
	DIR *dirp;
	struct dirent *direntp;
	var_4 len;

	if(!(dirp = opendir(folder)))
	{
		return -1;
	}
	while((direntp = readdir(dirp)))
	{
		if (direntp->d_type == 8)
		{
			len = strlen(direntp->d_name);
			if(len < 10 || strncmp(direntp->d_name + len - 4,".dat",4))
			{	
				continue;
			}
			if(strncmp(direntp->d_name, ident, strlen(ident)))
			{
				continue;
			}
			index = atoi(direntp->d_name + 7);
			if (index < oldIndex)
			{
				oldIndex = index;
				find = 1;
			}
		}
	}
	closedir(dirp);
	
	if (find == 1)
	{
		SPRINT(file_add, MAX_FILE_PATH, "%s%sAddxml_%04d.dat", folder, SLASH, oldIndex);
		SPRINT(file_del, MAX_FILE_PATH, "%s%sDelxml_%04d.dat", folder, SLASH, oldIndex);
		return 0;
	}
	else
	{
		return -4;
	}
}

var_4 Gather::GetRelativeNews(var_u8 docid, var_4 requestType, var_1* outXml, var_4 outXmlLen, var_4& totalNum, var_4& txtLen, var_1* pMemory, var_4 ith)
{
	var_1* tmpBuf = NULL;
	DOC_BUF* pDoc = NULL;
	uLong tmpBufLen = ALLOC_SIZE;
	var_4 processLen, iRet;

	tmpBuf = (var_1*)(pMemory + ith*ALLOC_SIZE);
	pDoc = (DOC_BUF*)calloc(sizeof(DOC_BUF), 1);
	
	if (pDoc == NULL)
	{
		FreeObj(pDoc);
		return -1;
	}

	iRet = GetDocData(docid, tmpBuf, &tmpBufLen, pMemory, ith + 1);
	if (iRet >= 0)
	{
		iRet = ProcessBufToDoc(tmpBuf, tmpBufLen, pDoc);
		if (iRet >= 0)
		{
			iRet = GetRelatives(outXml, outXmlLen, pDoc, totalNum);
			if (iRet >= 0)
			{
				processLen = iRet;
				iRet = DigestXml(pDoc, outXml + processLen, outXmlLen, requestType, 2);
				if (iRet >0)
				{
					txtLen = iRet - 1; // 减掉末尾的0x07
					processLen += iRet;
				}
				else
				{
					e_log->writelog("%s:%d[ERRNO:%d] Get doc[%lu] digest xml error.\n", FL, LN, iRet, docid);
				}
			}
			else
			{
				e_log->writelog("%s:%d[ERRNO:%d] Doc[%lu] getRelatives error.\n", FL, LN, iRet, docid);
			}
		}
		else
		{
			e_log->writelog("%s:%d[ERRNO:%d] Process buffer to doc[%lu] error.\n", FL, LN, iRet, docid);
		}
	}
	else
	{
		e_log->writelog("%s:%d[ERRNO:%d] Get doc[%lu] data error.\n", FL, LN, iRet, docid);
	}
	FreeObj(pDoc);
	return processLen;
}


var_vd Gather::MergeSmallFiles(var_1 *data_path, var_1 *pMemory)
{
	Fixsizehash<var_u8, DOC_POS> *addDocs = NULL;
	Fixsizehash<var_u8, var_1>  *delDocs = NULL;
	DIR *dirp;
	struct dirent *direntp;
	struct stat st;
	FILE* fp = NULL;
	FILE** fpAdds = NULL;
	FILE** fpDels = NULL;
	var_4 idx=0, iLen = 0, i, j;
	var_4 iRet = 0, dataLen, ithAdd = 0, ithDel  =0;
	var_4 end_flag = 0, hashCount = 0;
	var_8 offSet, fileSize, writeSize = 0;
	var_1 unused = 1;
	var_u8 docid = 0;
	var_1 **filenm = NULL;
	var_1 addFileName[MAX_FILE_PATH] = {0};
	var_1 delFileName[MAX_FILE_PATH] = {0};
	var_1 fileAddBak[MAX_FILE_PATH]  = {0};
	var_1 fileDelBak[MAX_FILE_PATH]  = {0};
	var_1 oldFileName[MAX_FILE_PATH] = {0};
	var_1 newFileName[MAX_FILE_PATH] = {0};
	var_1 sDocID[32] = {0};
	var_1 *docBuf  = (var_1*)(pMemory);
	
	if (!(dirp = opendir(data_path)))
	{
		e_log->writelog("%s:%d Open dir %s error.\n", FL, LN, data_path);
		return;
	}
	filenm = new var_1*[10000];
	while ((direntp = readdir(dirp)))
	{
		if (direntp->d_type == 8)
		{
			iLen = strlen(direntp->d_name);
			if (iLen < 10)
			{
				continue;
			}
			if (strncmp(direntp->d_name, "Add", 3))
			{
				continue;
			}
			if (strncmp(direntp->d_name + iLen - 4, ".dat", 4))
			{
				continue;
			}
			SPRINT(addFileName, MAX_FILE_PATH, "%s%s%s", data_path, SLASH, direntp->d_name);
			if (GetFileSize(addFileName) >= (1<<27))
			{
				continue;
			}
			SPRINT(delFileName, MAX_FILE_PATH, "%s%s%s%s", data_path, SLASH, "Del",  direntp->d_name + 3);
			if (GetFileSize(delFileName) >= (1<<27))
			{
				continue;
			}
			filenm[idx] = new var_1[MAX_FILE_PATH];
			strncpy(filenm[idx], direntp->d_name, iLen);
			// only store file name, to save time in the following bubble sort
			idx++;
		}
	}
	closedir(dirp);
	if (idx < m_cfg->small_file_num)
	{
		return;
	}
	var_1 tmp[MAX_FILE_PATH] = {0};
	
	fpAdds = (FILE**)calloc(idx, sizeof(FILE*));
	fpDels = (FILE**)calloc(idx, sizeof(FILE*));
		
	if (fpAdds == NULL || fpDels == NULL)
	{
		e_log->writelog("%s:%d Open file error.\n", FL, LN);
		return;
	}

	addDocs = new Fixsizehash<var_u8, DOC_POS>(m_cfg->max_data_num, m_cfg->max_data_num>>1);
	delDocs = new Fixsizehash<var_u8, var_1>(m_cfg->max_data_num, m_cfg->max_data_num>>1);
	if (addDocs == NULL || delDocs == NULL)
	{
		end_flag = 1;
		e_log->writelog("%s:%d Fixsizehash error.\n", FL, LN);
	}

	if (addDocs->Init(e_log) <= 0 || delDocs->Init(e_log) <= 0)
	{
		end_flag = 1;
		e_log->writelog("%s:%d Fixsizehash init error.\n", FL, LN);
	}
	
	assert(end_flag != 1);

	for (i=0; i<idx; i++)
	{
		for (j = 0; j<idx-i-1; j++)
		{
			if (strncmp(filenm[j], filenm[j+1], MAX_FILE_PATH) > 0)
			{
				strncpy(tmp, filenm[j], MAX_FILE_PATH);
				strncpy(filenm[j], filenm[j+1], MAX_FILE_PATH);
				strncpy(filenm[j+1],tmp, MAX_FILE_PATH);
			}
		}
	}
	DOC_POS pos;
	for (i = 0; i < idx; i++)
	{
		SPRINT(addFileName, MAX_FILE_PATH, "%s%s%s", data_path, SLASH, filenm[i]);
		SPRINT(delFileName, MAX_FILE_PATH, "%s%s%s%s", data_path, SLASH, "Del", filenm[i] + 3);
		fpDels[i] = fopen(delFileName, "rb");
		if (fpDels[i] == NULL)
		{
			end_flag = 1;
			e_log->writelog("%s:%d Open file %s error.\n", FL, LN, delFileName);
			break;
		}
		fileSize = GetFileSize(delFileName);
		while (fileSize != ftell(fpDels[i]))
		{
			fgets(sDocID, 32, fpDels[i]);
			docid = strtoul(sDocID, 0, 10);
			iRet = addDocs->Search(docid, pos);
			if (iRet > 0)
			{
				addDocs->Del(docid);
			}
			else
			{
				delDocs->Add(docid, &unused);
			}
		}
		if (end_flag == 1)
		{
			break;
		}
		fpAdds[i] = fopen(addFileName, "rb");
		if (fpAdds[i] == NULL)
		{
			end_flag = 1;
			e_log->writelog("%s:%d Open file %s error.\n", FL, LN, addFileName);
			break;
		}
		fileSize = GetFileSize(addFileName);
		while (fileSize != ftell(fpAdds[i]))
		{
			iRet = fread(&dataLen, 4, 1, fpAdds[i]);
			if (iRet <= 0 || dataLen < 0 || dataLen > ALLOC_SIZE)
			{
				end_flag =1;
				break;
			}
			offSet = ftell(fpAdds[i]);
			iRet = fread(docBuf, dataLen, 1, fpAdds[i]);
			if (iRet <= 0)
			{
				end_flag = 1;
				break;
			}
			pos.fp = fpAdds[i];									// file handle
			pos.offSet = offSet;								// offset
			pos.dataLen = dataLen;								// data length
			GetTagValue(docBuf, dataLen, "doc_id", sDocID, 1);
			docid = strtoul(sDocID, 0, 10);
			addDocs->Add(docid, &pos);
		}
		if (end_flag == 1)
		{
			break;
		}
	}
	for (i = 0;i<idx; i++)
	{
		CloseFile(fpDels[i]);
	}
	FreeObj(fpDels);
	if (end_flag == 1)
	{
		for (i=0; i<idx; i++)
		{
			CloseFile(fpAdds[i]);
		}
		FreeObj(fpAdds);
		DeleteObj(addDocs);
		DeleteObj(delDocs);
		return;
	}

	SPRINT(fileAddBak, MAX_FILE_PATH, "%s%s%s.bak", data_path, SLASH, filenm[ithAdd]);
	fp = fopen(fileAddBak, "wb");
	
	addDocs->GotoHead();
	hashCount = addDocs->Count();
	while (addDocs->GetNext(&docid, &pos) && hashCount)
	{
		offSet = pos.offSet;
		dataLen= pos.dataLen;
		if (writeSize > (1<<27))
		{
			ithAdd += 1;
			CloseFile(fp);
			SPRINT(fileAddBak, MAX_FILE_PATH, "%s%s%s.bak", data_path, SLASH, filenm[ithAdd]);
			fp = fopen(fileAddBak, "wb");
			if (fp == NULL)
			{
				end_flag = 1;
				e_log->writelog("%s:%d Open file %s error.\n", FL, LN, fileAddBak);
				break;
			}
			writeSize = 0;
		}
		fseek(pos.fp, offSet, SEEK_SET);
		iRet = fread(docBuf, dataLen, 1, pos.fp);
		if (iRet != 1)
		{
			end_flag = 1;
			e_log->writelog("%s:%d Read error.\n", FL, LN);
			break;
		}
		iRet = fwrite(&dataLen, 4, 1, fp);
		if (iRet != 1)
		{
			end_flag = 1;
			e_log->writelog("%s:%d Write error.\n", FL, LN);
			break;
		}
		iRet = fwrite(docBuf, dataLen, 1, fp);
		if (iRet != 1)
		{
			end_flag = 1;
			e_log->writelog("%s:%d Write error.\n", FL, LN);
			break;
		}
		writeSize += dataLen + 8;
		hashCount--;
	}
	CloseFile(fp);
	for (i =0; i<idx;i++)
	{
		CloseFile(fpAdds[i]);
	}
	FreeObj(fpAdds);
	DeleteObj(addDocs);
	if (end_flag == 1)
	{
		DeleteObj(delDocs);
		return;
	}
	
	SPRINT(fileDelBak, MAX_FILE_PATH, "%s%s%s%s.bak",  data_path, SLASH, "Del", filenm[ithDel] + 3);
	writeSize = 0;	
	delDocs->GotoHead();
	hashCount = delDocs->Count();
	fp = fopen(fileDelBak, "wb");
	while (delDocs->GetNext(&docid, &unused)&&hashCount)
	{
		iRet = fwrite(&docid, 8, 1, fp);
		if (iRet != 1)
		{
			DeleteObj(delDocs);
			CloseFile(fp);
			e_log->writelog("%s:%d Write error.\n", FL, LN);
			return;
		}
		if (writeSize > (1<<27))
		{
			CloseFile(fp);
			ithDel++;
			SPRINT(fileDelBak, MAX_FILE_PATH, "%s%s%s%s.bak", data_path, SLASH, "Del", filenm[ithDel] + 3);
			fp = fopen(fileDelBak, "wb");
			if (fp == NULL)
			{
				DeleteObj(delDocs);
				CloseFile(fp);
				e_log->writelog("%s:%d Open file %s error.\n", FL, LN, fileDelBak);
				return;
			}
			writeSize = 0;
		}
		writeSize += 8;
		hashCount--;
	}

	DeleteObj(delDocs);
	CloseFile(fp);
	while (ithAdd > ithDel)
	{
		ithDel++;
		SPRINT(fileDelBak, MAX_FILE_PATH, "%s%s%s%s.bak", data_path, SLASH, "Del", filenm[ithDel] + 3);
		fp = fopen(fileDelBak, "wb");
		CloseFile(fp);
	}

	while (ithDel > ithAdd)
	{
		ithAdd++;
		SPRINT(fileAddBak, MAX_FILE_PATH, "%s%s%s", data_path, SLASH, filenm[ithAdd]);
		fp = fopen(fileAddBak, "wb");
		CloseFile(fp);
	}
	for (i = 0; i < idx; i++)
	{
		SPRINT(oldFileName, MAX_FILE_PATH, "%s%s%s", data_path, SLASH, filenm[i]);
		RemoveFile(oldFileName);	
		SPRINT(oldFileName, MAX_FILE_PATH, "%s%s%s%s", data_path, SLASH, "Del", filenm[i] + 3);
		RemoveFile(oldFileName);
	}
	for (i = 0; i <= ithAdd;i++)
	{
		SPRINT(oldFileName, MAX_FILE_PATH, "%s%s%s%s.bak", data_path, SLASH, "Del", filenm[i] + 3);
		SPRINT(newFileName, MAX_FILE_PATH, "%s%s%s%s", data_path, SLASH, "Del", filenm[i] + 3);
		RenameFile(oldFileName, newFileName);

		SPRINT(oldFileName, MAX_FILE_PATH, "%s%s%s.bak", data_path, SLASH,  filenm[i]);
		SPRINT(newFileName, MAX_FILE_PATH, "%s%s%s", data_path, SLASH, filenm[i]);
		RenameFile(oldFileName, newFileName);
	}
	delete[] filenm;
	return;
}

var_vd Gather::GotoNextRecord(FILE *fp)
{
	var_4 readBytes;
	var_1 readBuf[8] = {0};
	var_8 fileSize = 0;
	fileSize = GetFileSize(fp);
	while (fileSize != ftell(fp))
	{
		readBytes = fread(readBuf, 4, 1, fp);
		if (readBytes == 1 && memcmp(readBuf, DATA_HEADER, 4) == 0)
		{
			break;
		}
	}
	return;
}
// flag 0 仅返回系统容量信息；1 打印堆信息和发送给索引的数据量
var_vd Gather::GetStatus(var_4& dataNum, var_4& groupNum, var_4 flag)
{
	var_u4 total, current, alloced, maxid;
	m_pGroupAlloc->stat(total, current, alloced, maxid);
	groupNum = alloced;
	dataNum = m_pDocInfo->Count();
	return;	
}



