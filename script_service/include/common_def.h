#ifndef __COMMON_DEF_H_
#define __COMMON_DEF_H_

#include "UH_Define.h"
#include "UC_MD5.h"
#include "UC_ReadConfigFile.h"

class UC_ReadConfigFile;

#define MAX_WORKER_NUM_PER_WORKSHOP (20)
#define MAX_WORKERSHOP_NUM_PER_FACTORY (10)
#define WORKER_HEARTBEAT_INTERVAL (300) // seconds
#define MAX_WORKER_TYPE_LEN (20)

class worker_inf
{
public:
    var_1 _ip[16];
    var_u2 _port;

    friend class factory;
    friend class factory_proxy;
private:
    var_u8 _ID;
};

typedef struct tag_workshop_inf
{
    //workshop_name _name;
    var_1  _type[MAX_WORKER_TYPE_LEN];

    var_4 _count;
    worker_inf _workers[MAX_WORKER_NUM_PER_WORKSHOP];

}workshop_inf, *pworkshop_inf;

typedef struct tag_pub_protocal
{
	var_u4          _WS_cnt;
	workshop_inf    _WS_infs[MAX_WORKERSHOP_NUM_PER_FACTORY];

}pub_protocal, *ppub_protocal;

const var_u4 pub_protocal_zize = sizeof(pub_protocal);
const var_u4 workshop_inf_size = sizeof(workshop_inf);
const var_u4 worker_inf_size = sizeof(worker_inf);
const var_u4 max_workers_count = MAX_WORKER_NUM_PER_WORKSHOP * MAX_WORKERSHOP_NUM_PER_FACTORY;

template<typename T1, typename T2>
class simple_pair
{
public:
    T1 _left;
    T2 _right;
};

static int sys_error_code()
{
#ifdef _WIN32_ENV_
    return GetLastError();
#else
    return errno;
#endif
}

#define __ZERO_MEMORY(p, l) memset(p, 0, l)

#define _DEBUG_P
#ifdef _DEBUG_P
#define PRINT_DEBUG_INFO printf
#define LOG_ERROR(x,y)        printf("FILE: %s LINE: %d\nERROR: [%s] [%s] \n", __FILE__, __LINE__, x, y)
#define LOG_WARNING(x)      printf("FILE: %s LINE: %d\nWARNING: [%s] \n", __FILE__, __LINE__, x)
#define LOG_NULL_POINTER(x) printf("FILE: %s LINE: %d\nERROR: [%s] is NULL pointer\n", __FILE__, __LINE__, x)
#define LOG_INVALID_PARAMETER(x,y) printf("FILE: %s LINE: %d\nERROR: %s parameter is invlalid, it's[[%s]]\n", __FILE__, __LINE__, x, y)
#define LOG_FAILE_CALL(x,y) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s]\n",__FILE__, __LINE__, x, y)
#define LOG_FAILE_CALL_PARAM(x,y,z) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], parameter is [%s]\n", __FILE__, __LINE__, x, y, z)
#define LOG_FAILE_CALL_RET(x,y,r) printf("FILE: %s LINE: %d\nERROR: in [%s], failed to call [%s], return [%d]\n", __FILE__, __LINE__, x, y, r)
#define LOG_PARAM(x,y,z) printf("FILE: %s LINE: %d\nERROR: in [%s], parameter [%s] is [%s]\n", __FILE__, __LINE__, x, y, z)
#define LOG_FAILE_NEW(x) printf("FILE: %s LINE: %d\nERROR: failed to new [%s] object\n", __FILE__, __LINE__, x)
#define LOG_FAILE_DUPSTR(x) printf("FILE: %s LINE: %d\nERROR: failed to duplicate [%s] string\n", __FILE__, __LINE__, x)
#else
#define PRINT_DEBUG_INFO
#define LOG_ERROR(x,y)
#define LOG_WARNING(x)
#define LOG_NULL_POINTER(x)
#define LOG_INVALID_PARAMETER(x,y)
#define LOG_FAILE_CALL(x,y)
#define LOG_FAILE_CALL_PARAM(x,y,z)
#define LOG_FAILE_CALL_RET(x,y,r)
#define LOG_PARAM(x,y,z)
#define LOG_FAILE_NEW(x)
#define LOG_FAILE_DUPSTR(x)
#endif

#endif