#include "script_service.h"

UC_MD5 script_service::m_md5;

script_service::script_service()
{
	m_large_allocator = NULL;
	m_reqsvr_socket = -1;
	m_run_status = true;
}
script_service::~script_service()
{}

inline var_1* block_alloc(UC_Allocator_Recycle* mem_pool)
{
	var_1* temp = mem_pool->AllocMem();
	while (NULL == temp)
	{
		cp_sleep(10);
		temp = mem_pool->AllocMem();
	}
	return temp;
}



var_4 script_service::init(const var_1* cfg)
{
	var_4 ret;

	UC_ReadConfigFile conf_reader;
	ret = conf_reader.InitConfigFile(cfg);
	if (ret)
	{
		//
		return -5;
	}
	m_serv_config = new service_config;
	ret = m_serv_config->read_cfg(conf_reader);
	if (ret)
	{
		//
		return -6;
	}

	assert(NULL == m_large_allocator);
	m_large_allocator = new UC_Allocator_Recycle;
	if (NULL == m_large_allocator)
	{
		//
		return -7;
	}
	var_4 m_large_size = 1<<20;
	var_4 count = 10000;
	//
	ret = m_large_allocator->initMem(m_large_size, count, (var_4)sqrt(count));
	if (ret)
	{
		//
		return -8;
	}
	assert(-1 == m_reqsvr_socket);
	ret = cp_listen_socket(m_reqsvr_socket, m_serv_config->request_port);
	if (ret)
	{
		//
		return -9;
	}
	// 接收请求线程
	ret = cp_create_thread(thread_request, (void*)this);
	if (ret)
	{
		return -1;
	}
	ret = cp_create_thread(thread_process, (void*)this);
	if (ret)
	{
		return -10;
	}
	// 发送数据线程
	ret = cp_create_thread(thread_reply, (void*)this);
	if (ret)
	{
		return -2;
	}
	// 清理线程
	ret = cp_create_thread(thread_clear, (void*)this);
	if (ret)
	{
		return -3;
	}
	ret = cp_create_thread(thread_mail, (void*)this);
	if (ret)
	{
		return -4;
	}
	      
	return 0;
}

void* script_service::thread_request(void* param)
{
	script_service* _this = static_cast<script_service*>(param);
	
	var_1* buffer = block_alloc(_this->m_large_allocator);

	CP_SOCKET_T client_sock = -1;

	while (_this->m_run_status)
	{
		var_4 ret = cp_accept_socket(_this->m_reqsvr_socket, client_sock);
		if (ret)
		{
			continue;
		}
		cp_set_overtime(client_sock, _this->m_serv_config->req_out_time);

		ret = cp_recvbuf(client_sock, buffer, 16);
		if (ret)
		{
			_this->send_error_code(client_sock, -1);
			continue;
		}
		if (memcmp(buffer, CHECKSTR, 8))
		{
			_this->send_error_code(client_sock, -2);
			continue;
		}
		var_4 recv_len = atoi(buffer + 8);
		if (MAX_BUFFER_SIZE < recv_len)
		{
			_this->send_error_code(client_sock, -3);
			continue;
		}
		ret = cp_recvbuf(client_sock, buffer, recv_len);
		if (ret)
		{
			_this->send_error_code(client_sock, -4);
			continue;
		}
		/////////////////////////////////////////////////
		struct task t;
		t.sockfd = client_sock;
		t.json_value.assign(buffer, recv_len);
		_this->m_recv_queue.push(t);
	}
	_this->m_large_allocator->FreeMem((var_1*)buffer);
	return NULL;
}


void* script_service::thread_process(void* param)
{
	script_service* _this = static_cast<script_service*>(param);

	while (_this->m_run_status)
	{
		while (_this->m_recv_queue.empty())
		{
			cp_sleep(10);
		}
		struct task *t = new task;
		struct thread_param *pt = new thread_param;
		pt->p1 = param;
		pt->p2 = (void*)t;

		*t = _this->m_recv_queue.front();
		_this->m_recv_queue.pop();

		var_4 ret = cp_create_thread(thread_work, (void*)pt);
		if (ret)
		{
			//
		}
		cp_sleep(1);
	}
	return NULL;
}

