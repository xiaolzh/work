#include "dispatcher.h"
#include <errno.h>
#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "configer.h"
#include "utilities.h"
#include "net_server.h"
#include "global_define.h"
#include "mailer.h"
#include "timer.h"
using namespace std;
using boost::lexical_cast;

string LOG_NAME = "dispatcher.log";


/// Dispatcher construction
Dispatcher::Dispatcher() {
    pthread_mutex_init(&m_lock,NULL);
}

/// Dispatcher destruction
Dispatcher::~Dispatcher() {
    pthread_mutex_destroy(&m_lock);
}

bool Dispatcher::Init(const string& config) {
    if( !_LoadConfig(CONFIG_FILE)) return false;
    /// data handlers initialize
    for(size_t i=0; i < m_data_handler_list.size(); ++i) {
        DHPtr dh_ptr = DHPtr(new DataHandlerAdapter);
        if( ! dh_ptr->Init(m_data_handler_list[i])) {
            LOG(LOG_ERROR, "Fail to load and initialize lib: [%s]", 
                m_data_handler_list[i].c_str());
            return false;
        }
        DataHandlerInfo info;
        info.owner_ptr = this;
        info.module_name = m_data_handler_list[i];
        info.dh_ptr = dh_ptr;
        info.thrd_id = 0;
        info.state_ptr = HandlerStateInfoPtr(new HandlerStateInfo);
        info.is_run = false;
        m_data_handlers[m_data_handler_list[i]] = info;

        if( ! CreatePath(m_main_data_path + info.module_name) ) return false;
    }
    for(size_t i=0; i < m_data_handler_list.size(); ++i) {
        if( !_LoadState(m_data_handler_list[i])) return false;
    }
    /// net data sender server initialize
    m_net_server_ptr = NetServerPtr(new NetServer);
    if( ! m_net_server_ptr->Init(string(CONFIG_FILE), this)) {
        LOG(LOG_ERROR, "Fail to init data sender");
        return false;
    }
    if( ! _InitLogger(CONFIG_FILE)) return false;
    return true;
}

bool Dispatcher::CoInit() {
    bool ret = true;
    /// stop data sender sever
    StopNetServer();
    /// check threads and stop them
    StopUpdateAll();
    if(NULL != m_net_server_ptr.get()) {
        m_net_server_ptr->CoInit();
    }
    for(DItor itor = m_data_handlers.begin(); itor != m_data_handlers.end();
        ++itor) {
        (itor->second).dh_ptr->CoInit();
    }
    return ret;
}

bool Dispatcher::Run() {
    if( !UpdateAll()) return false;
    if( !StartNetServer()) return false;
    /// wait for data update stopped
    for(DItor itor = m_data_handlers.begin(); itor != m_data_handlers.end();
        ++itor) {
        pthread_join((itor->second).thrd_id, NULL);
    }
    LOG(LOG_DEBUG, "data update threads are joined");
    /// wait for net server stopped
    m_net_server_ptr->WaitForQuit();
    LOG(LOG_DEBUG, "net server is quit");
    return true;
}

bool Dispatcher::UpdateAll() {
    LOG(LOG_DEBUG, "Start updating all ...");
    for(DItor itor = m_data_handlers.begin(); itor != m_data_handlers.end();
        ++itor) {
        /// begin thread for handling the data update
        int err = pthread_create(&((itor->second).thrd_id), NULL, _Run,
            (void*)&(itor->second));
        if( 0 != err ) {
            LOG(LOG_ERROR, "Fail to create thread: %s", strerror(err));
            return false;
        }
    }
    return true;
}

bool Dispatcher::StopUpdateAll() {
    LOG(LOG_DEBUG, "Stop updating all ...");
    for(DItor itor = m_data_handlers.begin(); itor != m_data_handlers.end();
        ++itor) {
        (itor->second).is_run = false;
        //pthread_join((itor->second).thrd_id, NULL);
    }
    LOG(LOG_DEBUG, "Done to stop updating all");
    return true;
}

