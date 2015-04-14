#ifndef __SERVICE_CONFIG_H__
#define __SERVICE_CONFIG_H__

#include "UH_Define.h"
#include "UC_ReadConfigFile.h"

class service_config
{
public:
	var_u4	req_out_time;
	var_u2	moniter_port;
	var_u2	request_port;

	service_config()
		: req_out_time(0)
	{}

	~service_config()
	{
		clear();
	}

	var_4 read_cfg(UC_ReadConfigFile& conf_reader)
	{
		clear();
		var_4 ret = conf_reader.GetFieldValue("req_out_time", req_out_time);
		if (ret)
			return -1;
		ret = conf_reader.GetFieldValue("moniter_port", moniter_port);
		if (ret)
			return -2;
		ret = conf_reader.GetFieldValue("request_port", request_port);
		if (ret)
			return -3;
	}
	var_vd clear()
	{
		req_out_time = 0;
		moniter_port = 0;
		request_port = 0;
	}
};

#endif