var_4 script_service::json_read(string json_value, struct request& req)
{
#ifdef __zxl_DEBUG__
	cout <<json_value <<endl;
#endif
	Json::Reader reader;
	Json::Value  value;
	if (false == reader.parse(json_value, value))
	{
		//
		return -1;
	}
	else
	{
		req.script = value["script"].asString();
		req.params = value["params"].asString();
		const Json::Value array_objs = value["emails"];
		req.email_num =  (array_objs.size() > MAX_EMAIL_NUM?MAX_EMAIL_NUM:array_objs.size());
		for (size_t i = 0; i < req.email_num; i++)
		{
			 req.emails[i] = array_objs[i].asString();
		}
	}
	return 0;
}

var_4 script_service::json_write(struct response res, string data_path, string& json_value)
{
	Json::Value root;
	root["ip"] = res.ip;
	root["value"] = res.value;
	root["script"] = res.script;
	root["params"] = res.params;
	root["datasize"] = res.datasize;
	root["sendsize"] = res.sendsize;
	root["execution"] = res.execution;
	root["timestamp"] = res.timestamp;
	root.toStyledString();
	json_value = root.toStyledString();
#ifdef __zxl_DEBUG__
	cout <<json_value <<endl;
#endif
	return 0;
}

void* script_service::thread_work(void* param)
{
	struct thread_param *pt = static_cast<struct thread_param*>(param);
	struct task *t 			= static_cast<struct task*>(pt->p2);
	script_service* _this 	= static_cast<script_service*>(pt->p1);

	string data_path, json_value;
	struct request req;
	struct response res;
	var_4 ret = _this->json_read(t->json_value, req); 
	if (ret)
	{//

	}
	else
	{
		ret = _this->parse_request(t->sockfd, req, res, data_path);
		if (ret)
		{
			//
		}
		else
		{
			ret = _this->json_write(res, data_path, json_value);
			if (ret)
			{//
			}
			else
			{

			}
		}
	}
	struct mail_task mt;
	mt.t.error_code = ret;
	mt.t.sockfd = t->sockfd;	
	mt.t.json_value = json_value;
	mt.data_path = data_path;
	mt.email_num = req.email_num;
	for (size_t i = 0; i < req.email_num; i++)
	{
		mt.emails[i] = req.emails[i];
	}
	//处理完毕，加入发送队列
	_this->m_send_queue.push(mt);

	delete t;
	t = NULL;
	delete pt;
	pt = NULL;
	return NULL;
}


void* script_service::thread_reply(void* param)
{
	script_service* _this = static_cast<script_service*>(param);

	var_4 send_size;
	struct mail_task mt;
	var_1* buffer = block_alloc(_this->m_large_allocator);
	while (_this->m_run_status)
	{
		var_bl need_mail = false;
		while (_this->m_send_queue.empty())
		{
			cp_sleep(10);
		}
		mt = _this->m_send_queue.front();
		_this->m_send_queue.pop();

		strncpy(buffer, CHECKSTR, 8);
		if (mt.t.error_code)
		{// 处理线程返回错误
			snprintf(buffer + 8, 9, "%-8d", ERROR_CODE + mt.t.error_code);
			send_size = 16;
		}
		else
		{
			send_size = mt.t.json_value.length();
			if (MAX_BUFFER_SIZE < send_size)
			{
				need_mail = true;
				send_size = MAX_BUFFER_SIZE;
			}
			snprintf(buffer + 8, 9, "%-8d", send_size);
			snprintf(buffer + 16, send_size + 1, "%s", mt.t.json_value.c_str());
			send_size += 16;
		}
		var_4 ret = cp_sendbuf(mt.t.sockfd, buffer, send_size);
		if (ret)
		{
			//
			need_mail = true;
		}
		cp_close_socket(mt.t.sockfd);
		
		if (need_mail)
		{
			_this->m_mail_queue.push(mt);
		}
	}
	_this->m_large_allocator->FreeMem(buffer);
	return NULL;
}

void* script_service::thread_mail(void* param)
{
	script_service* _this = static_cast<script_service*>(param);

	struct mail_task mt;
	while (_this->m_run_status)
	{
		while (_this->m_mail_queue.empty())
		{
			cp_sleep(10);
		}
		mt = _this->m_mail_queue.front();
		_this->m_mail_queue.pop();

	}
	return NULL;
}

void* script_service::thread_clear(void* param)
{
	script_service* _this = static_cast<script_service*>(param);

	while (_this->m_run_status)
	{
		cp_sleep(1000);
	}
	return NULL;
}