bool Dispatcher::ControlUpdateAll(UpdateStatus status) {
    LOG(LOG_DEBUG, "Control updating all ...");
    for(DItor itor = m_data_handlers.begin(); itor != m_data_handlers.end();
        ++itor) {
        if( ! ControlUpdate(itor->first, status)) return false;
    }
    return true;
}

bool Dispatcher::UpdateData(const string& module) {
    LOG(LOG_DEBUG, "Start updating data of module [%s]", module.c_str());
    DItor itor = m_data_handlers.find(module);
    if( m_data_handlers.end() == itor) {
        LOG(LOG_ERROR, "Fail to find module [%s]", module.c_str());
        return false;
    }
    /// begin thread for handling the data update
    int err = pthread_create(&((itor->second).thrd_id), NULL, _Run,
        (void*)&(itor->second));
    if( 0 != err ) {
        LOG(LOG_ERROR, "Fail to create thread: %s", strerror(err));
        return false;
    }
    return true;
}

bool Dispatcher::StopUpdate(const string& module) {
    LOG(LOG_DEBUG, "Stop updating data of module [%s]", module.c_str());
    DItor itor = m_data_handlers.find(module);
    if( m_data_handlers.end() == itor) {
        LOG(LOG_ERROR, "Fail to find module [%s]", module.c_str());
        return false;
    }
    (itor->second).is_run = false;
    pthread_join((itor->second).thrd_id, NULL);
    LOG(LOG_DEBUG, "Done to stop updating of module [%s]", module.c_str());
    return true;
}

bool Dispatcher::ControlUpdate(const string& module, UpdateStatus status) {
    LOG(LOG_DEBUG, "Control updating data of module [%s]", module.c_str());
    DItor itor = m_data_handlers.find(module);
    if( m_data_handlers.end() == itor) {
        LOG(LOG_ERROR, "Fail to find module [%s]", module.c_str());
        return false;
    }
    (itor->second).state_ptr->status = status;
    _SaveState(module);
    return true;
}

bool Dispatcher::StartNetServer() {
    bool ret = true;
    string config = "net server config";
    ret &= m_net_server_ptr->StartServer(config);
    return ret;
}

bool Dispatcher::StopNetServer() {
    bool ret = true;
    ret &= m_net_server_ptr->StopServer();
    LOG(LOG_DEBUG, "Stop net data server");
    return ret;
}

int Dispatcher::GetIncData(const string& module, int main_stamp,
    int inc_stamp, vector<char>& data) {
    if(m_data_handlers.end() == m_data_handlers.find(module)) {
        LOG(LOG_INFO, "unknown module type:[%s]", module.c_str());
        return ILLEGAL_MODULE;
    }
    int cur_main_time_stamp = _GetMainStamp(module);
    int cur_inc_time_stamp = _GetIncStamp(module);
    string data_path = m_main_data_url + string("/")
        + m_data_handlers[module].state_ptr->data_path;
    if(cur_main_time_stamp <=0 ) {
        return NO_DATA;
    }
    if(main_stamp != cur_main_time_stamp) {
        AddPairToVec("main_stamp", 
            lexical_cast<string>(cur_main_time_stamp), data);
        AddPairToVec("data_path", data_path, data);
        return ILLEGAL_MAIN_TIME_STAMP;
    }
    if( inc_stamp >= cur_inc_time_stamp) 
        return NO_DATA;
    DHPtr dh_ptr = m_data_handlers[module].dh_ptr;
    if( ! dh_ptr->GetIncData(inc_stamp, data)) {
        LOG(LOG_ERROR, "Fail to get data of module [%s], man stamp [%d], "
            "inc stamp [%d]", module.c_str(), main_stamp, inc_stamp);
        return ERROR;
    }
    return OK;
}

