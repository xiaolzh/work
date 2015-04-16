#include "script_service.h"

UC_MD5 script_service::m_md5;

script_service::script_service()
: m_large_allocator(NULL)
, m_error_logger(NULL)
, m_task_logger(NULL)
, m_reqsvr_socket(-1)
, m_run_status(true)
{
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
		cout<<	"init config failed";
		return -1;
	}
	m_serv_config = new service_config;
	ret = m_serv_config->read_cfg(conf_reader);
	if (ret)
	{
		cout<< "read config failed";
		return -2;
	}

	assert(NULL == m_large_allocator);
	m_large_allocator = new UC_Allocator_Recycle;
	if (NULL == m_large_allocator)
	{
		cout<< "new m_large_allocator failed";
		return -3;
	}
	var_4 m_large_size = 1<<20;
	var_4 count = 10000;
	
	ret = m_large_allocator->initMem(m_large_size, count, (var_4)sqrt(count));
	if (ret)
	{
		cout<< "init m_large_allocator failed";
		return -4;
	}
	
	var_1 folder[256];
	assert(NULL == m_error_logger);
	m_error_logger = new CDailyLog;
	if (!m_error_logger)
	{
		cout<< "new m_error_logger failed";
		return -5;
	}
	sprintf(folder, "error_log");
	if (access(folder, 0))
	{
		mkdir(folder, S_IRWXO|S_IRWXU);
	}

	ret = m_error_logger->Init(folder, "err");
	if (ret)
	{
		cout<< "init m_error_logger failed";
		return -6;
	}
	assert(NULL == m_task_logger);
	m_task_logger = new CDailyLog;
	if (!m_task_logger)
	{
		cout<< "new m_task_logger failed";
		return -7;
	}
	sprintf(folder, "task_log");
	if (access(folder, 0))
	{
		mkdir(folder, S_IRWXO|S_IRWXU);
	}
	ret = m_task_logger->Init(folder, "task");
	if (ret)
	{
		cout<< "init m_task_logger failed";
		return -8;
	}

	assert(-1 == m_reqsvr_socket);
	ret = cp_listen_socket(m_reqsvr_socket, m_serv_config->request_port);
	if (ret)
	{
		cout<< "listen socket failed";
		return -9;
	}
	// 接收请求线程
	ret = cp_create_thread(thread_request, (void*)this);
	if (ret)
	{
		cout<< "create thread_request failed";
		return -10;
	}
	ret = cp_create_thread(thread_process, (void*)this);
	if (ret)
	{
		cout<< "create thread_process failed";
		return -11;
	}
	// 发送数据线程
	ret = cp_create_thread(thread_reply, (void*)this);
	if (ret)
	{
		cout<< "create thread_reply failed";
		return -12;
	}
	// 清理线程
	ret = cp_create_thread(thread_clear, (void*)this);
	if (ret)
	{
		cout<< "create thread_clear failed";
		return -13;
	}
	ret = cp_create_thread(thread_mail, (void*)this);
	if (ret)
	{
		cout<< "create thread_mail failed";
		return -14;
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
			_this->m_error_logger->LPrintf(true, "create thread_work failed\n");	
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
		m_error_logger->LPrintf(true, "parse json failed:%s\n", json_value.c_str());
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

var_4 script_service::json_write(struct response res, string& json_value)
{
	Json::Value root;
	root["timestamp"] = res.timestamp;
	root["ip"] = res.ip;
	root["script"] = res.script;
	root["params"] = res.params;
	root["execution"] = res.execution;
	
	root["data_size"] = res.data_size;
	root["send_size"] = res.send_size;
	root["value"] = res.value;
	root.toStyledString();
	json_value = root.toStyledString();
	return 0;
}

var_4 script_service::json_write(struct mail_task mt, string& json_value)
{
	Json::Value root, value;
	Json::Reader reader;
	if (false == reader.parse(mt.t.json_value, value))
	{
		m_error_logger->LPrintf(true, "parse json failed:%s\n", mt.t.json_value.c_str());
		return -1;
	}
	root["timestamp"] = value["timestamp"];
	root["ip"] = value["ip"];
	root["script"] = value["script"];
	root["params"] = value["params"];
	root["execution"] = value["execution"];

	root["data_size"] = value["data_size"];
	root["send_size"] = value["send_size"];
	
	root["error_code"] = mt.t.error_code;
	root["email_num"] = mt.email_num;
	root["data_path"] = mt.data_path;
	Json::Value emails;
	for (size_t i = 0; i < mt.email_num; i++)
	{
		emails[i] = mt.emails[i];
	}
	root["emails"] = emails;
	root.toStyledString();
	json_value = root.toStyledString();
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
	var_1 task_info[1024]; 
	var_4 ret = _this->json_read(t->json_value, req); 
	if (ret)
	{
		_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("thread_work", "json_read", ret));
	}
	else
	{
		ret = _this->parse_request(t->sockfd, req, res, data_path);
		if (ret)
		{
			_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("thread_work", "parse_request", ret));
		}
		else
		{
			ret = _this->json_write(res, json_value);
			if (ret)
			{
				_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("thread_work", "json_write", ret));
			}
			else
			{// 记录任务信息
				snprintf(task_info, 1024, "\ntimestamp:%d\nip:%s\nscript:%s\nparams:%s\nexecution:%d\ndata_size:%d\n",
						 res.timestamp, res.ip.c_str(), res.script.c_str(), res.params.c_str(), res.execution, res.data_size);
				_this->m_task_logger->LPrintf(false, "%s", task_info);
				_this->m_task_logger->LPrintf(false, "\n-------------------------\n");
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
	
	struct mail_task mt;
	var_1* buffer = block_alloc(_this->m_large_allocator);
	while (_this->m_run_status)
	{
		var_4 send_size = 0, data_size = 0;
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
			struct stat st;
			if (!stat(mt.data_path.c_str(), &st))
			{
				data_size = st.st_size;
			}
			if (MAX_BUFFER_SIZE < data_size)
			{
				need_mail = true;
			}

			send_size  = mt.t.json_value.length();
			snprintf(buffer + 8, 9, "%-8d", send_size);
			snprintf(buffer + 16, send_size + 1, "%s", mt.t.json_value.c_str());
			send_size += 16;
		}
		var_4 ret = cp_sendbuf(mt.t.sockfd, buffer, send_size);
		if (ret)
		{
			need_mail = true;
			_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("thread_reply", "cp_sendbuf", ret));	
		}
		cp_recvbuf(mt.t.sockfd, buffer, 1);
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
	string json_value;
	string command;
	while (_this->m_run_status)
	{
		while (_this->m_mail_queue.empty())
		{
			cp_sleep(10);
		}
		mt = _this->m_mail_queue.front();
		_this->m_mail_queue.pop();
		var_4 ret = _this->json_write(mt, json_value);
		if (ret)
		{
			_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("thread_mail", "json_write", ret));
			continue;	
		}
		command = "python sendmail.py '" + json_value + "'";
		system(command.c_str());
		if (errno == 127 || errno == -1)
		{
			_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("thread_mail", "python sendmail.py", errno));	
		}
	}
	return NULL;
}

