#include "net_server.h"
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "http_server.h"
#include "dispatcher.h"
#include "configer.h"
#include "utilities.h"
#include "global_define.h"
#include "timer.h"
using namespace std;
using boost::lexical_cast;

static const int RESP_MAX_SIZE = 102400;
static const int RESP_MAX_NUM = 1000;
static const char* HEADER = "//Header";
static const char* BODY = "//Body";
static const char* DELIMITER = "\n";

/// NetServer construction
NetServer::NetServer() 
    : m_server_ptr(NULL) 
    , m_dispatcher_ptr(NULL)
    , m_port(0)
    , m_thread_num(0) {
}

/// NetServer destruction
NetServer::~NetServer() {
    if( NULL != m_server_ptr) {
        delete m_server_ptr;
        m_server_ptr = NULL;
    }
}

bool NetServer::Work(const vector<char>& request, vector<char>& response){
    string req_str = string(&request[0]);
    /// cost more than 1 s should give a warning
    timer_counter tc(req_str, LOG_WARN, 1000);
    if( "/?help" == TrimBoth(req_str).substr(0, 6)) 
        return _Help(req_str, response);
    string control_type = ParseUrl(req_str, "control");
    if( "" != control_type) return _ControlWork(req_str, response);
    return _DataWork(req_str, response);
}

bool NetServer::Init(const string& config, Dispatcher* dispatcher_ptr) {
    m_dispatcher_ptr = dispatcher_ptr;
    Configer configer;
    if( ! configer.Load(config)) {
        LOG(LOG_ERROR, "Fail to load config [%s]", config.c_str());
        return false;
    }
    string port_str = configer.Get("data_server_port");
    m_port = atol(port_str.c_str());
    if(0 == m_port) {
        LOG(LOG_ERROR, "Port is ellegal: [%s]", port_str.c_str());
        return false;
    }
    //int time_out = -1;
    string thrd_num_str = configer.Get("data_server_thread_num");
    m_thread_num = atol(thrd_num_str.c_str());
    if(0 == m_thread_num) {
        LOG(LOG_ERROR, "Thread number is ellegal:[%s]", thrd_num_str.c_str());
        return false;
    } 
    return true;
}

void NetServer::CoInit() {
    /// @todo counter initialize
}

bool NetServer::StartServer(const string& config) {
    if(NULL == m_server_ptr) {
        m_server_ptr = new HttpServer();
    }
    if( ! m_server_ptr->Create(m_port, m_thread_num, this)) {
        LOG(LOG_ERROR, "Fail to create data server");
        return false;
    }
    /// @todo use thread as a class
    pthread_create(&m_server_thread, NULL, _Run, m_server_ptr);
    return true;
}

bool NetServer::StopServer() {
    if ( NULL != m_server_ptr) {
        m_server_ptr->Close();
    }else{
        LOG(LOG_WARN, "No server to close");
        return false;
    }
    return true;
}

void NetServer::WaitForQuit() {
    void* ret;
    pthread_join(m_server_thread, &ret);
    LOG(LOG_DEBUG, "data server thread %d exit with code %d", m_server_thread,
        (long)ret);
}

bool NetServer::_DataWork(const string& req_str, vector<char>& response) {
    string module_type = ParseUrl(req_str, "module");
    string data_key = ParseUrl(req_str, "key");
    if ( ! data_key.empty() ) {
        /// get full data to display
        return _GetFullData(module_type, data_key, response);
    }
    string main_stamp_str = ParseUrl(req_str, "main_stamp");
    string inc_stamp_str = ParseUrl(req_str, "inc_stamp"); 
    int main_stamp = -1;
    if(  ! main_stamp_str.empty()) {
        main_stamp = atol(main_stamp_str.c_str());
    }
    int inc_stamp = -1;
    /// @todo here should not use atol, which returns 0 if ellegal
    if( ! inc_stamp_str.empty()) {
        inc_stamp = atol(inc_stamp_str.c_str());
    }
    int data_size = 0;
    QueryStatus ret = UNDEFINED;    
    vector<char> body;
    body.reserve(RESP_MAX_SIZE);
    AddPairToVec(BODY, " ", body);
    AddPairToVec(DELIMITER, " ", body);
    int inc_num = 0;
    while(data_size < RESP_MAX_SIZE && inc_num < RESP_MAX_NUM) {
        vector<char> data_buf;
        ret = (QueryStatus)m_dispatcher_ptr->GetIncData(module_type, 
            main_stamp, inc_stamp++, data_buf);
        copy(data_buf.begin(), data_buf.end(), back_inserter(body));
        AddPairToVec(DELIMITER, " ", body);
        if( OK != ret) break;
        inc_num ++;
        data_size = body.size();
    }
    if(inc_num > 0) ret = OK;
    if( !_PackHeader(inc_num, data_size, ret, response)) {
        LOG(LOG_ERROR, "Fail to pack header");
        return false;
    }
    copy(body.begin(), body.end(), back_inserter(response));
    return true;
}