bool Dispatcher::GetFullData ( const std::string& module, 
                              const std::string& key,
                              std::vector<char>& data ) {
    /// @todo get full data
    LOG(LOG_INFO, "Get full data of module [%s], key [%s]",
        module.c_str(), key.c_str());
    if (m_data_handlers.end() == m_data_handlers.find(module)) {
        LOG(LOG_INFO, "unknown module type:[%s]", module.c_str());
        return false;
    }
    DHPtr dh_ptr = m_data_handlers[module].dh_ptr;
    if( ! dh_ptr->GetFullData(key, data)) {
        LOG(LOG_INFO, "Fail to get data of module [%s], key value [%s]", 
            module.c_str(), key.c_str());
        return false;
    }
    return true;
}

bool Dispatcher::GetInfo(const string& config, vector<char>& info_buf) {
    string info = "//basic info:\n";
    info += string("wget path = ") + m_main_data_url;
    info += string("\ndata path = ") + m_main_data_path;
    info += string("\n\n//data moudles:\n");
    for(DItor itor = m_data_handlers.begin(); itor != m_data_handlers.end();
        ++itor){
        info += string("\nmodule = ") + itor->first;  
        info += string("\nmain time stamp = ") 
            + lexical_cast<string>(_GetMainStamp(itor->first));
        info += string("\ninc time stamp = ")
            + lexical_cast<string>(_GetIncStamp(itor->first));
        info += string("\ndata path = ") + itor->second.state_ptr->data_path;
        info += string("\nupdate running = ") 
            + string(itor->second.is_run?"yes":"no");
        info += "\n";
    }
    info += "\nTODO: more infos ...\n";
    AddStrToVec(info, info_buf);
    return true;
}

bool Dispatcher::LoadData(const string& module, const string& data_path) {
    /// avoid the update thread to overcover
    m_data_handlers[module].state_ptr->status = NO_UPDATE;
    if( !_AddModuleData(module, data_path)) return false;
    if( !_CheckSearcher(module, data_path)) return false;
    if( !_UpdateData(module, data_path)) return false;
    m_data_handlers[module].state_ptr->status = NEED_PROCESS_INC;
    return true;
}

void* Dispatcher::_Run(void* arg) {
    DataHandlerInfo* info_ptr = (DataHandlerInfo*)arg;
    info_ptr->is_run = true;
    Dispatcher* this_ptr = info_ptr->owner_ptr;
    string module = info_ptr->module_name;
    /// @todo config type defines
    while(info_ptr->is_run) {
        if( !this_ptr->_LoadState(module)) {
            info_ptr->is_run = false;
            return NULL;
        }
        this_ptr->_UpdateStatus(module);
        switch(info_ptr->state_ptr->status) {
        case NO_UPDATE:
            /// do nothing
            break;
        case NEED_PROCESS_MAIN:
            {
                timer_counter tc(module + string(" process main"));
                int retry_cnt = 0;
                while( !info_ptr->dh_ptr->ProcessData("main") 
                    && retry_cnt++ < 3 ) {
                    this_ptr->_NotifyError("Fail to do main update");
                    sleep((1<<retry_cnt));
                }
                if(retry_cnt >= 3) {
                    string err_info = string("Stop main updating of module ")
                        + module;
                    this_ptr->_NotifyError(err_info);
                    info_ptr->is_run = false;
                    return NULL;
                }
                string data_path = this_ptr->_GetDataPath(module);
                if( !this_ptr->LoadData(module, data_path)) {
                    this_ptr->_NotifyError("Fail to load main data"); 
                    info_ptr->is_run = false;
                    return NULL;
                }
                /// status turn to process inc
                info_ptr->state_ptr->status = NEED_PROCESS_INC;
            }
            break;
        case NEED_PROCESS_INC:
            {
                /// cost more than 5 mins should give a warning
                timer_counter tc(module + string(" process inc"), LOG_WARN,
                    300000);
                int retry_cnt = 0;
                while( !info_ptr->dh_ptr->ProcessData("inc")
                    && retry_cnt++ < 3 ) { 
                    this_ptr->_NotifyError("Fail to do inc update");
                    sleep((1<<retry_cnt));
                }
                if(retry_cnt >=3) {
                    string err_info = string("Stop inc updating of module ")
                        + info_ptr->module_name;
                    this_ptr->_NotifyError(err_info);
                    info_ptr->is_run = false;
                    return NULL;
                }
            }
            break;
        case NEED_RELOAD_MAIN:
            {
                timer_counter tc(module + string(" reload main"));
                string data_path = this_ptr->_GetDataPath(module);
                if( !this_ptr->LoadData(module, data_path)) {
                    this_ptr->_NotifyError("Fail to reload main data");
                    info_ptr->is_run = false;
                    return NULL;
                }
                /// status turn to process inc
                info_ptr->state_ptr->status = NEED_PROCESS_INC;
            }
            break;
        default:
            LOG(LOG_ERROR, "unknown status [%d] of module [%s]",
                info_ptr->state_ptr->status, 
                info_ptr->module_name.c_str()); 
        }
        if( !this_ptr->_SaveState(module)) {
            info_ptr->is_run = false;
            return NULL;
        }
        /// avoid to visit db frequently when no data to process
        sleep(1);
    }
    info_ptr->is_run = false;
    return NULL;
}

