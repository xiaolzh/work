#include "search_data_handler.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "logger.h"
#include "timer.h"
#include "utilities.h"
using namespace std;
using boost::lexical_cast;
using boost::shared_ptr;

static const unsigned int VALUE_SIZE = 4096;
static const char* STATE_FILE = "state.data";
static const char* DATA_PATH = "./data";

enum DataStatus {
    ST_REJECT = 0,       /// data is rejected, which means never be ok
    ST_DELETE = 1,       /// data is deleted, which means used to be ok
    ST_OK = 2            /// data is ok, which is ok yet
};

/// SearchDataHandler construction
SearchDataHandler::SearchDataHandler() : m_cur_state_ptr(NULL),
    m_stable_state_ptr(NULL) {
    _Empty();
}

/// SearchDataHandler destruction
SearchDataHandler::~SearchDataHandler() {
    _Empty();
}

bool SearchDataHandler::Init(const string& config) {
    LOG(LOG_DEBUG, "Initialize from config", config.c_str());
    _Empty();
    m_module_path = config + string("/");
    string config_path = m_module_path + string(CONFIG_FILE);
    if( ! m_config.LoadConfig(config_path)) {
        LOG(LOG_ERROR, "Fail to initialize config from [%s]", 
            config_path.c_str());
        return false;
    }
    /// initialize the data size from config
    if( ! _AutoSize()) {
        LOG(LOG_ERROR, "Fail to auto size");
        return false;
    }
    if( ! _MapField()) {
        LOG(LOG_ERROR, "Fail to map field");
        return false;
    }
    /// initialize the data table
    if ( ! _InitDataTable(m_config.cache_data_path) ) 
        return false;
    /// initialize the dictionary for inc data
    string inc_idx_file = m_config.cache_data_path + "inc_data.idx";
    string inc_ivt_file = m_config.cache_data_path + "inc_data.ivt";
    const int inc_page_size = 8192;
    if( ! m_inc_data_ptr->LoadDetail(inc_idx_file.c_str(), 
        inc_ivt_file.c_str(), inc_page_size) ) {
        LOG(LOG_ERROR, "Fail to initialize the inc data");
        return false;
    }
    /// initialize the state    
    string state_file = m_config.cache_data_path + string(STATE_FILE);
    if( ! _LoadState(state_file)) {
        LOG(LOG_ERROR, "Fail to initialize state from [%s]", 
            state_file.c_str());
        return false;
    }
    /// initialize the read threads
    m_read_threads.resize(m_config.db_tables.size());
    for(size_t i=0; i < m_read_threads.size(); ++i) {
        m_read_threads[i].this_ptr = this;
        m_read_threads[i].table = m_config.db_tables[i].table;
        m_read_threads[i].thread_id = 0;
        m_read_threads[i].is_run = false;
    }
    /// load the business_layer so
    string so_path = "./business_layer.so";
    m_layer_handle = dlopen(so_path.c_str(), RTLD_LAZY);
    if(NULL == m_layer_handle) {
        LOG(LOG_ERROR, "Fail to load so [%s], err [%s]", so_path.c_str(), 
            dlerror());
        return false;
    }
    m_check_func = (CheckFunc)dlsym(m_layer_handle, "check_data_legal");
    char *error = dlerror();
    if( NULL != error) {
         LOG(LOG_ERROR, "Fail to load func, [%s]", error);
         return false;
    }
    return true;
}

bool SearchDataHandler::CoInit() {
    /// stop the reading threads
    if( !_StopIncReading()) return false;
    if( NULL != m_inc_data_ptr.get()) {
        m_inc_data_ptr->Dispose();
    }
    m_text_db.CoInit();
    _Empty();
    m_check_func = NULL;
    dlclose(m_layer_handle);
    m_layer_handle = NULL;
    return true;
}

bool SearchDataHandler::ProcessData(const string& config) {
    LOG(LOG_DEBUG, "process search data...[%s]", config.c_str());
    string process_type = config;
    /// make sure the function is state-independent
    string state_file = m_config.cache_data_path + string(STATE_FILE);
    /**
     *   Here we can't load state every time for that it maybe rollbacks the
     *   status of table reading, which makes me unhappy and upset
    if( ! _LoadState(state_file)) {
        LOG(LOG_ERROR, "Fail to initialize state from [%s]", 
            state_file.c_str());
        return false;
    }
    */
    if ("main" == process_type) {
        if ( ! _ResetDataTable(m_config.cache_data_path))
            return false;
        if ( ! _ReadRawData("main")) return false;
        if ( ! _CheckRawData("main")) return false;
        if ( ! _IndexData("main")) return false;
        if ( ! _CheckIndexData("main")) return false;
        if ( ! _BuildDataTable()) return false;
        if ( ! _CheckDataTable()) return false;
        if ( ! _UpdateMainState()) return false;
    } else if("inc" == process_type) {
        if ( ! _ReadRawData("inc")) return false;
        if ( ! _CheckRawData("inc")) return false;
        if ( ! _BuildIncData()) return false;
        if ( ! _CheckIncData()) return false;
        if ( ! _UpdateIncState()) return false;
    } else {
        LOG(LOG_ERROR, "unknown process type: [%s]", process_type.c_str());
        return false;
    }
    if ( !_SaveState(state_file)) {
        LOG(LOG_ERROR, "Fail to save state to [%s]", state_file.c_str());
        return false;
    }    
    return true;
}

int SearchDataHandler::GetIncDataLen(int inc_time_stamp) {
    if(inc_time_stamp >= m_stable_state_ptr->inc_time_stamp){
        LOG(LOG_INFO, "No data for [%d], what we have is less than [%d]", 
            inc_time_stamp, m_stable_state_ptr->inc_time_stamp);
        return -1;
    }
    return m_inc_data_ptr->GetDocLen(inc_time_stamp);
}

int SearchDataHandler::GetIncData(int inc_time_stamp, void* data_ptr){
    LOG(LOG_DEBUG, "Inc time stamp: %d", inc_time_stamp);
    if(inc_time_stamp >= m_stable_state_ptr->inc_time_stamp){
        LOG(LOG_INFO, "No data for [%d], what we have is less than [%d]", 
            inc_time_stamp, m_stable_state_ptr->inc_time_stamp);
        return -1;
    }
    SFetchInfo fi;
    const char* inc_data_ptr;
    inc_data_ptr = m_inc_data_ptr->GetData(inc_time_stamp, fi);
    if( NULL == inc_data_ptr) {
        LOG(LOG_ERROR, "Can't get data: %d", inc_time_stamp);
        return -1;
    }
    int len = m_inc_data_ptr->GetDocLen(inc_time_stamp);
    if( len > 0 )
        memcpy(data_ptr, inc_data_ptr, len);
    if( ! m_inc_data_ptr->PutData(fi)) {
        LOG(LOG_WARN, "Fail to put data back");
    }
    return len;
}