var_4 script_service::parse_request(CP_SOCKET_T sockfd, struct request req,  struct response& res, string& data_path)
{
	var_u8 now, task_md5;
	var_1 foldname_new[MAX_FILE_PATH];
	var_1 foldname_old[MAX_FILE_PATH];
	var_1 filename_new[MAX_FILE_PATH];
	var_1 filename_old[MAX_FILE_PATH];
	var_1 md5file[MAX_FILE_PATH];
	
	int execution=0;// 脚本执行时长
	struct timeval tv;
	struct timeval begin;
	struct timeval end;
	// 文件目录名：脚本名称_时间戳。目录中task.info文件记录任务信息
	// 相同命令，必须在上次命令执行完毕后才执行下一个
	gettimeofday(&tv, 0);
	now = tv.tv_sec * 1000000 + tv.tv_usec;//微秒
	snprintf(foldname_new, MAX_FILE_PATH, "%s%s%lu", req.script.c_str(), "_", now);
	snprintf(filename_new, MAX_FILE_PATH, "%s/res.dat", foldname_new);

	if (access(foldname_new, 0) != -1)
	{// 目录存在
		return -2;
	}
	if (CreateDir(foldname_new))
	{//
		return -3;
	}
	// 任务命令字符串加密为MD5值并作为文件名；
	// 如果同名文件存在，标记任务正在执行；
	// 完毕后删除。
	string md5string = req.script + " " + req.params;
	task_md5 = m_md5.MD5Bits64((var_u1*)md5string.c_str(), md5string.length());
	snprintf(md5file, MAX_FILE_PATH, "%lu", task_md5);
	if (!access(md5file, 0))
	{// 任务正在执行
		ifstream fin(md5file);
		fin.get(foldname_old, MAX_FILE_PATH, 10);
		fin.close();
		while (!access(md5file, 0))
		{
			cp_sleep(1000);
		}
		snprintf(filename_old, MAX_FILE_PATH, "%s/res.dat", foldname_old);
		if (CopyFile(filename_old, filename_new))
		{//
			return -4;
		}
	}
	else
	{
		var_1 command[256];
		// 将执行脚本拷贝到数据目录
		snprintf(filename_old, MAX_FILE_PATH, "scripts/%s", req.script.c_str());
		snprintf(filename_new, MAX_FILE_PATH, "%s/%s", foldname_new, req.script.c_str());
		if (CopyFile(filename_old, filename_new))
		{//
			return -6;	
		}
		// 判断脚本语言，仅支持python和shell
		if (!strncmp(req.script.c_str() + req.script.length() - 3, ".sh", 3))
		{
			snprintf(command, 256, "cd %s;sh %s %s", foldname_new, req.script.c_str(), req.params.c_str());
		}
		else if (!strncmp(req.script.c_str() + req.script.length() - 3, ".py", 3))
		{
			snprintf(command, 256, "cd %s;python %s %s;", foldname_new, req.script.c_str(), req.params.c_str());
		}
		else
		{//
			return -5;
		}
		ofstream fout(md5file);
		fout<<foldname_new;
		fout.close();
		// 计时开始
		gettimeofday(&begin, 0);
		system(command);
		// 计时结束
		gettimeofday(&end, 0);
		execution = end.tv_sec - begin.tv_sec;
		RemoveFile(md5file);
		if (errno == 127 || errno == -1)
		{//
			return errno; 
		}
	}
	struct stat st;
	data_path.assign(filename_new); 	
	var_1 json_value[MAX_BUFFER_SIZE+1];	
	// 获取客户端IP
	static const char *ip  = CSocketPack::GetIPStr(sockfd);
	res.ip.assign(ip);
	// 写 struct response结构体
	res.datasize = 0;
	if (!stat(data_path.c_str(), &st))
	{
		res.datasize = st.st_size;
	}
	res.sendsize = (res.datasize > MAX_BUFFER_SIZE?MAX_BUFFER_SIZE:res.datasize);
	ifstream fin(data_path.c_str());
	fin.read(json_value, res.sendsize);
	fin.close();
	res.value.assign(json_value, res.sendsize);
	res.timestamp = tv.tv_sec;
	res.script = req.script;
	res.params = req.params;
	res.execution = execution;

	return 0;
}


