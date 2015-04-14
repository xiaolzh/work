#ifndef COMMON_DATA_HANDLER_H
#define COMMON_DATA_HANDLER_H
#include "swstd.h"
#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

class CommonDataHandler {
public:
    CommonDataHandler();
    virtual ~CommonDataHandler();

    virtual bool Init(const std::string& config);
    virtual bool CoInit();
    virtual bool ProcessData(const std::string& config);
    virtual int GetDataLen(int inc_stamp);
    virtual int GetData(int inc_stamp,  void* data_ptr);
    virtual int GetIncStamp();
    virtual int GetUpdateStatus();
    virtual bool GetDataPath(std::string& data_path);
private:
    void _Empty();   
    bool _LoadConfig(const std::string& config_path);
    bool _LoadState(const std::string& state_file);
    bool _SaveState(const std::string& state_file);
    bool _NeedMain();
private:
    struct _StateInfo{
        bool need_main;
        int main_time_stamp;
        std::string main_update_time;
        std::string main_data_dir;
        unsigned int inc_time_stamp;
        std::string inc_update_time;
    };
    typedef _StateInfo*  StatePtr; 

    /// state data
    StatePtr m_cur_state_ptr;
    StatePtr m_bak_state_ptr;
    /// config data
    std::string m_main_data_path;
    std::string m_cache_data_path;
    std::string m_main_process_cmd;
    std::string m_inc_process_cmd;
    float m_main_update_interval;

    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(CommonDataHandler);
};
#endif // ~>.!.<~ 