int SearchDataHandler::GetFullData( const std::string& key, void* data_ptr, 
                                    int len) {
    u64 data_key = atoll(key.c_str());
    char* full_data = (char*)m_data_table_ptr->get_check(data_key);
    if ( NULL != full_data) {
        /// pack the full data
        vector<char> data_val;
        _FillData(full_data, -1, data_val);
        int actual_len = len < data_val.size() ? len : data_val.size();
        memcpy(data_ptr, &data_val[0], actual_len);
        return actual_len;
    }
    return -1;
}

int SearchDataHandler::GetIncStamp() {
    if ( NULL == m_stable_state_ptr ) return -1;
    return m_stable_state_ptr->inc_time_stamp;
}

int SearchDataHandler::GetUpdateStatus() {
    if ( NULL == m_stable_state_ptr ) return 1;
    if ( m_stable_state_ptr->need_main) return 0;
    return 1;
}

bool SearchDataHandler::GetDataPath(string& data_path) {
    if ( NULL == m_stable_state_ptr ) return false;
    data_path = m_stable_state_ptr->main_data_dir;
    return true;
}

void SearchDataHandler::_Empty() {
    if ( NULL != m_cur_state_ptr) {
        delete m_cur_state_ptr;
        m_cur_state_ptr = NULL;
    }
    if ( NULL != m_stable_state_ptr) {
        delete m_stable_state_ptr;
        m_stable_state_ptr = NULL;
    }
    m_is_running = false;

    m_module_path = "";
    m_config.Empty();

    m_data_size = 0;
    
    m_read_threads.clear();
    m_data_table_ptr = DataTablePtr(new dy_hash_map<u64>);
    m_inc_data_ptr = IncDataPtr(new CDetail);
}

bool SearchDataHandler::_LoadState(const string& state_file) {
    if ( NULL == m_cur_state_ptr )
        m_cur_state_ptr = new StateInfo;
    if ( ! m_cur_state_ptr->LoadState(state_file, m_config.table_names)) {
        LOG(LOG_ERROR, "Fail to load state [%s]", state_file.c_str());
        return false;
    }
    /// back up the current legal state
    Backup<StateInfo>(m_cur_state_ptr, m_stable_state_ptr);
    return true;
}

bool SearchDataHandler::_SaveState(const string& state_file) {
    if ( ! m_cur_state_ptr->SaveState(state_file)) {
        LOG(LOG_ERROR, "Fail to save state [%s]", state_file.c_str());
        return false;
    }
    /// back up the current legal state
    Backup<StateInfo>(m_cur_state_ptr, m_stable_state_ptr);
    return true;
}

bool SearchDataHandler::_ReadRawData(const string& config) {
    string read_cmd;
    if( "main" == config){
        /// stop inc reading, start main reading
        if( ! _StopIncReading()) return false;
        /// @todo here we can't DO main data read by table, because the
        ///       builder can't index with mutli-tables
        /// update inc time stamp for next inc update
        m_cur_state_ptr->main_update_time = _GetDbTime();
        if( "" == m_cur_state_ptr->main_update_time){
            LOG(LOG_ERROR, "Fail to get main update time");
            return false;
        }
        string sql_query;
        Format(sql_query, m_config.sql_tmpl.c_str(), "*", 
            m_config.search_view.c_str(), "1");
        string read_cmd = m_config.read_cmd + string(" -q \"") + sql_query
            + string("\" -d ") + string(DATA_PATH);
        {
            timer_counter tc("read main");
            int ret = system(read_cmd.c_str());
            int retry_cnt = 0;
            while( 0 != ret && retry_cnt++ < 3 ) {
                /// some error occured, retry 3 times
                LOG(LOG_INFO, "Error occured, ret [%d]. "
                    "Retry [%d] to read main data", ret, retry_cnt);
                sleep(1<<retry_cnt);
                ret = system(read_cmd.c_str());
            }
            if( 0 != ret) {
                LOG(LOG_ERROR, "Fail to read main data");
                return false;
            }
        }
    }else if("inc" == config){
        /// cost more than 5 mins should give a warning
        timer_counter tc("read inc", LOG_WARN, 300000);
        /// check and start RunReading threads for reading data,
        if( ! _StartIncReading()) return false;
    }else{
        LOG(LOG_ERROR, "Unknown type [%s]", config.c_str());
        return false;
    }
    return true;
}

bool SearchDataHandler::_CheckRawData(const string& config) {
    /// @todo check the raw data's interity, size
    LOG(LOG_DEBUG, "Check raw data, [%s]", config.c_str());
    string data_path = DATA_PATH;
    if("main" == config) {
        
    }else if("inc" == config) {
        data_path = m_config.raw_data_path + string("data/");
    }
    return true;
}

bool SearchDataHandler::_IndexData(const string& config) {
    timer_counter tc("build index");
    LOG(LOG_DEBUG, "build index data");
    string index_cmd =  m_config.index_cmd;
    int ret = system(index_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to build index");
        return false;
    }
    return true;
}

bool SearchDataHandler::_CheckIndexData(const string& config) {
    /// @todo check index data using a simple api
    LOG(LOG_DEBUG, "TODO: check index data, [%s]", config.c_str());
    return true;
}

bool SearchDataHandler::_BuildDataTable() {
    timer_counter t_cnter("build main data");
    string data_path = DATA_PATH;
    LOG(LOG_DEBUG, "Load field data from files, [%s]", data_path.c_str());
    /// open the raw data files for reading
    string key_path = data_path+string("/")+m_config.key_field;
    ifstream key_fin(key_path.c_str());
    if( ! key_fin) {
        LOG(LOG_ERROR, "Fail to open key field [%s]", key_path.c_str());
        return false;
    }
    vector<vector<FilePtr> > str_files;
    str_files.resize(m_config.db_tables.size());
    for(size_t i=0; i < m_config.db_tables.size();++i) {
        if( !_ReadFiles(data_path, m_config.db_tables[i].str_fields, 
            str_files[i]))
            return false;
    }
    vector<vector<FilePtr> > num_files;
    num_files.resize(m_config.db_tables.size());
    for(size_t i=0; i < m_config.db_tables.size();++i) {
        if( !_ReadFiles(data_path, m_config.db_tables[i].num_fields, 
            num_files[i]))
            return false;
    }
    m_cur_state_ptr->total_data_num = 0;
    vector<char> data_val(m_data_size, 0);
    char* data_pos = &data_val[0];
    vector<string> num_fields;
    vector<string> str_fields; 
    u64 data_key=0;
    vector<string> layer_fields;
    layer_fields.resize(FIELD_NUM);
    vector<bool>  layer_flags;
    layer_flags.resize(FIELD_NUM, false);
    while( !key_fin.eof()) {
        /// new product data, here to be zero to avoid dirty data
        data_pos = &data_val[0];
        memset(data_pos, 0, sizeof(char)*m_data_size);
        /// read data from files
        string key_field;
        getline(key_fin, key_field, '\0');
        /// @see the last '\0' will be read, IGNORE it later, but not here
        for(size_t i=0; i < m_config.db_tables.size();++i) {
            /// read string & number data from files
            str_fields.resize(str_files[i].size());
            for(size_t j=0; j < str_fields.size();++j)
                getline(*str_files[i][j], str_fields[j], '\0');
            num_fields.resize(num_files[i].size());
            for(size_t j=0; j < num_fields.size();++j) 
                getline(*num_files[i][j], num_fields[j], '\0');
            /// build the row data
            _WriteMainData(data_pos, i, str_fields, num_fields);
            /// get the data needed by layer
            for(size_t j=0; j < m_table_map_field[i].size(); ++j) {
                FieldType type = (FieldType)m_table_map_field[i][j];
                if( ! _GetFieldVal(type, i, num_fields, layer_fields[type])) {
                    LOG(LOG_ERROR, "Fail to get val of layer field[%s]",
                        FIELD_NAME_LIST[type]);
                    return false;
                }
                layer_flags[type] = true;
            }
        }
        data_key = atoll(key_field.c_str());
        if( 0 == data_key) {
            LOG(LOG_DEBUG, "the key is empty.");
            continue;
        }
        if( ! _CheckLegal(layer_fields, layer_flags) ) {
            /// the product is illegal, here will not put it into data table
            LOG(LOG_ERROR, "the product [%s] is illegal", key_field.c_str());
            continue;
        } else {
            int* is_ok_ptr = (int*)data_pos;
            *is_ok_ptr = ST_OK; /// set it is ok
        }
        if( ! m_data_table_ptr->put(data_key, (void*)data_pos)) {
            LOG(LOG_ERROR, "Fail to put data into data table");
            return false;
        }
        m_cur_state_ptr->total_data_num ++;
        if( 0 == m_cur_state_ptr->total_data_num%100000){
            LOG(LOG_DEBUG, "Put %dth field data", 
                m_cur_state_ptr->total_data_num);
        }
    }
    LOG(LOG_DEBUG, "text db's size [%d]", m_text_db.GetSize());
    /// write memo data back  disk
    m_data_table_ptr->sync();
    return true;
}