bool Dispatcher::_LoadConfig(const string& config) {
    /// get config from config file
    Configer configer;
    if( ! configer.Load(config)) {
        LOG(LOG_ERROR, "Fail to get config from [%s]", config.c_str());
        return false;
    }
    m_main_data_url = configer.Get("main_data_url");
    if( m_main_data_url.empty()) {
        /// get the host
        string ip = GetLocalIP();
        if( ip.empty()) {
            LOG(LOG_ERROR, "Fail to get local ip");
            return false;
        }
        LOG(LOG_DEBUG, "The local ip is [%s]", ip.c_str());
        m_main_data_url = string("http://") + ip +string(":9528/main_data/");
    }
    m_main_data_path = configer.Get("main_data_path");
    m_main_data_path += "/";
    if( ! CreatePath(m_main_data_path)) return false;
    m_cache_data_path = configer.Get("cache_data_path");
    m_cache_data_path += "/";
    if( ! CreatePath(m_cache_data_path)) return false;
    m_searcher_path = configer.Get("searcher_path");
    m_searcher_path += "/";

    m_data_handler_list = configer.GetList("data_handler_list");
    m_searcher_port = configer.Get("searcher_port");
    int s_port = atoi(m_searcher_port.c_str());
    if( s_port < 3000 || s_port > 65535) {
        LOG(LOG_WARN, "The port is abnormal [%d]", s_port);
    }

    string bak_file_num = configer.Get("bak_file_num");
    if ( bak_file_num.empty()) {
        m_bak_file_num = 3;
        LOG(LOG_INFO, "bak file num is not configured, here set default [%d]",
            m_bak_file_num);    
    } else {
        m_bak_file_num = atoi(bak_file_num.c_str());
    }

    if ( m_bak_file_num < 1 ) {
        LOG(LOG_ERROR, "Abnormal bak file number [%d]", m_bak_file_num);
        return false;
    }

    vector<string> mailer_list = configer.GetList("mailer_list");
    for(size_t i=0;i<mailer_list.size();++i) {
        m_mailer.AddReceiver(mailer_list[i]);
    }
    return true;
}

bool Dispatcher::_InitLogger(const string& config) {
    Configer configer;
    if( !configer.Load(config)) {
        LOG(LOG_ERROR, "Fail to load config [%s]", config.c_str());
        return false;
    }
    string log_name = configer.Get("log_name");
    if("" != log_name) LOG_NAME = log_name;
    string log_path = configer.Get("log_path");
    string log_level = configer.Get("log_level");
    Logger* log_ptr = Logger::GetLogger(LOG_NAME);
    log_ptr->SetHandler(log_path + string("/") + LOG_NAME + string(".log"));
    if("LOG_DEBUG" == log_level)
        log_ptr->SetLevel(LOG_DEBUG);
    else if("LOG_INFO" == log_level)
        log_ptr->SetLevel(LOG_INFO);
    else if("LOG_WARN" == log_level)
        log_ptr->SetLevel(LOG_WARN);
    else if("LOG_ERROR" == log_level)
        log_ptr->SetLevel(LOG_ERROR);
    else if("LOG_FATAL" == log_level)
        log_ptr->SetLevel(LOG_FATAL);
    return true;
}

