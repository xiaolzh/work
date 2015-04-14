#ifndef DATA_HANDLER_ADAPTER_H
#define DATA_HANDLER_ADAPTER_H
/// head files include
#include "swstd.h"
#include <string>
#include <vector>

/// @todo need derived from an interface class: data_handler
class DataHandlerAdapter {
public:
    DataHandlerAdapter();
    ~DataHandlerAdapter();

    bool Init(const std::string& config);
    bool CoInit();
    bool ProcessData(const std::string& config);
    bool GetIncData(int inc_time_stamp, std::vector<char>& data_buf);
    bool GetFullData(const std::string& key, std::vector<char>& data_buf);
    int GetIncStamp();
    int GetUpdateStatus();
    std::string GetDataPath();

private:
    void _Empty();
    bool _LoadDynamicLib();
    bool _InvokeFunctions();
    void _UnloadDynamicLib();
    bool _InvokeMethod( void* dl_handle, const std::string& method_name,
                        void*& func);
private:
    typedef int (*InitFunc)(const char*);
    typedef int (*CoInitFunc)();
    typedef int (*ProcessDataFunc)(const char*);
    typedef int (*GetIncDataFunc)( int, void*, int);
    typedef int (*GetFullDataFunc)( const char*, void*, int);
    typedef int (*GetIncStampFunc)();
    typedef int (*GetUpdateStatusFunc)();
    typedef bool (*GetDataPathFunc)(char*, int);
private:
    std::string     m_dl_file;
    void*           m_dl_handle;
    InitFunc        m_init;
    CoInitFunc      m_co_init;
    ProcessDataFunc m_process_data;
    GetIncDataFunc     m_get_inc_data;
    GetFullDataFunc    m_get_full_data;
    GetIncStampFunc m_get_inc_stamp;
    GetUpdateStatusFunc m_get_update_status;
    GetDataPathFunc m_get_data_path;

    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(DataHandlerAdapter);
};

#endif // ~>.!.<~ 