bool SearchDataHandler::_CheckDataTable() {
    m_data_table_ptr->view_header();
    m_data_table_ptr->view_stat_info();
    /// check if the data size is normal
    float old_data_num = (float)(m_stable_state_ptr->total_data_num);
    float new_data_num = (float)(m_cur_state_ptr->total_data_num);
    if( 0 != old_data_num ) {
        int diff_percent = (int)(100*new_data_num / old_data_num);
        if( diff_percent < m_config.main_num_lower || 
            diff_percent > m_config.main_num_higher ) {
            LOG(LOG_ERROR, "Abnormal products num [%d] vs latest [%d]",
                (int)new_data_num, (int)old_data_num);
            return false;
        }
    }
    return true;
}

bool SearchDataHandler::_BuildIncData() {
    for(size_t i=0; i < m_config.db_tables.size(); ++i) {
        string table = m_config.db_tables[i].table;
        if( 1 == m_cur_state_ptr->tables[table].status) {
            if(!_BuildIncDataByTable(i)) {
                LOG(LOG_ERROR, "Fail to build inc data of table [%s]",
                    table.c_str());
                return false;
            }
            /// data is processed, need new data in
            m_cur_state_ptr->tables[table].status = 0;
        }
    }
    return true;
}

bool SearchDataHandler::_CheckIncData() {
    return true;
}

bool SearchDataHandler::_UpdateMainState() {
    /// - place data/ to module searcher's self data folder
    string date_str =  GetCurrentTime("%Y%m%d%H%M%S");
    string raw_file = m_config.raw_data_path + string("data_") + date_str;
    if( -1 != access(raw_file.c_str(), F_OK)) {
        LOG(LOG_WARN, "Target data folder is already exist [%s]",
            raw_file.c_str());
        string rm_cmd = string("rm -rf ") + raw_file;
        system(rm_cmd.c_str());
    }
    if ( ! KeepLatest(m_config.raw_data_path, m_config.bak_file_num) ) 
        return false;
    string mv_cmd = string("mv -f ") + DATA_PATH + string(" ") + raw_file + 
        string(" ;mkdir ") + DATA_PATH;
    int ret = system(mv_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to update the raw data, [%d]", ret);
        return false;
    }
    
    /// - place index/ to module searcher's self data folder
    string data_file = string("index_") + date_str;
    string data_dir = m_config.main_data_path  + data_file;
    if( -1 != access(data_dir.c_str(), F_OK)) {
        LOG(LOG_WARN, "Target data folder is already exist [%s]",
            data_dir.c_str());
        string rm_cmd = string("rm -rf ") + data_dir;
        system(rm_cmd.c_str());
    }
    if ( ! KeepLatest(m_config.main_data_path, m_config.bak_file_num) ) 
        return false;
    mv_cmd = string("mv -f ./index ") + data_dir + string(" ;mkdir index");
    ret = system(mv_cmd.c_str());
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to update the main data, [%d]", ret);
        return false;
    }

    /// - mark the main time stamp as this update
    string stamp_file = data_dir + string("/_full_timestamp");
    ifstream fin(stamp_file.c_str());
    if( ! fin.is_open()) {
        LOG(LOG_ERROR, "Fail to open the stamp file, [%s]", 
            stamp_file.c_str());
        return false;
    }
    m_cur_state_ptr->main_data_dir = data_dir;
    fin.read(reinterpret_cast<char *>(&(m_cur_state_ptr->main_time_stamp)),
        sizeof(m_cur_state_ptr->main_time_stamp));
    /// - update the inc state
    typedef map<string, TableStateInfo>::iterator Itor;
    for ( size_t i=0; i < m_config.db_tables.size(); ++i) {
        Itor it = m_cur_state_ptr->tables.find(m_config.db_tables[i].table);
        if ( m_config.db_tables[i].pre_read < 0) {
            it->second.inc_update_time = "";
        } else {
            it->second.inc_update_time = GetSysTime( 
                    m_cur_state_ptr->main_update_time,
                    m_config.db_tables[i].pre_read);
        }
        LOG(LOG_DEBUG, "Table [%s], inc update time [%s]", 
            m_config.db_tables[i].table.c_str(),
            it->second.inc_update_time.c_str());
        it->second.status = 0;
    }
    m_cur_state_ptr->inc_time_stamp = 0;
    m_cur_state_ptr->inc_modify_num = 0;
    m_cur_state_ptr->inc_add_num = 0;
    m_cur_state_ptr->inc_delete_num = 0;
    m_cur_state_ptr->inc_recover_num = 0;
    m_cur_state_ptr->new_product_num = 0;
    m_cur_state_ptr->need_main = _NeedMain();
    LOG(LOG_DEBUG, 
        "Succeed to update main data, update time [%s], main stamp [%d]", 
        m_cur_state_ptr->main_update_time.c_str(), 
        m_cur_state_ptr->main_time_stamp);
    return true;
}

bool SearchDataHandler::_UpdateIncState() {
    m_cur_state_ptr->need_main = _NeedMain();
    return true;
}

