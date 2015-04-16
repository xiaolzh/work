#ifndef __SCRIPT_SERVICE_H__
#define __SCRIPT_SERVICE_H__
#include <queue>
#include <string>
#include <iostream>
#include <fstream>
using namespace std;

#include "UC_MD5.h"
#include "WFLog.h"
#include "socketpack.h"
#include "Utility.h"
#include "UH_Define.h"
#include "common_def.h"
#include "publicfunction.h"
#include "service_config.h"
#include "UC_Allocator_Recycle.h"
#include "json/include/json/json.h"

#define	CHECKSTR		"SCRIPTSR"
#define	ERROR_CODE		(-2000)
#define	MAX_BUFFER_SIZE	(10240)
#define	MAX_EMAIL_NUM	(5)
#define	MAX_FILE_PATH	(256)

//#define __zxl_DEBUG__
using namespace nsWFLog;

struct task
{
	CP_SOCKET_T sockfd;
	var_4 		error_code;
	string 		json_value;
	task()
	{
		sockfd =0;
		error_code=0;
		json_value="";
	}
};

struct mail_task
{
	int email_num;
	string data_path;
	string emails[MAX_EMAIL_NUM];
	struct task t;
};

struct thread_param
{
	void* p1;
	void* p2;
};

struct request
{
	int email_num;
	string script;
	string params;
	string emails[MAX_EMAIL_NUM];
	request()
	{
		email_num = 0;
	}
};

struct response
{
	int execution;
	int timestamp;
	int send_size;
	int data_size;
	string ip;
	string script;
	string params;
	string value;
};

bool static equal(struct request lt, struct request rt)
{
	if (lt.email_num != rt.email_num ||
		lt.script.compare(rt.script) ||
		lt.params.compare(rt.params))
	{
		return false;
	}
	for (size_t i = 0; i < lt.email_num; i++)
	{
		if (lt.emails[i].compare(rt.emails[i]))
		{
			return false;
		}
	}
	return true;
}

class script_service
{
public:
	script_service();
	~script_service();
	var_4 init(const var_1* cfg);

	static void* thread_request(void* param);
	static void* thread_process(void* param);
	static void* thread_work(void* param);
	static void* thread_reply(void* param);
	static void* thread_clear(void* param);
	static void* thread_mail(void* param);

	inline var_4 send_error_code(const CP_SOCKET_T _sock, const var_4 code)
	{
		var_1 buffer[17];
		strncpy(buffer, CHECKSTR, 8);
		sprintf(buffer + 8, "%-8d", code);
		var_4 ret = cp_sendbuf(_sock, buffer, 16);
		cp_close_socket(_sock);
		return (ret? errno:0);
	}
	var_4 json_read(string json_value, struct request& req);
	var_4 json_write(struct mail_task mt, string& json_value);
	var_4 json_write(struct response res, string& json_value);
	var_4 parse_request(CP_SOCKET_T sockfd, struct request req, struct response& res, string& data_path);

	service_config* m_serv_config;
	static UC_MD5	m_md5;	
private:

	var_4 			m_run_status;
	CP_SOCKET_T		m_reqsvr_socket;
	CDailyLog*		m_error_logger;
	CDailyLog*		m_task_logger;
	UC_Allocator_Recycle*	m_large_allocator;
	
	/*
	 * 1. 接收线程 接收请求放到m_recv_queue队列中
	 * 2. 处理线程 从m_recv_queue中取出任务处理完毕
	 *             后放入发送队列m_send_queue
	 * 3. 发送线程 从m_send_queue中取出任务发送
	 * 4. 邮件线程 如果socket中断，或文件过大发送失败
	 * 			   将任务放入邮件队列中发送
	*/
	queue<task> m_recv_queue;
	queue<mail_task> m_send_queue;
	queue<mail_task> m_mail_queue;
};

#endif