void* script_service::thread_clear(void* param)
{
	script_service* _this = static_cast<script_service*>(param);
	
	DIR *dirp = NULL;
	struct dirent *direntp = NULL;
	struct timeval tv;
	while (_this->m_run_status)
	{
		/*
		 * 清理超过7天的数据；
		 * 文件夹命名规则：xxx_1429148362776987
		 * 时间戳精度为微秒，长度16位
		 */
		gettimeofday(&tv, 0);
		dirp = opendir(".");
		if (!dirp)
		{
			_this->m_error_logger->LPrintf(true, FAILE_CALL_RET("thread_clear", "opendir", errno));
			continue;
		}
		while (direntp = readdir(dirp))
		{
			var_4 name_len = strlen(direntp->d_name);
			if (name_len < 17)
			{
				continue;
			}
			if (!(direntp->d_type == DT_DIR))
			{
				continue;
			}
			size_t i = name_len - 1, j = 0;
			for (; i >= 0 && j < 16; i--, j++)
			{
				if (!(direntp->d_name[i] >= '0' && direntp->d_name[i] <= '9'))
				{
					break;
				}
			}
			if (j != 16)
			{
				continue;
			}
			if (i >= 0 && direntp->d_name[i] != '_')
			{
				continue;
			}
			var_8 timestamp = strtoul(direntp->d_name + name_len - 16, 0, 10);
			if (tv.tv_sec * 1000000 + tv.tv_usec > timestamp + 1000000L * 86400 * 7)
			{// 数据日期早于7天，删除
				ClearFolder(direntp->d_name);
				RemoveFile(direntp->d_name);
			}
		}
		closedir(dirp);
		cp_sleep(60000);
	}
	return NULL;
}