bool SearchDataHandler::_AutoSize() {
    /// autosize the data table size
    int sz = 0;
    /// is_ok fields lengths
    sz += sizeof(int);
    /// flag to mark the table has data or not
    m_flag_offset = sz;
    sz += m_config.db_tables.size();
    for(size_t i=0;i < m_config.db_tables.size(); ++i) {
        /// autosize the table offset
        m_config.db_tables[i].offset = sz;
        /// string field md5
        sz += sizeof(u64);
        /// string field index in text_db
        sz += sizeof(int);
        /// number field total
        for(size_t j=0; j < m_config.db_tables[i].num_field_lens.size();++j) {
            sz += m_config.db_tables[i].num_field_lens[j];
        }
    }
    /// address alignment by 8
    m_data_size = ((sz+8)/8)*8;

    return true;
}

bool SearchDataHandler::_MapField() {
    if( m_config.db_tables.empty()) return false; /// SHOULD NOT be empty
    m_field_map_table.resize(m_config.db_tables.size());
    m_table_map_field.resize(m_config.db_tables.size());
    m_field_map_data.resize(FIELD_NUM, -1);
    m_field_len_data.resize(FIELD_NUM);

    int offset = 0;

    for(size_t i=0; i < m_config.db_tables.size(); ++i) {
        /// @see the field of business MUST NOT be a string field here, which
        ///      is inefficient! Here we assume that the fields are all nums.
        offset = m_config.db_tables[i].offset;
        m_field_map_table[i].resize(FIELD_NUM, -1);
        offset += sizeof(u64); /// string fields length
        offset += sizeof(int); /// text index
        for(size_t j=0; j < m_config.db_tables[i].num_fields.size(); ++j) {
            for(size_t k=0; k < FIELD_NUM; ++k) {
                if( m_config.db_tables[i].num_fields[j] == 
                    FIELD_NAME_LIST[k] ) {
                    m_field_map_table[i][k] = j ;
                    m_table_map_field[i].push_back(k);
                    /// map the field to data table, given a field enum type,
                    /// return its data offset
                    m_field_map_data[k] = offset;
                    m_field_len_data[k] = 
                        m_config.db_tables[i].num_field_lens[j];
                    break;
                }
            }
            offset += m_config.db_tables[i].num_field_lens[j];
        }
    }
    return true;
}

bool SearchDataHandler::_InitDataTable(const string& path) {
    /// initialize the big table for main data
    string data_file = path + "maintable.dat";
    const unsigned int bucket_cnt = 100000;
    const unsigned int val_sz = m_data_size;
    const unsigned int slice_sz = 100000;
    const int allow_write = 1;
    if ( ! m_data_table_ptr->init(bucket_cnt, val_sz, slice_sz, 
        data_file.c_str(), allow_write) ) {
        LOG(LOG_ERROR, "Fail to initialize the data table");
        return false;
    }
    /// initialize the text db
    if ( ! m_text_db.Init(path)) {
        LOG(LOG_ERROR, "Fail to initialize the text db");
        return false;
    }
    return true;
}

bool SearchDataHandler::_ResetDataTable(const string& path) {
    /// @see here just move back the main table to reset data table
    ///      you can use a more graceful way to reset
    string data_file = path + "maintable.dat";
    if ( 0 == access(data_file.c_str(), F_OK) ) {
        string bak_cmd = string("mv -f ") + data_file + string(" ")
            + data_file + string(".bak");
        int ret = system(bak_cmd.c_str());
        if ( 0 != ret ) {
            LOG(LOG_ERROR, "Fail to back data table [%s]", bak_cmd.c_str());
            return false;
        }
    }
    /// reset the text db's cursor to 1
    m_text_db.Resize(1);
    /// reinit
    if ( ! _InitDataTable(path) ) return false;
    return true;
}

bool SearchDataHandler::_BuildIncDataByTable(int table_th) {
    LOG(LOG_DEBUG, "text db's size [%d]", m_text_db.GetSize());
    
    TableConfig& table_cfg = m_config.db_tables[table_th];
    string table = table_cfg.table;
    
    /// cost more than 1 mins should give a warning
    timer_counter t_cnter(table + string("build inc"), LOG_WARN, 60000);
    
    string data_path = string(DATA_PATH) + string("_") + table;
    LOG(LOG_DEBUG, "Load field data from files [%s]", data_path.c_str());
    /// open the raw data files for reading
    string key_path = data_path+string("/")+m_config.key_field;
    ifstream key_fin(key_path.c_str());
    if( ! key_fin) {
        LOG(LOG_ERROR, "Fail to open key field [%s]", key_path.c_str());
        return false;
    }
    vector<FilePtr> str_fin_vec;
    if( !_ReadFiles(data_path, table_cfg.str_fields, str_fin_vec)) 
        return false;
    vector<FilePtr> num_fin_vec;
    if( !_ReadFiles(data_path, table_cfg.num_fields, num_fin_vec)) 
        return false;

    /// Compare to screen inc datas and save them to file
    LOG(LOG_DEBUG, "Compare to screen inc datas and save them to file");
    vector<string> str_fields;
    str_fields.resize(str_fin_vec.size());
    vector<string> num_fields;
    num_fields.resize(num_fin_vec.size());
    vector<char> data_val(m_data_size, '\0');
    char* data_pos = &data_val[0];
    u64 data_key = 0;
    vector<string> layer_fields;
    layer_fields.resize(FIELD_NUM);
    vector<bool> layer_flags;
    layer_flags.resize(FIELD_NUM, false);
    while(!key_fin.eof()) {
        /// reset the layer_flags, here should not use memset!
        for(size_t i=0; i < layer_flags.size(); ++i)
            layer_flags[i] = false;
        /// no need to memet data here, we'll init it later
        data_pos = &data_val[0];
        /// read data from files
        string key_field;
        getline(key_fin, key_field, '\0');
        for(size_t i=0; i < str_fin_vec.size();++i)
            getline(*str_fin_vec[i], str_fields[i], '\0');
        for(size_t i=0; i < num_fin_vec.size();++i)
            getline(*num_fin_vec[i], num_fields[i], '\0');
        int status_new = ST_REJECT;
        int status_old = ST_REJECT;
        /// @see the last '\0' will be read, IGNORE it later, but not here
        data_key = atoll(key_field.c_str());
        if( 0 == data_key) {
            LOG(LOG_DEBUG, "the key is empty.");
            continue;
        }
        for(size_t i=0; i < m_table_map_field[table_th].size(); ++i) {
            FieldType type = (FieldType)m_table_map_field[table_th][i];
            if( ! _GetFieldVal(type, table_th, num_fields,
                layer_fields[type])) {
                LOG(LOG_ERROR, "Fail to get val of layer field[%s]",
                    FIELD_NAME_LIST[type]);
                return false;
            }
            layer_flags[type] = true;
        }

        int modify_type = TYPE_UNCHANGED;
        char* old_data = (char*)m_data_table_ptr->get_check(data_key);
        /// the inc message value
        vector<char> inc_data_val;
        if(NULL == old_data) {
            /// the data is illegal, here we will not write it to main data
            /// table and generate any inc messages
            /// @see must check legal when a new product is in, but not when
            ///      it is already exist, which maybe a ok -> no_ok change
            if( ! _CheckLegal(layer_fields, layer_flags)) 
                continue;
            /// new product
            memset(data_pos, 0, m_data_size);
        }else {
            /// make a copy
            memcpy(data_pos, old_data, m_data_size);
            status_old=*((int*)old_data);
        }
        /// write the new data to data val
        if(!_WriteMainData(data_pos, table_th, str_fields, num_fields)){
            LOG(LOG_ERROR, "Fail to write main data");
            return false;
        } 
        for(size_t i=0; i < layer_fields.size(); ++i) {
            if( ! layer_flags[i]) {
                /// @todo BUG, here confuses empty null and 0
                if( ! _GetFieldVal(data_pos + m_field_map_data[i], 
                    m_field_len_data[i], layer_fields[i])) {
                    LOG(LOG_ERROR, "Fail to get field val");
                    return false;
                }
                layer_flags[i] = true;
            }
        }
        if ( ! _CheckLegal(layer_fields, layer_flags)) {
            if ( status_old != ST_REJECT )   /// status jumps to delete
                *((int*)data_pos) = ST_DELETE;
            else 
                *((int*)data_pos) = ST_REJECT; /// usually new product
        } else {
            *((int*)data_pos) = ST_OK; /// is ok
        }
        status_new = *((int*)data_pos);
        if( status_new != ST_REJECT || status_old != ST_REJECT) {
            /// only when the change is not rejected will generate inc msg
            if( ! _ParseIncData(old_data, data_pos, table_th, str_fields,
                inc_data_val, modify_type) )
                return false;
        }
        /// generate inc message by inc_data_val
        if( !_GenerateIncMsg(modify_type, status_old, status_new, key_field, 
            inc_data_val)) {
            LOG(LOG_ERROR, "Fail to generate inc message with id [%s]", 
                key_field.c_str());
            return false;
        }
        /// flush the product info to data table
        if( ! m_data_table_ptr->put(data_key, (void*)data_pos)) {
            LOG(LOG_ERROR, "Fail to put [%d] into data table", data_key);
            return false;
        }
    }
    /// write memo data back into disk
    {
        /// cost more than 10 s should give a info
        timer_counter t_c(table + string("sync inc data"), LOG_INFO, 10000);
        // write back to disk
        m_data_table_ptr->sync();
        if( !m_inc_data_ptr->Sync()) {
            LOG(LOG_ERROR, "Fail to write inc data back to disk");
            return false;
        }
    }
    return true;
}

