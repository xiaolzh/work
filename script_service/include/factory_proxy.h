#ifndef __FACTORY_PROXY_H_
#define __FACTORY_PROXY_H_

#include "common_def.h"

class factory_proxy
{
public:
    factory_proxy();

    ~factory_proxy();

    //FUNC  启动
    //PARAM _pub_addr：指示配置中心广播地址
    //      _heartbeat_addr：指示配置中心心跳地址
    //      _clt_IPorts：当前应用提供的服务地址
    //      _clt_types ：当前应用提供的服务类型
    //      _clt_count ：当前应用提供的服务个数
    //RET   >0：成功
    //      其他：失败
    var_4 start(
        var_1* _pub_addr, 
        var_1* _heartbeat_addr, 
        worker_inf* _clt_IPorts, 
        var_1** _clt_types,
        var_u4 _clt_count
        );

    var_4 stop();

    //FUNC  获取worker信息
    //PARAM _name：指示待获取服务类型
    //      _workers_buffer：写入worker信息的buffer地址
    //      _max_workers_cnt ：写入worker信息的buffer大小
    //RET   >0：写入buffer的worker个数
    //      其他：失败
    var_4 get_workers(
        const var_1* _name,
        worker_inf* _workers_buffer,
        const var_u4 _max_workers_cnt
        );


private:

#ifdef _WIN32_ENV_
    static unsigned long __stdcall thread_pub(void* _param);
    static unsigned long __stdcall thread_heartbeat(void* _param);
#else
    static void* thread_pub(void* _param);
    static void* thread_heartbeat(void* _param);
#endif

private:
    bool            m_run_status;
    var_u4          m_active_thread_cnt;

    var_vd*         m_context;
    var_1*          m_pub_addr;
    var_1*          m_HB_addr; //heart beat

    var_u4          m_clt_cnt;
    worker_inf      m_clt_IPorts[MAX_WORKER_NUM_PER_WORKSHOP];    
    var_1           m_clt_types[MAX_WORKER_NUM_PER_WORKSHOP][MAX_WORKER_TYPE_LEN];

    CP_MUTEXLOCK_RW m_lock;
	pub_protocal	m_pub_prot;

};

#endif