var_4 script_service::parse_request(CP_SOCKET_T sockfd, struct request req,  struct response& res, string& data_path)
{
	var_u8 now, task_md5;
	// 任务目录
	var_1 foldname_new[MAX_FILE_PATH];
	var_1 foldname_old[MAX_FILE_PATH];
	// 数据文件
	var_1 filename_new[MAX_FILE_PATH];
	var_1 filename_old[MAX_FILE_PATH];
	// 脚本文件
	var_1 script_new[MAX_FILE_PATH];
	var_1 script_old[MAX_FILE_PATH];
	// 任务命令加密
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
		m_error_logger->LPrintf(true, "folder is existed:%s\n", foldname_new);
		return -1;
	}
	if (CreateDir(foldname_new))
	{
		m_error_logger->LPrintf(true, "create dir failed:%s\n", foldname_new);
		return -2;
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
		{
			m_error_logger->LPrintf(true, "copy file %s to %s failed.\n", filename_old, filename_new);
			return -3;
		}
	}
	else
	{
		var_1 command[1024];
		// 将执行脚本拷贝到数据目录
		snprintf(script_old, MAX_FILE_PATH, "scripts/%s", req.script.c_str());
		snprintf(script_new, MAX_FILE_PATH, "%s/%s", foldname_new, req.script.c_str());
		if (CopyFile(script_old, script_new))
		{
			m_error_logger->LPrintf(true, "copy file %s to %s failed.\n", script_old, script_new);
			return -4;	
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
		{
			m_error_logger->LPrintf(true, "script not support: %s\n", req.script.c_str());
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
		{
			m_error_logger->LPrintf(true, "system failed:%s\n", command);
			return errno; 
		}
	}
	data_path.assign(getcwd(NULL, 0));
	data_path.append("/");
	data_path.append(filename_new); 	
	
	struct stat st;
	var_1 json_value[MAX_BUFFER_SIZE+1];	
	// 获取客户端IP
	static const char *ip  = CSocketPack::GetIPStr(sockfd);
	res.ip.assign(ip);
	// 写 struct response结构体
	res.data_size = 0;
	if (!stat(data_path.c_str(), &st))
	{
		res.data_size = st.st_size;
	}
	res.send_size = (res.data_size > MAX_BUFFER_SIZE?MAX_BUFFER_SIZE:res.data_size);
	ifstream fin(data_path.c_str());
	fin.read(json_value, res.send_size);
	fin.close();
	res.value.assign(json_value, res.send_size);
	res.timestamp = tv.tv_sec;
	res.script = req.script;
	res.params = req.params;
	res.execution = execution;
	// 将任务信息写到任务目录下的task.info
	snprintf(filename_new, MAX_FILE_PATH, "%s/task.info", foldname_new);
	ofstream fout(filename_new);
	fout <<"timestamp: "<<res.timestamp <<endl;
	fout <<"ip: "<<res.ip <<endl;
	fout <<"script: " <<res.script <<endl;
	fout <<"params: " <<res.params <<endl;
	fout <<"execution: "<<res.execution <<endl;
	fout <<"data_size: "<<res.data_size <<endl;
	fout.close();
	return 0;
}


