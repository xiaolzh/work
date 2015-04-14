#include "state_info.h"
#include <unistd.h>
#include <pthread.h>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "configer.h"
using namespace std;
using boost::lexical_cast;

bool StateInfo::LoadState(const string& state_file, 
    const vector<string>& table_names) {
    Configer state_data;
    if( -1 == access(state_file.c_str(), F_OK)) {
        /// file no exist, init the state
        state_data.Set("need_main", "true");
        state_data.Set("main_time_stamp", "-1");
        state_data.Set("main_update_time", "");
        state_data.Set("main_data_dir", "");
        state_data.Set("inc_time_stamp", "0");
        state_data.Set("inc_modify_num", "0");
        state_data.Set("inc_add_num", "0");
        state_data.Set("inc_delete_num", "0");
        state_data.Set("inc_recover_num", "0");
        state_data.Set("new_product_num", "0");
        state_data.Set("total_data_num", "0");
        for(int i=0;i<table_names.size();++i) {
            state_data.Set("table_name", table_names[i], table_names[i]);
            state_data.Set("inc_update_time", "", table_names[i]); 
            /// default no inc update, so status is 0
            state_data.Set("process_status", "0", table_names[i]); 
        } 
    }else if( !state_data.Load(state_file.c_str())) {
        LOG(LOG_ERROR, "Fail to load state from [%s]", state_file.c_str());
        return false;
    } 
    /// get state infos       
    string str;
    str = state_data.Get("need_main");
    need_main = ("false"==str)?false:true;
    str = state_data.Get("main_time_stamp");
    if( str.empty()) return false;
    main_time_stamp = atoi(str.c_str());
    main_update_time = state_data.Get("main_update_time");
    main_data_dir = state_data.Get("main_data_dir");
    str = state_data.Get("inc_time_stamp");
    if( str.empty()) return false;
    inc_time_stamp = atoi(str.c_str());
    str = state_data.Get("inc_modify_num");
    if( str.empty()) return false;
    inc_modify_num = atoi(str.c_str());
    str = state_data.Get("inc_add_num");
    if( str.empty()) return false;
    inc_add_num = atoi(str.c_str());
    str = state_data.Get("inc_delete_num");
    if( str.empty()) return false;
    inc_delete_num = atoi(str.c_str());
    str = state_data.Get("inc_recover_num");
    if( str.empty()) return false;
    inc_recover_num = atoi(str.c_str());
    str = state_data.Get("new_product_num");
    if( str.empty()) return false;
    new_product_num = atoi(str.c_str());
    str = state_data.Get("total_data_num");
    if( str.empty()) return false;
    total_data_num = atoi(str.c_str());
    vector<string> table_list = state_data.GetSections();
    for(size_t i=0; i<table_list.size();++i) {
        if( ! table_list[i].empty()) {
            tables[table_list[i]].table = 
                state_data.Get("table_name", table_list[i]);
            tables[table_list[i]].inc_update_time = 
                state_data.Get("inc_update_time", table_list[i]);
            tables[table_list[i]].status = 
                atoi(state_data.Get("process_status", table_list[i]).c_str());
        }
    }
    return true;
}

bool StateInfo::SaveState(const string& state_file) {
    Configer state_data;
    string str_need_main = need_main?"true":"false";
    state_data.Set("need_main", str_need_main);
    state_data.Set("main_time_stamp", lexical_cast<string>(main_time_stamp));
    state_data.Set("main_update_time", 
        lexical_cast<string>(main_update_time));
    state_data.Set("main_data_dir", main_data_dir);
    state_data.Set("inc_time_stamp", lexical_cast<string>(inc_time_stamp));
    state_data.Set("inc_modify_num", lexical_cast<string>(inc_modify_num));
    state_data.Set("inc_add_num", lexical_cast<string>(inc_add_num));
    state_data.Set("inc_delete_num", lexical_cast<string>(inc_delete_num));
    state_data.Set("inc_recover_num", lexical_cast<string>(inc_recover_num));
    state_data.Set("new_product_num", lexical_cast<string>(new_product_num));
    state_data.Set("total_data_num", lexical_cast<string>(total_data_num));
    typedef map<string, TableStateInfo>::iterator Itor;
    for(Itor it=tables.begin(); it != tables.end();++it) {
        state_data.Set("table_name", it->second.table, it->first);
        state_data.Set("inc_update_time", it->second.inc_update_time,
            it->first);
        state_data.Set("process_status", 
            lexical_cast<string>(it->second.status), it->first);
    }
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
    return true;
}

// I wanna sing a song for you, dead loop no escape
