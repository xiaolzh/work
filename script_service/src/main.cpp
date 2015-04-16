#include "script_service.h"

enum ErrorType
{
	TYPE_OK = 0,	//��������
	TYPE_NETWORK,   //�������
	TYPE_SERVICE,   //�������
	TYPE_OTHER,	 //��������
};
	
enum ErrorLevel
{
	LEVEL_A = 1,	//����
	LEVEL_B,		//��Ҫ
	LEVEL_C,		//һ��
	LEVEL_D,		//����
	LEVEL_E,		//�ɺ���
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