bool NetServer::_ControlWork(const string& req_str, vector<char>& response){
    string control_type = ParseUrl(req_str, "control");
    if("continue_update" == control_type) {
        bool ret = true;
        string module = ParseUrl(req_str, "module");
        if( module == "" || module == "all") {
            ret = m_dispatcher_ptr->ControlUpdateAll(NEED_PROCESS_INC);
        } else {
            ret = m_dispatcher_ptr->ControlUpdate(module, NEED_PROCESS_INC);
        }
        LOG(LOG_INFO, "continue update data...[%d]", ret);
        string status = "OK";
        if(!ret) status = "Failed";
        AddPairToVec("status", "cotinue update data..."+status, response);    
    }else if("get_info" == control_type){
        vector<char> info_buf;
        m_dispatcher_ptr->GetInfo("info", info_buf);
        LOG(LOG_INFO, "get dispatcher info...");
        copy(info_buf.begin(), info_buf.end(), back_inserter(response));
    }else if("pause_update" == control_type) {
        bool ret = true;
        string module = ParseUrl(req_str, "module");
        if( module == "" || module == "all") {
            ret = m_dispatcher_ptr->ControlUpdateAll(NO_UPDATE);
        } else {
            ret = m_dispatcher_ptr->ControlUpdate(module, NO_UPDATE);
        }
        LOG(LOG_INFO, "pause update data...[%d]", ret);
        string status = "OK";
        if(!ret) status = "Failed";
        AddPairToVec("status", "pause update data..."+status, response);    
    }else if("reload_main" == control_type) {
        bool ret = true;
        string module = ParseUrl(req_str, "module");
        if( module == "" || module == "all") {
            ret = m_dispatcher_ptr->ControlUpdateAll(NEED_RELOAD_MAIN);
        } else {
            ret = m_dispatcher_ptr->ControlUpdate(module, NEED_RELOAD_MAIN);
        }
        LOG(LOG_INFO, "reload main data...[%d]", ret);
        string status = "OK";
        if(!ret) status = "Failed";
        AddPairToVec("status", "reload main data..."+status, response);    
    }else if("process_main" == control_type) {
        bool ret = true;
        string module = ParseUrl(req_str, "module");
        if( module == "" || module == "all") {
            ret = m_dispatcher_ptr->ControlUpdateAll(NEED_PROCESS_MAIN);
        } else {
            ret = m_dispatcher_ptr->ControlUpdate(module, NEED_PROCESS_MAIN);
        }
        LOG(LOG_INFO, "process main data...[%d]", ret);
        string status = "OK";
        if(!ret) status = "Failed";
        AddPairToVec("status", "process main data..."+status, response);    
    }else if("stop_update" == control_type){
        bool ret = true;
        string module = ParseUrl(req_str, "module");
        if( module == "" || module == "all") {
            ret = m_dispatcher_ptr->StopUpdateAll();
        } else {
            ret = m_dispatcher_ptr->StopUpdate(module);
        }
        LOG(LOG_INFO, "stop update data...[%d]", ret);
        string status = "OK";
        if(!ret) status = "Failed";
        AddPairToVec("status", "stop update data..."+status, response);    
    }else if("restart_update" == control_type){
        bool ret = true;
        string module = ParseUrl(req_str, "module");
        if( module == "" || module == "all" ) {
            ret &= m_dispatcher_ptr->StopUpdateAll();
            ret &= m_dispatcher_ptr->UpdateAll();
        } else {
            ret &= m_dispatcher_ptr->StopUpdate(module);
            ret &= m_dispatcher_ptr->UpdateData(module);
        }
        LOG(LOG_INFO, "restart update data...[%d]", ret);
        string status = "OK";
        if(!ret) status = "Failed";
        AddPairToVec("status", "restart update data..."+status, response);    
    }else{
        LOG(LOG_ERROR, "unknown control type: [%s]", control_type.c_str());
    }
    LOG(LOG_DEBUG, "Make a response here to [%s]", &req_str[0]);
    return true;
}

