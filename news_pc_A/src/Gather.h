#ifndef _GATHER_H_
#define _GATHER_H_

#include "libs/UH_Define.h"
#include "libs/Utility.h"
#include "libs/Fixsizehash.h"
#include "libs/socketpack.h"
#include "libs/stateinfo.h"
#include "storage/UT_Persistent_KeyValue.h"
#include "protocol.h"
#include "libs/zlib.h"
#include "config.h"

//group infomation in memory
typedef struct _group_info_
{
	var_u8 leadid; 		// the representative docid in this group
	var_u8 finger;		//
	var_u8 weight;
	var_u4 time;		//
	var_u4 next;		// address in DATA_INFO, docids in this list  belong to this group
	var_4 number;		// how much docids in this group
}GROUP_INFO;

//data information in memory
typedef struct _data_info_
{
	var_u8 docid;
	var_u8 weight;
	var_u4 time;
	var_u4 next;
}DATA_INFO;

//doc info in hash
typedef struct _doc_info_
{
	var_u4 groupid;
	var_u4 dataid;
}DOC_INFO;

typedef struct _doc_pos
{
	var_4 dataLen;
	var_8 offSet;
	FILE* fp;
}DOC_POS;

class Gather
{
	public:
		Gather(void);
		~Gather(void);
		var_4 Init(CONFIG_INFO *cfg, var_4 loadBinData = 0);
		var_4 GetThreadState(var_4 timeout);
	private:
		var_4 AddDoc(DOC_BUF*, var_4, var_1*, var_4);
		var_4 AddDoc_noduplicate(DOC_BUF*, var_4, var_1*, var_4);
		var_4 DelDoc(var_u8, var_4);
		var_4 AttachDataToGroup(var_u4, var_u4);
		//delete old data by time
		var_4 DelOldData(var_u4 overTime);
		//get similar data
		var_4 GetSimilarDocs(var_u8, var_4, var_4, var_4, var_1*, var_4, var_4 &, var_4 &);
		//////////////

		var_4 Save();
		var_4 Backup();
		//recover data or init new data, need check whether data file exist first
		var_4 Recover(var_1*, var_4);
		//copy all current data for other index, copy from backup files
		var_4 CopyAllData();
		//push all current data for one group index server
		var_4 PushAllData();

		var_vd FreeFiles();
		var_vd GetStatus(var_4&, var_4&, var_4);
		var_4 ReloadIncData(var_1*, var_4);
		var_4 GetDocData(var_u8, var_1*, uLong*, var_1*, var_4);
		var_4 GetDocXml(var_u8 docid, var_1* outBuf, var_4 outBufLen, var_1*, var_4);
		var_4 DumpBinData(var_1*, var_4);
		var_4 LoadBinData(var_1*, var_4);
		var_4 GetSendFiles(var_1 *folder, var_1* ident, var_1 *file_add, var_1 *file_del);
		var_4 SendDataToIndex(var_1 *buf, var_4 bufLen, var_1 *ip, var_4 port);
		var_4 GetSameNews(var_u8, var_4, var_4, var_4, var_1*, var_4, var_4&, var_4&,var_1*, var_4);
		var_4 GetRelativeNews(var_u8, var_4, var_1*, var_4, var_4&, var_4&, var_1*, var_4);
		var_4 GetSimilarDocs_test(var_u8, var_4, var_4, var_4, var_1*, var_4, var_4 &, var_4 &);	
		var_vd ChooseLeader(DATA_INFO *pDataInfo, GROUP_INFO *pGroupInfo);
		// merge small files to a big one
		var_vd MergeSmallFiles(var_1* data_path, var_1* pMemory);
		// go to process the next record in date file
		var_vd GotoNextRecord(FILE* fp);
		// Communication thread, accept commands and deal the related work
		static THREADFUNTYPE ThreadComm(var_vd* vpInfo);
		// query thread
		static THREADFUNTYPE ThreadQuery(var_vd* vpInfo);
		// recv data thread
		static THREADFUNTYPE ThreadRecvData(var_vd* vpInfo);
		// delete data thread
		static THREADFUNTYPE ThreadDelData(var_vd* vpInfo);
		// add data thread
		static THREADFUNTYPE ThreadAddData(var_vd* vpInfo);
		// generate xml data for pushing to index
		static THREADFUNTYPE ThreadGenPushData(var_vd* vpInfo);
		// pushing data to index
		static THREADFUNTYPE ThreadPushData(var_vd* vpInfo);
		// pushing all data to index
		static THREADFUNTYPE ThreadPushAllData(var_vd* vpInfo);
		// save and backup thread
		static THREADFUNTYPE ThreadSave(var_vd* vpInfo);

		//other functions
		var_vd SendErrorPack(SOCKET soClient, var_4 iErr);
        bool		m_bExit; 					// exit flag
		var_1		m_push_flag[MAX_INDEX_GROUP];
        var_1		*bm_modify; 	            
        var_1		*m_lpszThreadErr; 			// record thread errors
		var_4		m_iThreadMax; 				// all thread numbers

		var_4 		FileSerial[MAX_INDEX_GROUP][MAX_INDEX_IN_EACH_GROUP];
		var_4		bm_groupFile;
		var_8		bm_groupSize;
		var_1		*bm_groupBuf;				
		var_u8		*bm_groupStatus; 			// check whether groupid exist before, if yes, send del and add to index, otherwise, just send add
	
		Log_file 	*e_log;						// 错误日志
		Log_file	*p_log;						// 流程日志
        DATA_INFO	*m_pDataBase; 			    // data id chain
		var_1		*m_pDataBuf;				//
        GROUP_INFO	*m_pGroupBase; 				// group id chain
		var_1		*m_pIdxBuf;					//
        CONFIG_INFO	*m_cfg;

		IDAlloc		*m_pGroupAlloc; 			// group id alloc
		IDAlloc		*m_pDataAlloc; 				// data id alloc
		SOCKET		m_addServer; 				// for add data
		SOCKET		m_commServer; 				// for search data
		SOCKET		m_delServer;				// for delete data
		SOCKET		m_queryServer;				// for query data
		THREAD_INFO	*m_spThreadInfo; 		    // for thread monitor

		CP_MUTEXLOCK                    recv_data_lock;        // mutex lock, for receiving data, writing to recv.dat, used in recv data thread and add data thread
        CP_MUTEXLOCK                    update_data_lock;      // mutex lock, for adding data, generate pushing xml, used in add data thread and generate push data thread
        CP_MUTEXLOCK                 	save_data_lock;        // mutex lock, for save data thread
        Fixsizehash<var_u8, DOC_INFO>   *m_pDocInfo;           // docid to doc info
        Fixsizehash<var_u8, var_u4>     *m_pFingerGroup;       // finger to groupid
		//need add data storage, mutex_lock, rw_lock ... below
        UT_Persistent_KeyValue<var_u8>  *m_data_store;					// this is used for rw lock
};
#endif
