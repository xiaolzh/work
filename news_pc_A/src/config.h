// read config file
// max return -46
#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "libs/UH_Define.h"
#include "libs/UC_ReadConfigFile.h"

#define MAX_FILE_PATH 256
#define MAX_INDEX_GROUP 8
#define MAX_INDEX_IN_EACH_GROUP 32
#define MAX_IP_LEN 32


typedef struct _config_file_
{
	//config members
	var_1 log_path[MAX_FILE_PATH];
	var_1 data_path[MAX_FILE_PATH];
	var_1 backup_path[MAX_FILE_PATH];
	var_1 dump_path[MAX_FILE_PATH];
	var_1 recv_path[MAX_FILE_PATH];
	var_1 send_path[MAX_FILE_PATH];
	var_1 store_backup_path[MAX_FILE_PATH];
	var_1 whitelist[MAX_FILE_PATH];

	var_1 file_add[MAX_FILE_PATH];
	var_1 file_add_xml_pre[MAX_FILE_PATH];
	var_1 file_del_xml_pre[MAX_FILE_PATH];
	var_1 index_ip[MAX_INDEX_GROUP][MAX_INDEX_IN_EACH_GROUP][MAX_IP_LEN];
	var_1 file_idx[MAX_FILE_PATH];
	var_1 file_data[MAX_FILE_PATH];
	var_1 file_idx_alloc[MAX_FILE_PATH];
	var_1 file_data_alloc[MAX_FILE_PATH];
	var_1 file_doc_map[MAX_FILE_PATH];
	var_1 file_finger_map[MAX_FILE_PATH];
	var_1 file_group_status[MAX_FILE_PATH];
	var_1 file_save_flag[MAX_FILE_PATH];
	var_1 data_store_config_file[MAX_FILE_PATH];
	var_4 recv_port;
	var_4 comm_port;
	var_4 query_port;
	var_4 del_port;
	var_4 watch_port;
	var_4 recv_overtime;
	var_4 send_overtime;
	var_4 watch_overtime;
	var_4 max_group_num;
    var_4 max_data_num;
	var_4 small_file_num;

	var_4 thread_num_recv;
	var_4 thread_num_comm;
	var_4 thread_num_query; 

	var_4 update_interval; 			// use seconds as unit
	var_4 save_interval; 			// use minutes as unit
	var_4 backup_count; 			// backup when save x counts	

	var_4 noduplicate;				// whether remove the duplicates
	var_4 index_group_num;
	var_4 index_num[MAX_INDEX_GROUP];
	var_4 index_port[MAX_INDEX_GROUP][MAX_INDEX_IN_EACH_GROUP];

	var_4 init(const var_1 *cfg_file)
	{
		if (cfg.InitConfigFile(cfg_file) != 0)
		{
			printf("init config file failure\n");
			return -1;
		}

		var_4 i = 0, j = 0;
		var_1 tmp_buf[256];

		if (cfg.GetFieldValue("RECV_PORT", recv_port))
			return -2;
		if (cfg.GetFieldValue("COMM_PORT", comm_port))
			return -3;
		if (cfg.GetFieldValue("WATCH_PORT", watch_port))
			return -4;
		if (cfg.GetFieldValue("QUERY_PORT", query_port))
			return -46;
		if (cfg.GetFieldValue("DEL_PORT", del_port))
			return -5;
		if (cfg.GetFieldValue("RECV_OVERTIME", recv_overtime))
			return -6;
		if (cfg.GetFieldValue("SEND_OVERTIME", send_overtime))
			return -7;
		if (cfg.GetFieldValue("WATCH_OVERTIME",	watch_overtime))
			return -8;
		if (cfg.GetFieldValue("MAX_GROUP_NUM", max_group_num))
			return -9;
		if (cfg.GetFieldValue("MAX_DATA_NUM", max_data_num))                                                                                       
			return -10;
		if (cfg.GetFieldValue("SMALL_FILE_NUM", small_file_num))
			return -43;
		if (cfg.GetFieldValue("THREAD_NUM_RECV", thread_num_recv))
			return -12;
		if (cfg.GetFieldValue("THREAD_NUM_QUERY", thread_num_query))
			return -45;
		if (cfg.GetFieldValue("THREAD_NUM_COMM", thread_num_comm))
			return -13;
		if (cfg.GetFieldValue("DATA_STORE_CONFIG_FILE", data_store_config_file))
			return -15;
		if (cfg.GetFieldValue("LOG_PATH", log_path))
			return -16;
		if (cfg.GetFieldValue("RECV_PATH", recv_path))
			return -17;
		if (cfg.GetFieldValue("SEND_PATH", send_path))
			return -18;
		if (cfg.GetFieldValue("DATA_PATH", data_path))
			return -19;
		if (cfg.GetFieldValue("DUMP_PATH", dump_path))
			return -40;
		if (cfg.GetFieldValue("BACKUP_PATH", backup_path))
			return -20;
		if (cfg.GetFieldValue("STORE_BACKUP_PATH", store_backup_path))	
			return -42;
		if (cfg.GetFieldValue("UPDATE_INTERVAL", update_interval))
			return -21;
		if (cfg.GetFieldValue("SAVE_INTERVAL", save_interval))
			return -22;
		if (cfg.GetFieldValue("BACKUP_COUNT", backup_count))
			return -23;
		if (cfg.GetFieldValue("INDEX_GROUP_NUM", index_group_num))
			return -24;
		if (cfg.GetFieldValue("NODULPLICATE", noduplicate))
			return -25;

		if (cfg.GetFieldValue("FILE_IDX", file_idx))
			return -26;
		if (cfg.GetFieldValue("FILE_DATA", file_data))
			return -27;
		if (cfg.GetFieldValue("FILE_IDX_ALLOC", file_idx_alloc))
			return -28;
		if (cfg.GetFieldValue("FILE_DATA_ALLOC", file_data_alloc))
			return -29;
		if (cfg.GetFieldValue("FILE_DOC_MAP", file_doc_map))
			return -30;
		if (cfg.GetFieldValue("FILE_FINGER_MAP", file_finger_map))
			return -31;
		if (cfg.GetFieldValue("FILE_GROUP_STATUS", file_group_status))
			return -39;
		if (cfg.GetFieldValue("FILE_SAVE_FLAG", file_save_flag))
			return -41;

		for (i = 0; i < index_group_num; i++)
		{
			sprintf(tmp_buf, "INDEX_NUM_%.2d", i);
			if(cfg.GetFieldValue(tmp_buf, index_num[i]))
				return -34;
			for (j = 0; j < index_num[i]; j++)
			{
				sprintf(tmp_buf, "INDEX_IP_%.2d_%.2d", i, j);
				if(cfg.GetFieldValue(tmp_buf, index_ip[i][j]))
					return -35;
				sprintf(tmp_buf, "INDEX_PORT_%.2d_%.2d", i, j);
				if(cfg.GetFieldValue(tmp_buf, index_port[i][j]))
					return -36;
			}
		}
		return 0;
	}

	template <class Type>
	var_4 GetFieldValue(const var_1* fieldName, Type& fieldValue)
	{
		return cfg.GetFieldValue(fieldName, fieldValue);
	}

private:
	UC_ReadConfigFile cfg;

}CONFIG_INFO;

#endif // __CONFIG_H__