bool Dispatcher::_AddModuleData(const string& module, 
    const string& data_path) {
    /// cost more than 1 mins should give a warning
    timer_counter tc(module + string(" add module data"), LOG_WARN, 60000);
    /// copy module data to searcher work dir
    /// add the module data to data path, something like .so file, etc.
    string module_data_path = string("./modules/") + module
        + string("/module_data/");
    if( GetFileNum(module_data_path) > 0) {
        string copy_cmd = string("cp -rf ") + module_data_path + string("* ")
            + data_path;
        int ret = system(copy_cmd.c_str());
        if( 0 != ret) {
            LOG(LOG_ERROR, "Fail to copy module data to path [%s]",
                copy_cmd.c_str());
            return false;
        }
    }// else do nothing
    /// create full time stamp file
    time_t cur_time_stamp;
    time(&cur_time_stamp);
    int main_time_stamp = cur_time_stamp;
    string stamp_file = data_path + string("/_full_timestamp");
    if( ! _WriteTimeStamp(stamp_file, main_time_stamp)) {
        LOG(LOG_ERROR, "Fail to write full time stamp to file [%s]",
            stamp_file.c_str());
        return false;
    }
    return true;
}

bool Dispatcher::_CheckSearcher(const string& module,
    const string& data_path) {
    /// cost more than 10 mins should give a warning
    timer_counter tc(module + string(" check searcher"), LOG_WARN, 600000);
    bool check_ret = true;
    /// check if the searcher pass the test or not
    string dst_data_path = m_searcher_path + string("modules/") + module;
    if(module == "searcher") {
        /// the special main data
        dst_data_path = m_searcher_path + "index";
    }
    string update_cmd = string("rm -rf ") + dst_data_path + string(";cp -rf ")
        + data_path + string(" ") + dst_data_path;
    int ret = system(update_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to update data, cmd [%s]", update_cmd.c_str());
        return false;
    }
    /// need mutex lock
    pthread_mutex_lock(&m_lock);
    /// run the searcher and check the result is ok or not
    string run_cmd = string("cd ") + m_searcher_path 
        + string(";Searcher -n 4 -p ") + m_searcher_port 
        + string(" -k start;cd -;");
    ret = system(run_cmd.c_str());
    if( 0 != ret ) {
        LOG(LOG_ERROR, "Fail to run Searcher [%s] by module [%s], ret [%d]",
            run_cmd.c_str(), module.c_str(), ret);
        pthread_mutex_unlock(&m_lock);
        return false;
    }
    int tm = 0;
    if ( ! _ReadTimeStamp(data_path + string("/_full_timestamp"), tm)) {
        LOG(LOG_ERROR, "Fail to read time stamp");
        pthread_mutex_unlock(&m_lock);
        return false;
    }
    string check_cmd = string("python checker.py -m ") + module
        + string(" -f ") + lexical_cast<string>(tm)
        + string(" -p ") + m_searcher_port;
    ret = system(check_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to pass the test of module [%s], error [%d]",
            module.c_str(), ret);
        check_ret = false;
    }
    string stop_cmd = string("cd ") + m_searcher_path
        + string(";Searcher -n 4 -p ") + m_searcher_port 
        + string(" -k stop;cd -;");
    ret = system(stop_cmd.c_str());
    if( 0 != ret ) {
        LOG(LOG_ERROR, "Fail to sop the Searcher by module [%s], ret [%d]",
            module.c_str(), ret);
        pthread_mutex_unlock(&m_lock);
        return false;
    }
    pthread_mutex_unlock(&m_lock);
    return check_ret;
}

