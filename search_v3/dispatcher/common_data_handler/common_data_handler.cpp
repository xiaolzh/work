#include "common_data_handler.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <boost/lexical_cast.hpp>
#include "configer.h"
#include "logger.h"
#include "utilities.h"
#include "global_define.h"
#include "timer.h"
using namespace std;
using boost::lexical_cast;

static const char* STATE_FILE = "state.data";

CommonDataHandler::CommonDataHandler() : m_cur_state_ptr(NULL), 
    m_bak_state_ptr(NULL) {
    _Empty();
}

CommonDataHandler::~CommonDataHandler() {
    _Empty();
}

bool CommonDataHandler::Init(const string& config) {
    LOG(LOG_DEBUG, "Initialize from config");
    _Empty();
    string config_path = config + string("/") + string(CONFIG_FILE);
    if( !_LoadConfig(config_path)) {
        LOG(LOG_ERROR, "Fail to load config [%s]", config_path.c_str());
        return false;
    }
    string state_file = m_cache_data_path + string("/") + string(STATE_FILE);
    if( !_LoadState(state_file)) {
        LOG(LOG_ERROR, "Fail to load state [%s]", state_file.c_str());
        return false;
    }
    return true;
}

bool CommonDataHandler::CoInit() {
    LOG(LOG_DEBUG, "Counter initialize from config");
    _Empty();
    return true;
}

bool CommonDataHandler::ProcessData(const string& config) {
    LOG(LOG_DEBUG, "process search data...[%s]", config.c_str());
    string process_type = config;
    /// make sure the function is state-independent
    string state_file = m_cache_data_path + string("/") + string(STATE_FILE);
    if( !_LoadState(state_file)) {
        LOG(LOG_ERROR, "Fail to load state [%s]", state_file.c_str());
        return false;
    }
    if("main" == process_type) {
        timer_counter tc("process main");
        /// update state
        m_cur_state_ptr->need_main = false;
        time_t cur_time_stamp;
        time(&cur_time_stamp);
        m_cur_state_ptr->main_time_stamp = cur_time_stamp;
        m_cur_state_ptr->main_update_time = GetCurrentTime();
        m_cur_state_ptr->inc_time_stamp = 0;
        m_cur_state_ptr->inc_update_time = m_cur_state_ptr->main_update_time;

        int ret = system(m_main_process_cmd.c_str());
        if( 0 != ret) {
            LOG(LOG_ERROR, "Fail to do main process error [%d]", ret);
            return false;
        }
    } else if("inc" == process_type) {
        int ret = system(m_inc_process_cmd.c_str());
        if( 0 != ret) {
            LOG(LOG_ERROR, "Fail to do inc process error [%d]", ret);
            return false;
        }
    }else {
        LOG(LOG_ERROR, "unknown process type: [%s]", process_type.c_str());
        return false;
    }
    m_cur_state_ptr->need_main = _NeedMain();
    m_cur_state_ptr->main_data_dir = m_main_data_path;
    if( !_SaveState(state_file)) {
        LOG(LOG_ERROR, "Fail to save state to [%s]", state_file.c_str());
        return false;
    }
    return true;
}

int CommonDataHandler::GetDataLen(int inc_stamp) {
    LOG(LOG_ERROR, "No implement yet for getting inc data len");
    return -1;
}

int CommonDataHandler::GetData(int inc_stamp, void* data_ptr) {
    /// @todo no need to do the work right now
    LOG(LOG_ERROR, "No implement yet for getting inc data");
    return false;
}

int CommonDataHandler::GetIncStamp() {
    if ( NULL == m_bak_state_ptr ) return -1;
    return m_bak_state_ptr->inc_time_stamp;
}

int CommonDataHandler::GetUpdateStatus() {
    if ( NULL == m_bak_state_ptr ) return 1; 
    if( m_bak_state_ptr->need_main) return 0;
    return 1;
}

bool CommonDataHandler::GetDataPath(string& data_path) {
    if( NULL == m_bak_state_ptr ) return false;
    data_path = m_bak_state_ptr->main_data_dir;
    return true;
}

void CommonDataHandler::_Empty() {
    if ( NULL != m_cur_state_ptr ) {
        delete m_cur_state_ptr;
        m_cur_state_ptr = NULL;
    }
    if ( NULL != m_bak_state_ptr ) {
        delete m_bak_state_ptr;
        m_bak_state_ptr = NULL;
    }

    m_main_data_path = "";
    m_cache_data_path = "";
    m_main_process_cmd = "";
    m_inc_process_cmd = "";
    m_main_update_interval = -1.0;
}

