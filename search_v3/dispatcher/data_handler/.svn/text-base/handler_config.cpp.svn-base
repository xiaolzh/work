#include "handler_config.h"
#include <unistd.h>
#include <sys/stat.h>
#include "configer.h"
#include "logger.h"
#include "utilities.h"

using namespace std;

void TableConfig::Empty() {
    table = "";
    num_fields.clear();
    num_field_lens.clear();
    str_fields.clear();
    timestamp_fields.clear();
    pre_read = 0;
    mark_num_fields.clear();
    mark_str_fields.clear();
    offset = -1;
    flag_offset = -1;
}

void HandlerConfig::Empty() {
    main_data_path = "";
    raw_data_path = "";
    cache_data_path = "";
    read_cmd = "";
    search_view = "";
    sql_tmpl = "";
    index_cmd = "";
    table_names.clear();
    db_tables.clear();
    key_field = "";
    main_update_threshold = 100;
    force_update_threshold = -1;
    main_num_lower = 90;
    main_num_higher = 120;
    main_update_start_time = 0;
    main_update_stop_time = 0;
    main_update_interval = 0;
}

bool HandlerConfig::LoadConfig(const string& config_path) {
    Configer configer;
    if( ! configer.Load(config_path)) {
        LOG(LOG_ERROR, "Fail to get config from [%s]", config_path.c_str());
        return false;
    }
    /// get path of main data
    main_data_path = configer.Get("main_data_path");
    if( main_data_path.empty()) {
        LOG(LOG_ERROR, "Fail to get main_data_path from config file");
        return false;
    }
    if( ! CreatePath(main_data_path) ) return false;
    main_data_path += "/";
    /// get path of raw data read from database
    raw_data_path = configer.Get("raw_data_path");
    if( raw_data_path.empty()) {
        LOG(LOG_ERROR, "Fail to get raw_data_path from config file");
        return false;
    }
    if( ! CreatePath(raw_data_path) ) return false;
    raw_data_path += "/";
    /// get path of cache data mapping from memory
    cache_data_path = configer.Get("cache_data_path");
    if( cache_data_path.empty()) {
        LOG(LOG_ERROR, "Fail to get cache_data_path from config file");
        return false;
    }
    if( ! CreatePath(cache_data_path) ) return false;
    cache_data_path += "/";
    /// get main update threshold
    string str = configer.Get("main_update_threshold");
    if( str.empty()) {
        LOG(LOG_ERROR, "Fail to get main_update_threshold form config file");
        return false;
    }
    main_update_threshold = atoi(str.c_str());
    /// get force update threshold
    str = configer.Get("force_update_threshold");
    if ( str.empty()) {
        LOG(LOG_INFO, "No force update threshold is set");
        force_update_threshold = -1;
    } else {
        force_update_threshold = atoi(str.c_str());
    }
    /// get main update number lower limit
    main_num_lower = atoi(configer.Get("main_num_lower").c_str());
    if( main_num_lower < 50 || main_num_lower > 100 )
        LOG(LOG_WARN, "Abnormal num percent [%d]", main_num_lower);
    /// get main update number higher limit
    main_num_higher = atoi(configer.Get("main_num_higher").c_str());
    if( main_num_higher > 200 || main_num_higher < 100 )
        LOG(LOG_WARN, "Abnormal num percent [%d]", main_num_higher);
    /// get command of reader for data reading
    read_cmd = configer.Get("read_cmd"); 
    if( read_cmd.empty()) {
        LOG(LOG_ERROR, "Fail to get read_cmd from config file");
        return false;
    }
    /// get main read view
    search_view = configer.Get("search_view");
    if( search_view.empty()) {
        LOG(LOG_ERROR, "Fail to get search_view from config file");
        return false;
    }
    /// get sql query template
    sql_tmpl = configer.Get("sql_template");
    if( sql_tmpl.empty() ) {
        LOG(LOG_ERROR, "Illegal sql_template [%s] from config file",
            sql_tmpl.c_str());
        return false;
    }
    /// get command of indexer for main data
    index_cmd = configer.Get("index_cmd");       
    if( index_cmd.empty()) {
        LOG(LOG_ERROR, "Fail to get index_cmd from config file");
        return false;
    }
    /// get database tables to inc read
    table_names = configer.GetList("db_tables");
    if( table_names.empty()) {
        LOG(LOG_ERROR, "Fail to get db_tables from config file");
        return false;
    }
    db_tables.resize(table_names.size());
    for(size_t i=0; i < db_tables.size(); ++i) {
        if( configer.GetKeys(table_names[i]).empty() ) {
            LOG(LOG_ERROR, "Illegal db table [%s]", table_names[i].c_str());
            return false;
        }
        db_tables[i].table = table_names[i];
        vector<string> num_fields = configer.GetList("number_fields", 
            table_names[i]);
        for(size_t j=0; j < num_fields.size(); ++j) {
            vector<string> str_vec = Split(num_fields[j], ":");
            if( str_vec.size() != 2) {
                LOG(LOG_ERROR, "Illegal style of number field [%s]",
                    num_fields[j].c_str());
                return false;
            }
            db_tables[i].num_fields.push_back(str_vec[0]);
            int sz = atoi(str_vec[1].c_str());
            if(0 == sz) {
                LOG(LOG_ERROR, "Illegal style of number field [%s]",
                    num_fields[j].c_str());
                return false;
            }
            db_tables[i].num_field_lens.push_back(sz);
        }
        db_tables[i].str_fields = configer.GetList("string_fields",
            table_names[i]);
        db_tables[i].timestamp_fields = configer.GetList("timestamp_fields",
            table_names[i]);
        string pre_read_str = configer.Get("pre_read", table_names[i]);
        db_tables[i].pre_read = atoi(pre_read_str.c_str());
    }
    /// get key field name
    key_field = configer.Get("key_field");
    if( key_field.empty()) {
        LOG(LOG_ERROR, "Fail to get key_field from config file");
        return false;
    }
    
    /// get main update start_time and stop_time
    string str_main_update_time=configer.Get("main_update_time");
    string::size_type pos = str_main_update_time.find("-");
    if( pos == std::string::npos) {
        LOG(LOG_ERROR,"Fail to get main_update_time from config file");
        return false;
    }

    string start_time = TrimBoth(str_main_update_time.substr(0,pos));
    string stop_time = TrimBoth(str_main_update_time.substr(pos+1));

    main_update_start_time = atoi(start_time.c_str());
    main_update_stop_time = atoi(stop_time.c_str());

    /// get main update interval time
    string str_main_update_interval = configer.Get("main_update_interval");
    string::size_type itv_pos;
    if ( string::npos != (itv_pos = str_main_update_interval.find("h")))
        main_update_interval = 
            atoi(str_main_update_interval.substr(0,itv_pos).c_str());
    else if(string::npos != (itv_pos=str_main_update_interval.find("d")))
        main_update_interval = 
            24*(atoi(str_main_update_interval.substr(0,itv_pos).c_str()));
    else if(string::npos != (itv_pos=str_main_update_interval.find("w")))
        main_update_interval = 
            7*24*(atoi(str_main_update_interval.substr(0,itv_pos).c_str()));
    else {
        LOG(LOG_ERROR,"Fail to get main_update_interval from config file");
        return false;
    }

    string bak_file_num_str = configer.Get("bak_file_num");
    if ( bak_file_num_str.empty()) {
        bak_file_num = 3;
        LOG(LOG_INFO, "bakup files is not configured, set it default [%d]",
            bak_file_num);
    } else {
        bak_file_num = atoi(bak_file_num_str.c_str());
    }

    if ( bak_file_num < 1 ) {
        LOG(LOG_ERROR, "Abnormal bak file number [%d]", bak_file_num);
        return false;
    }

    return true;
}

// I wanna sing a song for you, dead loop no escape