bool SearchDataHandler::_ParseIncData( const char* old_data, 
                                       const char* new_data, 
                                       int table_th, 
                                       const vector<string>& str_fields, 
                                       vector<char>& inc_data_val, 
                                       int& modify_type ) {
    TableConfig& table_cfg = m_config.db_tables[table_th];
    /// @todo BAD style, we should package the data as a class later
    int status_old = ST_REJECT;
    int status_new = ST_REJECT;
    if ( old_data != NULL )  status_old = *((int*)old_data);
    status_new = *((int*)new_data);
    /// the info string of changed fields
    string info_str;
    /// new add 
    if( old_data == NULL && new_data != NULL && status_new == ST_OK) { 
        /// here the important fields are all ready 
        /// and the data is legal, so we'll add it increasly
        /// pack the inc data for add
        inc_data_val.reserve(8092);
        for(size_t i=0; i < str_fields.size(); ++i) {
            AddPairToVec(table_cfg.str_fields[i], str_fields[i], 
                inc_data_val);
        }
        vector<string> new_nums;
        if ( ! _GetNumFields(new_data, table_th, new_nums))
            return false;
        for ( size_t i=0; i < new_nums.size(); ++i) {
            AddPairToVec(table_cfg.num_fields[i], new_nums[i], inc_data_val);
        }
        modify_type |= TYPE_NEW;
    } else { /// already exist
        if ( status_old == ST_REJECT && status_new == ST_OK ) {
            /// first into the data table
            modify_type |= TYPE_NEW;
            inc_data_val.reserve(8092);
        } else if( *((u64*)(old_data + table_cfg.offset)) != 
            *((u64*)(new_data + table_cfg.offset))) {
            /// check if the string fields are changed
            modify_type |= TYPE_STR;
        }

        if ( (modify_type & TYPE_NEW) || (modify_type & TYPE_STR) ) {
            /// pack the inc data for add
            inc_data_val.reserve(8092);
            for(size_t i=0; i < str_fields.size(); ++i) {
                AddPairToVec(table_cfg.str_fields[i], str_fields[i], 
                    inc_data_val);
            }
            /// pack num fields using data table num
            vector<string> new_nums;
            if ( ! _GetNumFields(new_data, table_th, new_nums))
                return false;
            for ( size_t i=0; i < new_nums.size(); ++i) {
                AddPairToVec(table_cfg.num_fields[i], new_nums[i],
                    inc_data_val);
            }
            if ( ! table_cfg.str_fields.empty() ) {
                /// add infos of changed string fields
                vector<string> old_strs;
                if( !_GetStrFields(old_data, table_th, old_strs)) 
                    return false;
                for(size_t i=0; i < table_cfg.str_fields.size(); ++i) {
                    if( old_strs[i] != str_fields[i] ) {
                        info_str += string("[") + table_cfg.str_fields[i]
                            + string(":") + old_strs[i] + string("]");
                    }
                }
            }
        }
        if( (modify_type & TYPE_STR) == 0 && (modify_type & TYPE_NEW) == 0 ) {
            /// the string fields are not changed and it is not new add
            /// check if the number fields are changed
            vector<string> old_nums;
            if ( ! _GetNumFields(old_data, table_th, old_nums))
                return false;
            vector<string> new_nums;
            if ( ! _GetNumFields(new_data, table_th, new_nums))
                return false;
            for ( size_t i=0; i < old_nums.size(); ++i) {
                if( old_nums[i] != new_nums[i]) {
                    modify_type |= TYPE_NUM;
                    AddPairToVec(table_cfg.num_fields[i], new_nums[i],
                        inc_data_val);
                    info_str += string("[") + table_cfg.num_fields[i] 
                        + string(":") + old_nums[i] + string("]"); 
                }
            }
        }
    }
    /// fill text data to inc message
    if( (modify_type & TYPE_STR) || (modify_type & TYPE_NEW) ) {
        if( ! _FillData(new_data, table_th, inc_data_val)) {
            LOG(LOG_ERROR, "Fail to fill data with table store");
            return false;
        }
    }
    /// add info string to inc message
    AddPairToVec("@info", info_str, inc_data_val);
    return true;
}

