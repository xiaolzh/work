#include "factory_proxy.h"
#include "zmq.h"

typedef struct tag_thread_param
{
    var_vd* _pointer;
    var_u4 _index;
}thread_param;

factory_proxy::factory_proxy()
: m_run_status(0)
, m_active_thread_cnt(0)
, m_context(NULL)
, m_pub_addr(NULL)
, m_HB_addr(NULL)
, m_clt_cnt(0)
, m_clt_IPorts()
, m_clt_types()
, m_lock()
, m_pub_prot()
{
    __ZERO_MEMORY(m_clt_IPorts, sizeof(m_clt_IPorts));
    __ZERO_MEMORY(m_clt_types, sizeof(m_clt_types));
    __ZERO_MEMORY(&m_pub_prot, sizeof(m_pub_prot));
}

factory_proxy::~factory_proxy()
{
    if (m_run_status)
    {
        stop();
    }
}

var_4 factory_proxy::start(
    var_1* _pub_addr, 
    var_1* _heartbeat_addr, 
    worker_inf* _clt_IPorts, 
    var_1** _clt_types,
    var_u4 _clt_count)
{
    if ((NULL == _pub_addr) || 
        (NULL == _heartbeat_addr) || 
        (MAX_WORKER_NUM_PER_WORKSHOP < _clt_count))
    {
        return -1;
    }

    assert(NULL == m_pub_addr);
    m_pub_addr = strdup(_pub_addr);
    if (NULL == m_pub_addr)
    {
        return -1;
    }

    assert(NULL == m_HB_addr);
    m_HB_addr = strdup(_heartbeat_addr);
    if (NULL == m_HB_addr)
    {
        return -1;
    }

    if (m_run_status)
    {
        return 0;
    }
    
    m_context = zmq_ctx_new();
    if (!m_context)
    {
        return -1;
    }

    m_run_status = 1;

    memcpy(m_clt_IPorts, _clt_IPorts, _clt_count * sizeof(worker_inf));

    var_u4 idx = 0u;
    for (; _clt_count > idx; ++idx)
    {
        if (strlen(_clt_types[idx]) > MAX_WORKER_TYPE_LEN)
        {
            return -100;
        }
        strcpy(m_clt_types[idx], _clt_types[idx]);
    }
    m_clt_cnt = _clt_count;

    thread_param prm;
    prm._pointer = this;
    for (idx = 0u; _clt_count > idx; ++idx)
    {
        m_lock.lock_w();
        prm._index = idx;
        var_4 ret = cp_create_thread(thread_heartbeat, &prm);
        if (ret)
        {
            m_lock.unlock();
            return  -4;
        }
    }
    
    m_lock.lock_w();
    m_lock.unlock();

    var_4 ret = cp_create_thread(thread_pub, this);
    if (ret)
    {
        return -4;
    }

    return 0;
}

var_4 factory_proxy::stop()
{
    m_run_status = 0;

    while (m_active_thread_cnt)
    {
        cp_sleep(100);
    }
        
    if (NULL != m_pub_addr)
    {
        free(m_pub_addr);
    }

    if (NULL != m_HB_addr)
    {
        free(m_HB_addr);
    }

    if (NULL != m_context)
    {
        zmq_ctx_destroy(m_context);
        m_context = NULL;
    }

    return 0;
}

#ifdef _WIN32_ENV_
unsigned long __stdcall factory_proxy::thread_pub(void* _param)
#else
void* factory_proxy::thread_pub(void* _param)
#endif
{
    factory_proxy* _this = static_cast<factory_proxy*>(_param);
    assert(NULL != _this);

    cp_lock_inc(&_this->m_active_thread_cnt);

    var_vd* subscriber = zmq_socket(_this->m_context, ZMQ_SUB);
    assert(subscriber);

    var_4 ret = zmq_connect(subscriber, _this->m_pub_addr);
    assert(!ret);
    
    ret = zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);
    assert(!ret);

    const var_u4 buf_len = pub_protocal_zize;
    var_vd* buf = malloc(buf_len);
    assert(buf);
    
    while (_this->m_run_status)
    {//广播配置信息
        
        var_4 ret = zmq_recv(subscriber, buf, buf_len, 0);
        if (0 > ret)
        {
            LOG_FAILE_CALL_RET("factory_proxy::thread_pub", "::zmq_recv", sys_error_code());
            continue;
        }
        else if (buf_len < ret)
        {
            ret = buf_len;
        }
        else if (buf_len > ret )
        {
			assert(0);
        }
		else
		{

		}
        PRINT_DEBUG_INFO("succeed factory_proxy::thread_pub::zmq_recv\n");

        _this->m_lock.lock_w();
        memcpy(&_this->m_pub_prot, buf, ret);
        _this->m_lock.unlock();
        
        cp_sleep(1000);
    }

    free(buf);
    zmq_close(subscriber);
    
    cp_lock_dec(&_this->m_active_thread_cnt);

    return 0;
}