bool NetServer::_Help(const string& request, vector<char>& response) {
    static char* help[] = {
        /// with no passed in value
        "useage:\n"
        "  http://host:port/?param1=value1&param2=value2&...\n\n"
        "list of params:\n"
        "  control      control type with a list control cmd\n"
        "  help         show help for a given topic or a help overview\n"
        "  inc_stamp    inc_stamp of the data, an increase 4 bytes integer\n"
        "  main_stamp   full time stamp of the data, an 4 bytes integer\n"
        "  module       module name, include searcher, list_ranking, etc.\n"
        "  key          the full data key with which the data is stored.\n"
        , 
        /// with cotrol
        "useage:\n"
        "  http://host:port/?control=params&module=\n\n"
        "list of params:\n"
        "  continue_update      continue update of the data module\n"
        "  get_info             get information of dispatcher and its \n"
        "                       modules\n"
        "  pause_update         pause update of the data module\n"
        "  reload_main          reload main data of the data module\n"
        "  restart_update       restart update of the data module\n"
        "  stop_update          stop update of the data module\n"
        , 
        /// with help
        "useage:\n"
        "  http://host:port/?help=[params]\n\n"
        "list of params:\n"
        "  control        show help of the param [control]\n"
        "  help           show help of the param [help]\n"
        "  inc_stamp      show help of the param [inc_stamp]\n"
        "  main_stamp     show help of the param [main_stamp]\n"
        "  module         show help of the param [module]\n"
        ,
        /// with inc_stamp
        "useage:\n"
        "  http://host:port/?module=searcher&main_stamp=&inc_stamp=\n\n"
        "list of params:\n"
        "  1~999999     an 4 bytes integer, which usually start from 1\n"
        ,
        /// with key
        "useage:\n"
        "  http://host:port/?module=searcher&key=123\n\n"
        "list of params:\n"
        "  1~999999     an 8 bytes integer, which is the key of value stored.\n"
        ,
        /// with main_stamp
        "useage:\n"
        "  http://host:port/?module=&main_stamp=\n\n"
        "list of params:\n"
        "  1~999999     an 4 bytes integer, which is a time stamp in linux\n"
        ,
        /// with module
        "useage:\n"
        "  http://host:port/?module=&main_stamp=\n\n"
        "list of params:\n"
        "  searcher, query_pack, key_ranking, the name of data modules\n"
        };
    string help_val = ParseUrl(request, "help");
    string help_info;
    if(help_val.empty()) {
        help_info = help[0];
    }else if("control" == help_val) {
        help_info = help[1];
    }else if("help" == help_val) {
        help_info = help[2];
    }else if("inc_stamp" == help_val) {
        help_info = help[3];
    }else if("key" == help_val) {
        help_info = help[4];
    }else if("main_stamp" == help_val) {
        help_info = help[5];
    }else if("module" == help_val) {
        help_info = help[6];
    }
    AddStrToVec(help_info, response);
    return true;
}

bool NetServer::_PackHeader(int inc_num, int data_sz, int status, 
    vector<char>& resp) {
    AddPairToVec(HEADER, " ", resp);
    AddPairToVec(DELIMITER, " ", resp);
    AddPairToVec("@status code", lexical_cast<string>(status), resp);
    AddPairToVec("@status info", GetStatus(status), resp);
    AddPairToVec("@inc number", lexical_cast<string>(inc_num), resp);
    AddPairToVec("@total size", lexical_cast<string>(data_sz), resp);
    AddPairToVec(DELIMITER, " ", resp);
    return true;
}

bool NetServer::_GetFullData( const string& module, const string& key,
                              vector<char>& response ) {
    /// get full data to display
    if ( ! m_dispatcher_ptr->GetFullData(module, key, response) ) {
        LOG(LOG_INFO, "Fail to get the data of key [%s]", key.c_str());
        response.clear();
        string err_ret = string("Can not get the data info of key ")
            + key;
        AddStrToVec(err_ret, response);
    }
    /// @mark always return true
    return true;
}

void* NetServer::_Run(void* arg) {
    HttpServer* server_ptr = (HttpServer*)arg;
    server_ptr->Run();
    return NULL;
}


#ifdef UNIT_TEST
/// include head files for unit test
#include "iostream"
using namespace std;

bool test_net_server() {
    cout<<"Unit test - net_server"<<endl;
    {
        bool ret = true;
        cout<<"usecase 1: "<<endl;
        // TODO: add your test code here
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - NetServer"<<endl;
    return true;
}

#ifdef TEST_NET_SERVER
int main() {
    if(!test_net_server())
        return -1;
    return 0;
}
#endif // TEST_NET_SERVER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