bool Dispatcher::_UpdateData(const string& module, const string& data_path){
    /// cost more than 5 mins should give a warning
    timer_counter tc(module + string(" update data"), LOG_WARN, 300000);
    /// move the module data to target path and notify the clients
    if( !data_path.empty()) {
        string update_time = GetCurrentTime("%Y%m%d%H%M%S");
        string module_path = module + string("/main_data_") 
            + update_time + string("/");        
        string dst_data_path = m_main_data_path + module_path;
        string cp_cmd = string("cp -rf ") + data_path + string(" ")
            + dst_data_path;
        int ret = system(cp_cmd.c_str());
        if( 0 != ret) {
            LOG(LOG_ERROR, "Fail to move data [%s]", cp_cmd.c_str());
            return false;
        }
        /// create md5 file
        _MD5Dir(dst_data_path);
        /// clean up the oldest data to keep lastest 3 data    
        if ( ! KeepLatest(m_main_data_path + module, m_bak_file_num) ) 
            return false;
        int tm = 0;
        if ( ! _ReadTimeStamp(data_path + string("/_full_timestamp"), tm)) {
            LOG(LOG_ERROR, "Fail to read time stamp");
            return false;
        }
        m_data_handlers[module].state_ptr->main_update_time = update_time;
        m_data_handlers[module].state_ptr->data_path = 
            module_path + string("/");
        /// update the main time stamp
        m_data_handlers[module].state_ptr->main_stamp = tm;
    }else{
        LOG(LOG_INFO, "data path is empty of module [%s]",
            module.c_str());
    }
    return true;
}

void Dispatcher::_NotifyError(const std::string& err_info) {
    /// @todo notify the system manager an error occured
    LOG(LOG_ERROR, "Error occured: [%s]", err_info.c_str());
    string title = string("Dispatcher Error ") + GetCurrentTime();
    m_mailer.Send(title, err_info);
}

void Dispatcher::_MD5Dir(const std::string& data_path) {
    /// get the md5 value of files in the data path
    string md5_cmd = string("cd ") + data_path
        + string(";md5sum * > _md5;");
    int ret = system(md5_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to compute md5 value of [%s]", 
            data_path.c_str());
        return;
    }    
}

bool Dispatcher::_LoadState(const string& module) {
    HandlerStateInfoPtr state_ptr = m_data_handlers[module].state_ptr;
    string state_file = m_cache_data_path + string("/") + module
        + string(".state");
    Configer state_data;
    if( -1 == access(state_file.c_str(), F_OK)) {
        /// file no exist, init the state
        /// default as no update, which may be better
        state_data.Set("update_status", "NO_UPDATE");
        state_data.Set("data_path", "");
        state_data.Set("main_stamp", "-1");
        state_data.Set("main_update_time", "");
    }else if( !state_data.Load(state_file.c_str())) {
        LOG(LOG_ERROR, "Fail to load state from [%s]", state_file.c_str());
        return false;
    }        
    
    string update_status = state_data.Get("update_status");
    if("NO_UPDATE" == update_status) 
        state_ptr->status = NO_UPDATE;
    else if("NEED_PROCESS_MAIN" == update_status)
        state_ptr->status = NEED_PROCESS_MAIN;
    else if("NEED_PROCESS_INC" == update_status)
        state_ptr->status = NEED_PROCESS_INC;
    else if("NEED_RELOAD_MAIN" == update_status)
        state_ptr->status = NEED_RELOAD_MAIN;
    state_ptr->data_path = state_data.Get("data_path");
    state_ptr->main_stamp = lexical_cast<int>(state_data.Get("main_stamp"));
    state_ptr->main_update_time =  state_data.Get("main_update_time");
    return true;
}

