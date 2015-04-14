#include "data_handler_adapter.h"
#include <dlfcn.h>
#include <algorithm>
#include <iterator>
#include "logger.h"
#include "global_define.h"
using std::string;
using std::vector;
using std::copy;
using std::back_inserter;

static const char* MODULE_PATH = "./modules/";

/// DataHandlerAdapter construction
DataHandlerAdapter::DataHandlerAdapter() {
    _Empty();
}

/// DataHandlerAdapter destruction
DataHandlerAdapter::~DataHandlerAdapter() {
    if( NULL != m_dl_handle) {
        CoInit();
    }
}

bool DataHandlerAdapter::Init(const string& config) {
    string module_path = string(MODULE_PATH) + config + string("/");
    m_dl_file = module_path + config + string(".so");
    if ( ! _LoadDynamicLib()) return false;
    if ( ! _InvokeFunctions()) return false;
    if ( 0 != m_init(module_path.c_str()) ) return false;
    return true;
}

bool DataHandlerAdapter::CoInit() {
    /// this function should always return true
    if ( 0 != m_co_init() ) {
        LOG(LOG_ERROR, "Fail to co_init, [%s]", m_dl_file.c_str());
        return false;
    }
    _UnloadDynamicLib();
    _Empty();
    return true;
}

bool DataHandlerAdapter::ProcessData(const string& config) {
    if ( 0 != m_process_data(config.c_str()) ) {
        LOG(LOG_ERROR, "Fail to process data [%s]", config.c_str());
        return false;
    }
    return true;
}

bool DataHandlerAdapter::GetIncData( int inc_time_stamp, 
                                     vector<char>& data_buf ) {
    int data_len = m_get_inc_data(inc_time_stamp, NULL, 0);
    if ( data_len < 0 ) return false;
    data_buf.clear();
    data_buf.resize(data_len);
    data_len = m_get_inc_data(inc_time_stamp, (void*)&data_buf[0], data_len);
    if ( data_len > (int)data_buf.size() || data_len < 0 ) 
        return false;
    return true;    
}

bool DataHandlerAdapter::GetFullData( const std::string& key,
                                      std::vector<char>& data_buf) {
    data_buf.clear();
    int buf_size = 8092;
    data_buf.resize(buf_size);
    int len = m_get_full_data( key.c_str(), (void *)&data_buf[0], buf_size);
    while ( len >= buf_size ) {
        buf_size = buf_size<<1;
        data_buf.resize(buf_size);
        len = m_get_full_data(key.c_str(), (void *)&data_buf[0], buf_size);
    }
    if ( len < 0 ) return false;
    data_buf.resize(len);
    return true;
}

int DataHandlerAdapter::GetIncStamp() {
    return m_get_inc_stamp();
}

int DataHandlerAdapter::GetUpdateStatus() {
    return m_get_update_status();
}

string DataHandlerAdapter::GetDataPath() {
    char data_path[1024];
    if ( m_get_data_path(data_path, sizeof(data_path)) > 0 ) {
        return string(&data_path[0]);
    }
    return "";
}

void DataHandlerAdapter::_Empty() {
    m_dl_file = "";
    m_dl_handle = NULL;
    m_init = NULL;
    m_co_init = NULL;
    m_process_data = NULL;
    m_get_inc_data = NULL;
    m_get_full_data = NULL;
    m_get_inc_stamp = NULL;
    m_get_update_status = NULL;
    m_get_data_path = NULL;
}

bool DataHandlerAdapter::_LoadDynamicLib() {
    void* dl_handle = NULL;
    /// open the shared object
    dl_handle = dlopen(m_dl_file.c_str(), RTLD_LAZY);
    if( !dl_handle) {
        LOG(LOG_ERROR, "Fail to open the shared object: [%s], error: [%s]",
            m_dl_file.c_str(), dlerror());
        return false;
    }
    m_dl_handle = dl_handle;
    return true;
}

bool DataHandlerAdapter::_InvokeFunctions() {
    if( ! _InvokeMethod(m_dl_handle, "init", (void*&)m_init)) {
         return false;
    }
    if( ! _InvokeMethod(m_dl_handle, "co_init", (void*&)m_co_init)) {
        return false;
    }
    if( ! _InvokeMethod(m_dl_handle, "process_data", (void*&)m_process_data)){
        return false;
    }
    if( ! _InvokeMethod( m_dl_handle, "get_inc_data", 
                         (void*&)m_get_inc_data) ) {
        return false;
    }
    if( ! _InvokeMethod( m_dl_handle, "get_full_data", 
                         (void*&)m_get_full_data) ) {
        return false;
    }
    if( ! _InvokeMethod( m_dl_handle, "get_inc_stamp", 
                        (void*&)m_get_inc_stamp) ) {
        return false;
    }
    if( ! _InvokeMethod( m_dl_handle, "get_update_status", 
                        (void*&)m_get_update_status) ) {
        return false;
    }
    if( ! _InvokeMethod( m_dl_handle, "get_data_path",
                        (void*&)m_get_data_path) ) {
        return false;
    }    
    return true;
}

void DataHandlerAdapter::_UnloadDynamicLib() {
    /// close the shared object
    dlclose(m_dl_handle);
}

bool DataHandlerAdapter::_InvokeMethod(void* dl_handle, 
        const string& method_name, void*& dl_func) {
    char *error = NULL;
    /// Resolve the symbol (method) from the object
    dl_func = dlsym(dl_handle, method_name.c_str());
    error = dlerror();
    if (error != NULL) {
        LOG(LOG_ERROR, "Fail to invoke method from dynamic lib, error: [%s]", 
            error);
        return false;
    }
    return true;
}

#ifdef UNIT_TEST
/// include head files for unit test
#include "iostream"
#include <vector>
using namespace std;


bool test_DataHandlerAdapter() {
    cout<<"Unit test - DataHandlerAdapter"<<endl;
    {
        bool ret = true;
        cout<<"usecase: load data handler so file"<<endl;
        DataHandlerAdapter dh;
        ret &= dh.Init("searcher");
        ret &= dh.ProcessData("main");
        vector<char> data_buf;
        ret &= ( !dh.GetIncData(123, data_buf) );
        cout<<string(&data_buf[0])<<endl;
        vector<char> info;
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - DataHandlerAdapter"<<endl;
    return true;
}

#ifdef TEST_DATA_HANDLER_ADAPTER
int main() {
    if(!test_DataHandlerAdapter())
        return -1;
    return 0;
}
#endif // TEST_DATA_HANDLER_ADAPTER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