bool SearchDataHandler::_GenerateIncMsg(int modify_type, 
                                        int status_old,
                                        int status_new, 
                                        const string& key_field, 
                                        vector<char>& inc_data_val) {
    if( TYPE_UNCHANGED == modify_type )
        return true;  /// no needed
    vector<char> temp_data;  /// for 'delete', 'recover'
    if (modify_type & TYPE_NEW) {
        /// add only if status_new is ok
        if ( ! _WriteIncData(key_field, INC_ADD, inc_data_val) ) 
            return false;
        m_cur_state_ptr->new_product_num ++;
    }else if (modify_type & TYPE_STR) {
        /// delete only if status_old is ok
        if ( status_old == ST_OK ) { 
            temp_data.clear();
            if ( ! _WriteIncData(key_field, INC_DELETE, temp_data) )
                return false;
        }
        /// add
        if ( ! _WriteIncData(key_field, INC_ADD, inc_data_val) )
            return false;
        /// delete again if status_new is not ok
        if ( status_new != ST_OK ) {
            temp_data.clear();
            if ( ! _WriteIncData(key_field, INC_DELETE, temp_data) )
                return false;
        }
    }else if (modify_type & TYPE_NUM) {
        /// modify if status_old is ok
        if ( !_WriteIncData(key_field, INC_MODIFY, inc_data_val) )
            return false;
        if ( status_new == ST_OK && status_old == ST_DELETE ) {
            temp_data.clear();
            /// add when it is new add
            if( ! _WriteIncData(key_field, INC_RECOVER, temp_data) )
                return false;
        }
        if ( status_new != ST_OK && status_old == ST_OK ) {
            temp_data.clear();
            /// delete when it is no longer legal
            if ( ! _WriteIncData(key_field, INC_DELETE, temp_data) )
                return false;
        }
    }else {
        LOG(LOG_ERROR, "modify_type [%d], never should be", modify_type);
        return false;
    }
    return true;
}

bool SearchDataHandler::_WriteMainData(char* data_ptr, int table_th,
    const vector<string>& str_fields, const vector<string>& num_fields) {
    /// mark the table fields has data
    *(data_ptr + m_flag_offset + table_th) = 1;
    char* data_pos = data_ptr + m_config.db_tables[table_th].offset;
    if( ! str_fields.empty()) {
        /// write string fields' MD5 value
        u64 old_md5 = *((u64*)data_pos);
        u64 new_md5 = _MD5(str_fields);
        *((u64*)data_pos) = new_md5;
        data_pos += sizeof(u64);
        if( old_md5 != new_md5) {
            /// string fields have been changed, rewrite the string fields
            /// back to text db
            vector<char> str_vec;
            str_vec.reserve(8092);
            for(size_t i=0; i < str_fields.size(); ++i)
                AddStrToVec(str_fields[i], str_vec);
            int index = m_text_db.WriteData(str_vec);
            if( index <= 0) return false;
            *((int*)data_pos) = index;
        }
    } else {
        /// write string fields' MD5 value
        *((u64*)data_pos) = 0;
        data_pos += sizeof(u64);
        *((int*)data_pos) = -1; 
    }
    data_pos += sizeof(int);
    /// write number fields 
    for(size_t i=0; i< num_fields.size();++i) {
        _PackData(data_pos, num_fields[i],
            m_config.db_tables[table_th].num_field_lens[i]);
        data_pos += m_config.db_tables[table_th].num_field_lens[i];
    }
    return true;
}

bool SearchDataHandler::_WriteIncData( const std::string& key, 
                                       IncMsgType type,
                                       vector<char>& inc_data_val ) {
    AddPairToVec(string("@")+m_config.key_field, key, inc_data_val);
    AddPairToVec( "@stamp",
                  lexical_cast<string>(m_cur_state_ptr->inc_time_stamp), 
                  inc_data_val);
    switch(type) {
        case INC_MODIFY: 
            AddPairToVec("@type", "modify", inc_data_val);
            m_cur_state_ptr->inc_modify_num++;
            break;
        case INC_ADD: 
            AddPairToVec("@type", "add", inc_data_val);
            m_cur_state_ptr->inc_add_num++;
            break;
        case INC_DELETE: 
            AddPairToVec("@type", "delete", inc_data_val);
            m_cur_state_ptr->inc_delete_num++;
            break;
        case INC_RECOVER: 
            AddPairToVec("@type", "recover", inc_data_val);
            m_cur_state_ptr->inc_recover_num++;
            break;
        default: 
            break;
    }
    if( !m_inc_data_ptr->WriteDetail( &inc_data_val[0], 
                                      inc_data_val.size(),
                                      m_cur_state_ptr->inc_time_stamp)) {
        LOG(LOG_ERROR, "Fail to write [%d]th data", 
            m_cur_state_ptr->inc_time_stamp);
        return false;
    }
    m_cur_state_ptr->inc_time_stamp++;
    return true;
}

bool SearchDataHandler::_GetStrFields(const char* data_pos, int table_th,
    vector<string>& str_fields) {
    /// get the offset of string index 
    const char* data_ptr = data_pos + m_config.db_tables[table_th].offset
        + sizeof(u64);
    str_fields.resize(m_config.db_tables[table_th].str_fields.size());
    if ( 1 != *(data_pos + m_flag_offset + table_th) ) {
        /// if the data is not filled, here returns empty strings
        return true;
    }
    int str_index = *((int*)data_ptr);
    if( str_index <= 0) {
        LOG(LOG_ERROR, "The index is illegal [%d]", str_index);
        return false;
    }
    vector<char> str_vec;
    if( ! m_text_db.ReadData(str_index, str_vec)) {
        LOG(LOG_ERROR, "Fail to read text data [%d]", str_index);
        return false;
    }
    size_t pos = 0;
    for(size_t i=0; i < str_fields.size(); ++i) {
        str_fields[i] = string(&str_vec[pos]);
        pos += str_fields[i].size() + 1;
    }
    return true;
}

bool SearchDataHandler::_GetNumFields(const char* data_pos, int table_th,
    vector<string>& num_fields) {
    /// pack num fields using data table num
    const char* num_pos = data_pos + m_config.db_tables[table_th].offset 
        + sizeof(u64) + sizeof(int);
    num_fields.resize(m_config.db_tables[table_th].num_fields.size());
    for(size_t i=0; i < num_fields.size(); ++i) {
        int num_len = m_config.db_tables[table_th].num_field_lens[i];
        if( ! _GetFieldVal(num_pos, num_len, num_fields[i]) )
            return false;
        num_pos += num_len;
    }
    return true;
}

bool SearchDataHandler::_FillData( const char* data_pos, int exclude_table, 
                                   vector<char>& data_val) {
    for(size_t i = 0; i < m_config.db_tables.size(); ++i)
        /// only fill the data of which DO have, excluded passed in table
        if ( (int)i != exclude_table && 
             1 == *(data_pos + m_flag_offset + i) ) {
            /// pack string fields if have any
            if( ! m_config.db_tables[i].str_fields.empty()) {
                vector<string> str_fields;
                if( ! _GetStrFields(data_pos, i, str_fields)) 
                    return false;
                for(size_t j=0; j < str_fields.size(); ++j ) {
                    AddPairToVec(m_config.db_tables[i].str_fields[j],
                        str_fields[j], data_val);
                }
            }
            /// pack number fields 
            vector<string> num_fields;
            if ( ! _GetNumFields(data_pos, i, num_fields))
                return false;
            for(size_t j=0;j < num_fields.size();++j) {
                AddPairToVec(m_config.db_tables[i].num_fields[j], 
                    num_fields[j], data_val);
            }
        }
    return true;
}

