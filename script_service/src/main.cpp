#include "script_service.h"

enum ErrorType
{
	TYPE_OK = 0,	//运行正常
	TYPE_NETWORK,   //网络故障
	TYPE_SERVICE,   //服务错误
	TYPE_OTHER,	 //其它错误
};
	
enum ErrorLevel
{
	LEVEL_A = 1,	//严重
	LEVEL_B,		//重要
	LEVEL_C,		//一般
	LEVEL_D,		//调试
	LEVEL_E,		//可忽略
};

int main(int argc, char* argv[])
{
	var_1* cfg_path = (var_1*)"./script_service.cfg";
	script_service service;
	var_4 ret = service.init(cfg_path);
	if (ret)
	{
		cout<< "service init failed";	
		return -1;
	}

	CP_SOCKET_T lis_sock;
	ret = cp_listen_socket(lis_sock, service.m_serv_config->moniter_port);
	if (ret)
	{
		cout<< "listen moniter port failed";
		return -2;
	}
	var_1 moniter_buffer[1024];
	while (1)
	{
		CP_SOCKET_T sock;
		ret = cp_accept_socket(lis_sock, sock);
		if (ret)
		{
			continue;
		}
		cp_set_overtime(sock, 5000);
		ret = cp_recvbuf(sock, moniter_buffer, 4);
		if (ret)
		{
			cp_close_socket(sock);
			continue;
		}
		var_1* pos = moniter_buffer;
		memcpy(pos, "MonitorP1", 10);
		pos += 10 + 4;
		*(var_4*)pos = TYPE_OK;
		pos += 4;
		*(var_4*)pos = LEVEL_B;
		pos += 4;

		*(var_4*)(moniter_buffer + 10) = pos - moniter_buffer - 14;
		ret = cp_sendbuf(sock, moniter_buffer, pos - moniter_buffer);
		cp_close_socket(sock);
		if (ret)
		{
			continue;
		}
	}
	return 0;
}