bool CommonDataHandler::_LoadConfig(const string& config_path) {
    Configer config;
    if( ! config.Load(config_path)) {
        LOG(LOG_ERROR, "Fail to get config from [%s]", config_path.c_str());
        return false;
    }
    /// get path of main data
    m_main_data_path = config.Get("main_data_path");
    if( m_main_data_path.empty()) {
        LOG(LOG_ERROR, "Fail to get main data path from config file");
        return false;
    }
    if( ! CreatePath(m_main_data_path) ) return false;
    /// get path of cache data
    m_cache_data_path = config.Get("cache_data_path");
    if( m_cache_data_path.empty()) {
        LOG(LOG_ERROR, "Fail to get cache data path from config file");
        return false;
    }
    if( ! CreatePath(m_cache_data_path) ) return false;
    /// get main process command
    m_main_process_cmd = config.Get("main_process_cmd");
    if( m_main_process_cmd.empty()) {
        LOG(LOG_ERROR, "Fail to get main process cmd from config file");
        return false;
    }
    /// get inc process command
    m_inc_process_cmd = config.Get("inc_process_cmd");
    if( m_inc_process_cmd.empty()) {
        LOG(LOG_INFO, "Inc process cmd is no set");
    }
    /// get main update interval
    string interval_str = config.Get("main_update_interval");
    if( interval_str.empty()) {
        m_main_update_interval = -1.0;
    } else {
        m_main_update_interval = atof(interval_str.c_str());
    }
    return true;
}

bool CommonDataHandler::_LoadState(const string& state_file) {
    Configer state_data;
    if( -1 == access(state_file.c_str(), F_OK)) {
        /// file no exist, init the state
        state_data.Set("need_main", "true");
        state_data.Set("main_time_stamp", "-1");
        state_data.Set("main_update_time", "");
        state_data.Set("main_data_dir", "");
        state_data.Set("inc_time_stamp", "0");
        state_data.Set("inc_update_time", "");
    }else if( !state_data.Load(state_file.c_str())) {
        LOG(LOG_ERROR, "Fail to load state from [%s]", state_file.c_str());
        return false;
    }
    if ( NULL == m_cur_state_ptr) {
        m_cur_state_ptr = new _StateInfo;
    }
    string str;
    str = state_data.Get("need_main");
    m_cur_state_ptr->need_main = ("false"==str)?false:true;
    str = state_data.Get("main_time_stamp");
    if( str.empty()) return false;
    m_cur_state_ptr->main_time_stamp = lexical_cast<int>(str);
    m_cur_state_ptr->main_update_time = state_data.Get("main_update_time");
    m_cur_state_ptr->main_data_dir = state_data.Get("main_data_dir");
    str = state_data.Get("inc_time_stamp");
    if( str.empty()) return false;
    m_cur_state_ptr->inc_time_stamp = lexical_cast<int>(str);
    m_cur_state_ptr->inc_update_time = state_data.Get("inc_update_time");
    /// back up the current legal state
    Backup<_StateInfo>(m_cur_state_ptr, m_bak_state_ptr);
    return true;
}

bool CommonDataHandler::_SaveState(const string& state_file) {
    Configer state_data;
    string need_main = m_cur_state_ptr->need_main?"true":"false";
    state_data.Set("need_main", need_main);
    state_data.Set("main_time_stamp", 
        lexical_cast<string>(m_cur_state_ptr->main_time_stamp));
    state_data.Set("main_update_time", 
        lexical_cast<string>(m_cur_state_ptr->main_update_time));
    state_data.Set("main_data_dir", m_cur_state_ptr->main_data_dir);
    state_data.Set("inc_time_stamp", 
        lexical_cast<string>(m_cur_state_ptr->inc_time_stamp));
    state_data.Set("inc_update_time", m_cur_state_ptr->inc_update_time);
    string back_file = state_file + lexical_cast<string>(getpid()) 
        + string("_") + lexical_cast<string>(pthread_self()) + string("~");
    if( ! state_data.Save(back_file)) {
        LOG(LOG_ERROR, "Fail to write to file [%s]", back_file.c_str());
        return false;
    }
    string mv_cmd = string("mv -f ") + back_file + string(" ") + state_file;
    int ret = system(mv_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to cover file [%s], ret [%d]",
            mv_cmd.c_str(), ret);
        return false;
    }
    /// back up the current legal state
    Backup<_StateInfo>(m_cur_state_ptr, m_bak_state_ptr);
    return true;
}

bool CommonDataHandler::_NeedMain() {
    if( m_main_update_interval < 0 ) return false; 
    time_t cur_time_stamp;
    time(&cur_time_stamp);
    /// transfer interval to second
    int interval = (int)(m_main_update_interval*3600);
    if((cur_time_stamp - m_cur_state_ptr->main_time_stamp) >= interval) {
        return true;
    }
    return false;
}

#ifdef UNIT_TEST
#include <iostream>
using namespace std;

string LOG_NAME = "common_handler.log";

bool test_CommonDataHandler() {
    cout<<"Unit test - CommonDataHandler"<<endl;
    {
        bool ret = true;
        cout<<"usecase: process data in main and get the state"<<endl;
        CommonDataHandler hd;
        ret &= hd.Init("init");
        ret &= hd.ProcessData("main");
        vector<char> info_buf;
        cout<<string(&info_buf[0])<<endl;
        ret &= hd.CoInit();
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    return true;
}

#ifdef TEST_COMMON_DATA_HANDLER
int main() {
    if( ! test_CommonDataHandler()) {
        cout<<"Fail to pass test"<<endl;
        return -1;
    }
    return 0;
}
#endif // TEST_COMMON_DATA_HANDLER

#endif // UNIT_TEST
// I wanna sing a song for you, dead loop no escape