bool SearchDataHandler::_NeedMain() {
    bool need_main = false;
    int per = (int)((float)(100*m_cur_state_ptr->inc_add_num) / 
        (float)(m_cur_state_ptr->total_data_num+1));
    char fmt[] = "%Y-%m-%d %H:%M:%S";
    struct tm tb;
    if (strptime(m_cur_state_ptr->main_update_time.c_str(),fmt,&tb)==NULL){
        LOG(LOG_ERROR,"Fail to read last main update time");
        return false;
    }
    time_t last_time=mktime(&tb);
    long ts = time(NULL);
    struct tm *ptm;
    ptm = localtime(&ts);
    int h = ptm-> tm_hour;
    int interval = int((ts-last_time)/3600);
    if( per >= m_config.main_update_threshold
        || interval >= m_config.main_update_interval ){
        if ( h >= m_config.main_update_start_time 
            && h <= m_config.main_update_stop_time ) {
            /// only when it is between 01:00:00 and 04:00:00 will do main
            LOG(LOG_INFO, "Need do main update, new [%d] vs total [%d], "
                "per [%d] vs threshold [%d], interval [%d] vs [%d]",
                m_cur_state_ptr->inc_add_num,
                m_cur_state_ptr->total_data_num,
                per, m_config.main_update_threshold,
                interval, m_config.main_update_interval);
            need_main = true;
        }
    }
    if( m_config.force_update_threshold > 0 && 
        per >= m_config.force_update_threshold) {
        LOG(LOG_INFO, "Force to do main update, new [%d] vs total [%d], "
            "per [%d] vs threshold [%d]",
            m_cur_state_ptr->inc_add_num,
            m_cur_state_ptr->total_data_num,
            per, m_config.force_update_threshold);
        need_main = true;
    }
    return need_main;
}

string SearchDataHandler::_GetDbTime() {
    string db_time;
    string time_str = string("CURRENT_TIMESTAMP_") 
        + lexical_cast<string>(getpid()) + string("_")
        + lexical_cast<string>(pthread_self());
    string time_cmd = m_config.read_cmd
        + string(" -q \"select CURRENT_TIMESTAMP() as ")
        + time_str + string("\"");
    int ret = system(time_cmd.c_str());
    int retry_cnt = 0;
    while( 0 != ret && retry_cnt++ < 3) {
        LOG(LOG_WARN, "Fail to get database time, ret [%d]. Retry [%d]",
            ret, retry_cnt);
        sleep(1<<retry_cnt);
        ret = system(time_cmd.c_str());
    }
    if( 0 != ret) {
        LOG(LOG_ERROR, "Fail to get db time [%s], error ret [%d]",
            time_cmd.c_str(), ret);
        return "";
    }
    ifstream fin(time_str.c_str());
    if(fin) getline(fin, db_time, '\0');
    string rm_cmd = string("rm -f ") + time_str;
    ret = system(rm_cmd.c_str());
    if(0 != ret) {
        LOG(LOG_ERROR, "Fail to remove temp time file [%s], error ret [%d]",
            rm_cmd.c_str(), ret);
    }
    return db_time;
}

void* SearchDataHandler::_RunReading(void* arg) {
    _ThreadData* ti = (_ThreadData*)arg;
    ti->is_run = true;
    SearchDataHandler* this_ptr = ti->this_ptr;
    string table = ti->table;
    while(this_ptr->m_is_running) { 
        StatePtr state_ptr = this_ptr->m_cur_state_ptr;
        if( 0 == state_ptr->tables[table].status ) {
            /// cost more than 5 mins should give a warning
            timer_counter tc(table + string(" reading inc"), LOG_WARN, 300000);
            LOG(LOG_DEBUG, "Reading inc data in thread [%d] for table [%s]",
                pthread_self(), table.c_str());
            /// generate the sql query
            string timestamp_sql;
            timestamp_sql += "(";
            for(size_t i=0; i < this_ptr->m_config.db_tables.size(); ++i) {
                if( table == this_ptr->m_config.db_tables[i].table) {
                    /// get the limit sql for inc update
                    vector<string>& ts_fields = 
                        this_ptr->m_config.db_tables[i].timestamp_fields;
                    int j=0;
                    for(; j < (int)ts_fields.size() - 1; ++j) {
                        timestamp_sql += ts_fields[j] + string(" >= \\\"")
                            + state_ptr->tables[table].inc_update_time
                            + string("\\\" or ");
                    }
                    timestamp_sql += ts_fields[j] + string(" >= \\\"")
                        + state_ptr->tables[table].inc_update_time
                        + string("\\\"");
                    break;
                }
            }
            timestamp_sql += ")"; /// avoid logic wrong
            string sql_query;
            Format(sql_query, this_ptr->m_config.sql_tmpl.c_str(), "*", 
                table.c_str(), timestamp_sql.c_str()); 
            string read_cmd = this_ptr->m_config.read_cmd 
                + string(" -q \"") + sql_query + string("\" -d ") 
                + string(DATA_PATH) + string("_") + table;
            /// get raw data
            string update_time = this_ptr->_GetDbTime();
            int ret = system(read_cmd.c_str());
            int retry_cnt = 0;
            while( this_ptr->m_is_running && 0 != ret && retry_cnt++ < 3) {
                LOG(LOG_INFO, "Error occured, ret [%d]. Retry [%d] to read"
                    " inc data", ret, retry_cnt);
                sleep(1<<retry_cnt);
                ret = system(read_cmd.c_str());
            }
            if( 0 != ret) {
                LOG(LOG_ERROR, "Fail to read inc [%s]", read_cmd.c_str());
                /// thread is abnormally quit
                ti->is_run = false;
                return (void*)-1;
            }
            /// set the flags
            this_ptr->m_cur_state_ptr->tables[table].status = 1;
            this_ptr->m_cur_state_ptr->tables[table].inc_update_time = 
                update_time;
        }
        /// sleep 1 second to avoid busy data reading
        sleep(1);
    }
    /// normally quit
    ti->is_run = false;
    return (void*)0;
}

bool SearchDataHandler::_StartIncReading() {
    m_is_running = true;
    for(size_t i=0; i < m_read_threads.size(); ++i) {
        if( !m_read_threads[i].is_run) {
            if(0 != m_read_threads[i].thread_id) {
                /// the thread has been stopped, should join to clean up
                int ret = pthread_join(m_read_threads[i].thread_id, NULL);
                if( 0 != ret ) {
                    LOG(LOG_WARN, "Fail to join thread [%s], err [%d]",
                        m_read_threads[i].table.c_str(), ret);
                }
                m_read_threads[i].thread_id = 0;
            }
            int ret = pthread_create(&(m_read_threads[i].thread_id), NULL,
                _RunReading, (void*)&m_read_threads[i]);
            if( 0 != ret ) {
                LOG(LOG_ERROR, "Fail to create thread [%s], err [%d]",
                    m_read_threads[i].table.c_str(), ret);
                return false;
            }
        }
    }
    return true;
}