#ifdef _WIN32_ENV_
unsigned long __stdcall factory_proxy::thread_heartbeat(void* _param)
#else
void* factory_proxy::thread_heartbeat(void* _param)
#endif
{
    factory_proxy* _this = static_cast<factory_proxy*>((static_cast<thread_param*>(_param))->_pointer);
    assert(NULL != _this);
    var_u4 idx = (static_cast<thread_param*>(_param))->_index;
    _this->m_lock.unlock();

    cp_lock_inc(&_this->m_active_thread_cnt);
    
    var_vd* pusher = zmq_socket(_this->m_context, ZMQ_REQ);
    assert(pusher);

    var_4 ret = zmq_connect(pusher, _this->m_HB_addr);
    assert(!ret);

    var_4 one = 1;
    ret = zmq_setsockopt(pusher, ZMQ_SNDHWM, &one, sizeof(one));
    assert(!ret);
    one = 1000;//ms
    ret = zmq_setsockopt(pusher, ZMQ_SNDTIMEO, &one, sizeof(one));
    assert(!ret);

    UC_MD5 md5er;
    _this->m_clt_IPorts[idx]._ID = 0;
    _this->m_clt_IPorts[idx]._ID = md5er.MD5Bits64((var_u1*)(_this->m_clt_IPorts + idx), worker_inf_size);
    
    var_u4 buf_len = worker_inf_size + 4 + MAX_WORKER_TYPE_LEN;
    var_vd* buf = malloc(buf_len);
    assert(buf);

    memcpy(buf, _this->m_clt_IPorts + idx, worker_inf_size);
    
    *(var_u4*)((var_1*)buf + worker_inf_size) = strlen(_this->m_clt_types[idx]);

    strcpy((var_1*)buf + worker_inf_size + 4, _this->m_clt_types[idx]);
    buf_len = worker_inf_size + 4 + *(var_u4*)((var_1*)buf + worker_inf_size);
    
	var_1 rcv[1024];
    while (_this->m_run_status)
    {// 发送心跳
        ret = zmq_send(pusher, buf, buf_len, 0);
        if (0 > ret)
        {
            if (sys_error_code() != 11)
                LOG_FAILE_CALL_RET("factory_proxy::thread_heartbeat", "::zmq_send", sys_error_code());   
            continue;
        }

		ret = zmq_recv(pusher, rcv, 1024, 0);

        //PRINT_DEBUG_INFO("succeed factory_proxy::thread_heartbeat::zmq_send\n");

        cp_sleep(200);
    }

    free(buf);
    zmq_close(pusher);

    cp_lock_dec(&_this->m_active_thread_cnt);
    
    return 0;
}

var_4 factory_proxy::get_workers(
    const var_1* _name,
    worker_inf* _workers_buffer,
    const var_u4 _max_workers_cnt)
{
    if ((NULL == _workers_buffer) || (0 >= _max_workers_cnt))
    {
        return -1;
    }

    var_4 ret = 0;

    m_lock.lock_r();
    
    workshop_inf* beg = m_pub_prot._WS_infs;
    workshop_inf* end = m_pub_prot._WS_infs + m_pub_prot._WS_cnt;

    for (; end != beg ; ++beg)
    {
        if (!strcmp(_name, beg->_type))
        {
            ret = beg->_count;
            if (_max_workers_cnt < beg->_count)
            {
                ret *= -1;
            }
            else
            {
                memcpy(_workers_buffer, beg->_workers, ret * worker_inf_size);
            }
            break;
        }
    }

    m_lock.unlock();

    return ret;
}

int mains(int argc, char* argv[])
{
    factory_proxy factoryer;
    //tcp://127.0.0.1:7777 为ZMQ的广播端地址，需在各子系统配置文件内指定。
    worker_inf cur_worker;
    strcpy(cur_worker._ip, "127.0.0.1");
    cur_worker._port = atoi(argv[1]);
    var_1* cur_types = "acquire";
    var_4 ret = factoryer.start("tcp://127.0.0.1:9777", "tcp://127.0.0.1:9888", &cur_worker, &cur_types, 1);
    if (ret)
    {
        LOG_FAILE_CALL_RET("::main", "factoryer.start", ret);
        return -1;
    }

    while (1)
    {
        worker_inf workers_buf[MAX_WORKER_NUM_PER_WORKSHOP];
        ret = factoryer.get_workers("acquire", workers_buf, MAX_WORKER_NUM_PER_WORKSHOP);
        if (ret < 0)
        {
            LOG_FAILE_CALL_RET("::main", "factoryer.get_workers", ret);
            return -1;
        }
        for (var_u4 idx = 0u; ret > idx; ++idx)
        {
            printf("第%d个负责采集的worker信息：【IP：%s】【port：%d】\n",
                idx + 1, workers_buf[idx]._ip, workers_buf[idx]._port);
        }
        cp_sleep(10000);
    }

    return 0;
}