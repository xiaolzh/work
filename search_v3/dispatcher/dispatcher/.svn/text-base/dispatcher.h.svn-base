#ifndef DISPATCHER_H
#define DISPATCHER_H
/// head files include
#include "swstd.h"
#include <vector>
#include <string>
#include <map>
#include <boost/shared_ptr.hpp>
#include "mailer.h"
#include "data_handler_adapter.h"
#include "global_define.h"

class NetServer;

struct HandlerStateInfo{
    UpdateStatus status;
    std::string data_path;
    int main_stamp;
    std::string main_update_time;
};

class Dispatcher {
public:
    typedef boost::shared_ptr<NetServer> NetServerPtr;
    typedef boost::shared_ptr<DataHandlerAdapter> DHPtr;
    typedef boost::shared_ptr<HandlerStateInfo> HandlerStateInfoPtr;
    typedef struct DataHandlerInfo {
        Dispatcher* owner_ptr;
        std::string module_name;
        DHPtr dh_ptr;
        pthread_t thrd_id;
        HandlerStateInfoPtr state_ptr;
        bool is_run;
    }DataHandlerInfo;
    typedef std::map<std::string, DataHandlerInfo> DH_MAP;
    typedef DH_MAP::iterator DItor;
public:
    Dispatcher();
    ~Dispatcher();

    bool Init(const std::string& config);
    bool CoInit();
    
    bool Run();
    bool UpdateAll();
    bool StopUpdateAll();
    bool ControlUpdateAll(UpdateStatus status);

    bool UpdateData(const std::string& module);
    bool StopUpdate(const std::string& module);

    bool ControlUpdate(const std::string& module, UpdateStatus status);

    bool StartNetServer();
    bool StopNetServer();

    /// get inc data of module with inc stamp
    int GetIncData ( const std::string& module, int main_stamp, 
                     int inc_stamp, std::vector<char>& data );
    /// get full data of module with key
    bool GetFullData ( const std::string& module, const std::string& key,
                      std::vector<char>& data );

    bool GetInfo(const std::string& config, std::vector<char>& info_buf);
    bool LoadData(const std::string& module, const std::string& data_path); 
private:
    static void * _Run(void * arg);  
    bool _LoadConfig(const std::string& config); 
    bool _InitLogger(const std::string& config);
    bool _AddModuleData(const std::string& module,
        const std::string& data_path);
    bool _CheckSearcher(const std::string& module,
        const std::string& data_path);
    bool _UpdateData(const std::string& module, 
        const std::string& data_path);
    inline std::string _GetDataPath(const std::string& module) {
        return m_data_handlers[module].dh_ptr->GetDataPath();
    }
    inline int _GetMainStamp(const std::string& module) {
        return m_data_handlers[module].state_ptr->main_stamp;
    }
    inline int _GetIncStamp(const std::string& module) {
        return m_data_handlers[module].dh_ptr->GetIncStamp();
    }
    void _NotifyError(const std::string& err_info);
    void _MD5Dir(const std::string& data_path);
    bool _LoadState(const std::string& module);
    bool _SaveState(const std::string& module);
    void _UpdateStatus(const std::string& module);

    bool _ReadTimeStamp(const std::string& file, int& tm);
    bool _WriteTimeStamp(const std::string& file, int tm);

private:
    /// config
    std::string m_main_data_url; 
    std::string m_main_data_path;
    std::string m_cache_data_path;
    std::string m_searcher_path;
    std::vector<std::string> m_data_handler_list;
    /// the listen port of Searcher for testing
    std::string m_searcher_port;
    int m_bak_file_num;

    NetServerPtr   m_net_server_ptr;
    DH_MAP m_data_handlers;
    pthread_mutex_t m_lock;
    
    Mailer m_mailer;
    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(Dispatcher);
};

#endif // ~>.!.<~ 