bool Dispatcher::_SaveState(const string& module) {
    HandlerStateInfoPtr state_ptr = m_data_handlers[module].state_ptr;
    /// @todo need a lock
    string state_file = m_cache_data_path + string("/") + module
        + string(".state");
    Configer state_data;   
    /// set the state data into config
    switch(state_ptr->status){
    case NO_UPDATE:
        state_data.Set("update_status", "NO_UPDATE");
        break;
    case NEED_PROCESS_MAIN:
        state_data.Set("update_status", "NEED_PROCESS_MAIN");
        break;
    case NEED_PROCESS_INC:
        state_data.Set("update_status", "NEED_PROCESS_INC");
        break;
    case NEED_RELOAD_MAIN:
        state_data.Set("update_status", "NEED_RELOAD_MAIN");
        break;
    default:
        LOG(LOG_ERROR, "unknown status %d", state_ptr->status);
    }
    state_data.Set("data_path", state_ptr->data_path);
    state_data.Set("main_stamp", lexical_cast<string>(state_ptr->main_stamp));
    state_data.Set("main_update_time", state_ptr->main_update_time);
    /// save the config to back file
    string back_file = state_file + lexical_cast<string>(getpid()) 
        + string("_") + lexical_cast<string>(pthread_self()) + string("~");
    if( ! state_data.Save(back_file)) {
        LOG(LOG_ERROR, "Fail to write to file [%s]", back_file.c_str());
        return false;
    }
    /// cover the oral file with back file
    string mv_cmd = string("mv -f ") + back_file + string(" ") + state_file;
    int ret = system(mv_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to cover file [%s], ret [%d]",
            mv_cmd.c_str(), ret);
        return false;
    }
    return true;
}

void Dispatcher::_UpdateStatus(const std::string& module) {
    if(0 == m_data_handlers[module].dh_ptr->GetUpdateStatus())
        m_data_handlers[module].state_ptr->status = NEED_PROCESS_MAIN;
}

bool Dispatcher::_ReadTimeStamp(const string& file, int& tm) {
	FILE* fp = fopen(file.c_str(), "rb");
	if(NULL==fp) {
        LOG(LOG_ERROR, "Fail to open time stamp file [%s]", file.c_str());
        return false;
    }
	if((fread(&tm, sizeof(int), 1,fp)!=1)) {
        fclose(fp);
        return false;
    }
	fclose(fp);
	return true;
}

bool Dispatcher::_WriteTimeStamp(const string& file, int tm) {
    FILE* fp=fopen(file.c_str(),"wb");
    if(NULL == fp) {
        LOG(LOG_ERROR, "Fail to create time stamp file [%s]", file.c_str());
        return false;
    }
    fwrite(&tm,sizeof(int),1,fp);
    fclose(fp); 
    return true;
}

#ifdef UNIT_TEST
/// include head files for unit test
#include <iostream>
#include "configer.h"
using namespace std;


bool test_dispatcher() {
    cout<<"Unit test - dispatcher"<<endl;

    if(0)
    {
        bool ret = true;
        cout<<"usecase: dispatcher start data sender server"<<endl;
        Dispatcher dp;
        ret &= dp.Init("dispatcher.conf");
        ret &= dp.StartNetServer();
        sleep(120);
        ret &= dp.StopNetServer();
        ret &= dp.CoInit();
        cout<<boolalpha<<ret<<endl;
        return ret;
    }

    {
        bool ret = true;
        cout<<"usecase 1: start and stop a dispatcher"<<endl;
        Dispatcher dp;
        ret &= dp.Init("dispatcher.conf");
        ret &= dp.UpdateAll();
        ret &= dp.StartNetServer();
        while(true) {
            sleep(30);
        }
        ret &= dp.StopNetServer();
        ret &= dp.StopUpdateAll();
        ret &= dp.CoInit();
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }

    if(0)
    {
        bool ret = true;
        cout<<"usecase 2: remote control a dispatcher"<<endl;
        /*
        Dispatcher ob;
        string cmd = "start";
        ret &= ob.start();
        string control_cmd = "restart";
        ret &= http_send(control_cmd);
        string control_cmd = "stop";
        ret &= http_send(control_cmd);
        ret &= ob.is_stopped();
        */
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }

    {
        
    }
    cout<<"Done unit test - Dispatcher"<<endl;
    return true;
}

#ifdef TEST_DISPATCHER
int main() {
    if(!test_dispatcher())
        return -1;
    return 0;
}
#endif // TEST_DISPATCHER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