bool SearchDataHandler::_StopIncReading() {
    m_is_running = false;
    for(size_t i=0; i < m_read_threads.size(); ++i) {
        if( 0 != m_read_threads[i].thread_id) {
            int ret = pthread_join(m_read_threads[i].thread_id, NULL);
            if( 0 != ret ) {
                LOG(LOG_WARN, "Fail to join thread [%s], err [%d]",
                    m_read_threads[i].table.c_str(), ret);
                return false;
            }
        }
        m_read_threads[i].thread_id = 0;
    }
    return true;
}

bool SearchDataHandler::_CheckLegal(const vector<string>& layer_fields,
    const vector<bool>& layer_flag) {
    LAYER_FIELD layer = {0};
    /// @see following codes are some kind ugly.
    if( layer_flag[PRODUCT_NAME_STATUS] ) 
        layer.product_name_status = layer_fields[PRODUCT_NAME_STATUS].c_str();
    if( layer_flag[DISPLAY_STATUS] ) 
        layer.display_status = layer_fields[DISPLAY_STATUS].c_str();
    if( layer_flag[PRODUCT_MEDIUM] ) 
        layer.product_medium = layer_fields[PRODUCT_MEDIUM].c_str();
    if( layer_flag[PRODUCT_TYPE] ) 
        layer.product_type = layer_fields[PRODUCT_TYPE].c_str();
    if( layer_flag[IS_PUBLICATION] ) 
        layer.is_publication = layer_fields[IS_PUBLICATION].c_str();
    if( layer_flag[IS_CATALOG_PRODUCT] ) 
        layer.is_catalog_product = layer_fields[IS_CATALOG_PRODUCT].c_str();
    if( layer_flag[MAIN_PRODUCT_ID] ) 
        layer.main_product_id = layer_fields[MAIN_PRODUCT_ID].c_str();
    if( layer_flag[DD_SALE_PRICE] ) 
        layer.dd_sale_price = layer_fields[DD_SALE_PRICE].c_str();
    if( layer_flag[STOCK_STATUS] ) 
        layer.stock_status = layer_fields[STOCK_STATUS].c_str();
    if( layer_flag[PRE_SALE] ) 
        layer.pre_sale = layer_fields[PRE_SALE].c_str();
    if( layer_flag[IS_SHARE_PRODUCT] ) 
        layer.is_share_product = layer_fields[IS_SHARE_PRODUCT].c_str();
    if( layer_flag[SHOP_INFO_STATUS] ) 
        layer.shop_info_status = layer_fields[SHOP_INFO_STATUS].c_str();
    if( layer_flag[CATEGORY_ID] ) 
        layer.category_id = layer_fields[CATEGORY_ID].c_str();
    if( layer_flag[SHOP_ID] ) 
        layer.shop_id = layer_fields[SHOP_ID].c_str();
    if( layer_flag[IS_PUBLISH] ) 
        layer.is_publish = layer_fields[IS_PUBLISH].c_str();
    int ret = m_check_func(&layer);
    if( 0 != ret) 
        return false; /// illegal
    return true;
}

bool SearchDataHandler::_ReadFiles(const string& data_path, 
    const vector<string>& file_name_vec, vector<FilePtr>& file_in_vec) {
    for(size_t i=0;i < file_name_vec.size();i++) {
        string field_path = data_path+string("/")+file_name_vec[i];
        FilePtr fin_ptr = FilePtr(new ifstream(field_path.c_str()));
        if( ! (*fin_ptr)) {
            LOG(LOG_ERROR, "Fail to open file of field [%s]",
                field_path.c_str());
                return false;
        }
        file_in_vec.push_back(fin_ptr);
    }
    return true;
}

bool SearchDataHandler::_PackData(char* data_ptr, const string& str, int sz) {
    switch(sz) {
        case 1:
            *((char*)data_ptr) = (char)atoi(str.c_str());
            break;
        case 2:
            *((short*)data_ptr) = (short)atoi(str.c_str());
            break;
        case 4:
            *((int*)data_ptr) = atoi(str.c_str());
            break;
        case 8:
            *((long long*)data_ptr) = atoll(str.c_str());
            break;
        default:
            strcpy(data_ptr, str.c_str());
            if((int)str.size() >= sz ) {
                LOG(LOG_ERROR, "Abnormal time string [%s]", str.c_str());
                return false;
            }
            break;
    }    
    return true;
}

bool SearchDataHandler::_GetFieldVal(const char* data_ptr, int sz, 
    string& val) {
    switch(sz) {
        case 1:
            val = lexical_cast<string>((int)(*((char*)data_ptr)));
            break;
        case 2:
            val = lexical_cast<string>((int)(*((short*)data_ptr)));
            break;
        case 4:
            val = lexical_cast<string>((int)(*((int*)data_ptr)));
            break;
        case 8:
            val = lexical_cast<string>((long long)(*((long long*)data_ptr)));
            break;
        default:
            /// as a string
            val = data_ptr;
            if( (int)val.size() >= sz ) {
                LOG(LOG_ERROR, "Abnormal time string [%s]", val.c_str());
                return false;
            }
            break;
    }
    return true;
}


#ifdef UNIT_TEST
/// include head files for unit test
#include <iostream>
using namespace std;

string LOG_NAME = "searcher_handler.log";

bool test_SearchDataHandler() {
    cout<<"Unit test - SearchDataHandler"<<endl;
    if(0)
    {
        bool ret = true;
        cout<<"usecase : read and build raw data in main mode"<<endl;
        SearchDataHandler hd;
        ret &= hd.Init("./modules/searcher/");
        ret &= hd.ProcessData("main");
        vector<char> data_buf;
        ret &= hd.CoInit();
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    if(0)
    {
        bool ret = true;
        cout<<"usecase : read and build raw data in inc mode"<<endl;
        SearchDataHandler hd;
        ret &= hd.Init("./modules/searcher/");
        ret &= hd.ProcessData("inc");
        vector<char> data_buf;
        cout<<"Sleep to wait for data ready ..."<<endl;
        sleep(280);
        ret &= hd.ProcessData("inc");
        ret &= hd.CoInit();
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    //if(0)
    {
        bool ret = true;
        cout<<"usecase : reset and build in main and inc mode"<<endl;
        SearchDataHandler hd;
        ret &= hd.Init("./modules/searcher/");
        if( ! ret) {
            cout<< "Fail to init"<<endl;
            return false;
        }
        ret &= hd.ProcessData("main");
        ret &= hd.ProcessData("inc");
        sleep(300);
        ret &= hd.ProcessData("inc");
        /*
        int cnt = 2;
        while(cnt--) {
            ret &= hd.ProcessData("inc");
            sleep(300);
        }
        */
        ret &= hd.ProcessData("inc");
        sleep(30);
        ret &= hd.CoInit();
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - SearchDataHandler"<<endl;
    return true;
}

#ifdef TEST_SEARCH_DATA_HANDLER
int main() {
    if(!test_SearchDataHandler())
        return -1;
    return 0;
}
#endif // TEST_SEARCH_DATA_HANDLER
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
