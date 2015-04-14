#ifndef __FACTORY_PROXY_H_
#define __FACTORY_PROXY_H_

#include "common_def.h"

class factory_proxy
{
public:
    factory_proxy();

    ~factory_proxy();

    //FUNC  ����
    //PARAM _pub_addr��ָʾ�������Ĺ㲥��ַ
    //      _heartbeat_addr��ָʾ��������������ַ
    //      _clt_IPorts����ǰӦ���ṩ�ķ����ַ
    //      _clt_types ����ǰӦ���ṩ�ķ�������
    //      _clt_count ����ǰӦ���ṩ�ķ������
    //RET   >0���ɹ�
    //      ������ʧ��
    var_4 start(
        var_1* _pub_addr, 
        var_1* _heartbeat_addr, 
        worker_inf* _clt_IPorts, 
        var_1** _clt_types,
        var_u4 _clt_count
        );

    var_4 stop();

    //FUNC  ��ȡworker��Ϣ
    //PARAM _name��ָʾ����ȡ��������
    //      _workers_buffer��д��worker��Ϣ��buffer��ַ
    //      _max_workers_cnt ��д��worker��Ϣ��buffer��С
    //RET   >0��д��buffer��worker����
    //      ������ʧ��
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