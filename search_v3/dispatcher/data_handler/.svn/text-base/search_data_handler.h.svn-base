#ifndef SEARCH_DATA_HANDLER_H
#define SEARCH_DATA_HANDLER_H
/// head files include
#include "swstd.h"
#include <errno.h>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <fstream>
#include <pthread.h>
#include <boost/shared_ptr.hpp>
#include "dynamic_hash.h"
#include "Detail.h"
#include "global_define.h"
#include "handler_config.h"
#include "state_info.h"
#include "business_layer.h"
#include "text_db.h"

enum IncMsgType {
    INC_MODIFY = 0,        /// inc msg of modify a data  
    INC_ADD = 1,           /// inc msg of add a data
    INC_DELETE = 2,        /// inc msg of delete a data
    INC_RECOVER = 3        /// inc msg of recover a data
};

class SearchDataHandler {
public:
    typedef boost::shared_ptr<dy_hash_map<u64> > DataTablePtr;
    typedef boost::shared_ptr<CDetail> IncDataPtr;
    typedef boost::shared_ptr<std::ifstream> FilePtr;
public:
    SearchDataHandler();
    ~SearchDataHandler();
   
    bool Init(const std::string& config);
    bool CoInit();

    bool ProcessData(const std::string& config);
    int GetIncDataLen(int inc_time_stamp);
    int GetIncData(int inc_time_stamp, void* data_ptr);
    int GetFullData(const std::string& key, void* data_ptr, int len);
    int GetIncStamp();
    int GetUpdateStatus();
    bool GetDataPath(std::string& data_path);

private:
    void _Empty();
    bool _LoadState(const std::string& state_file);
    bool _SaveState(const std::string& state_file);

    bool _ReadRawData(const std::string& config);
    bool _CheckRawData(const std::string& config);
    bool _IndexData(const std::string& config);
    bool _CheckIndexData(const std::string& config);
    bool _BuildDataTable();
    bool _CheckDataTable();
    bool _BuildIncData();
    bool _CheckIncData();
    bool _UpdateMainState();
    bool _UpdateIncState();

    /// follow are subcall functions
    bool _AutoSize();
    bool _MapField();
    bool _InitDataTable(const std::string& path);
    bool _ResetDataTable(const std::string& path);
    bool _BuildIncDataByTable(int table_th);
    bool _ParseIncData(const char* old_data, const char* new_data,
        int table_th, const std::vector<std::string>& str_fields, 
        std::vector<char>& inc_data_val, int& modify_type);
    bool _GenerateIncMsg(int modify_type, int status_old, int status_new,
        const std::string& key_field, std::vector<char>& inc_data_val);
    bool _WriteMainData(char* data_pos, int table_th, 
        const std::vector<std::string>& str_fields,
        const std::vector<std::string>& num_fields);
    bool _WriteIncData( const std::string& key, IncMsgType type,
                        std::vector<char>& inc_data_val );
    bool _GetStrFields(const char* data_pos, int table_th, 
        std::vector<std::string>& str_fields);
    bool _GetNumFields(const char* data_pos, int table_th,
        std::vector<std::string>& num_fields);
    bool _FillData( const char* data_pos, int exclude_table, 
                    std::vector<char>& inc_data_val);
    bool _NeedMain();
    std::string _GetDbTime();
    static void * _RunReading(void* arg);
    bool _StartIncReading();
    bool _StopIncReading();
    bool _CheckLegal(const std::vector<std::string>& layer_fields,
        const std::vector<bool>& layer_flag);
    
    /// follow are utility functions
    static bool _ReadFiles(const std::string& data_path, 
        const std::vector<std::string>& file_name_list,
        std::vector<FilePtr>& file_in_vec);

    /// inline functions
    inline bool _GetFieldVal(FieldType type, int table_th,
        const std::vector<std::string>& num_fields,
        std::string& val) {
        int nth = m_field_map_table[table_th][type];
        if( -1 == nth || nth >= (int)num_fields.size()) return false;
        val = num_fields[nth];
        return true;
    }
    static inline u64 _MD5(const std::vector<std::string>& str_vec) {
        std::string str_vec_sum;
        for(size_t i=0;i < str_vec.size();++i)
            str_vec_sum += str_vec[i];
        return _hash64(str_vec_sum.c_str());
    }
    static bool _PackData(char* data_ptr, const std::string& str, int sz);
    static bool _GetFieldVal(const char* data_ptr, int sz, std::string& val);

private:
    typedef StateInfo* StatePtr;
    typedef int (*CheckFunc)(LAYER_FIELD*);
    struct _ThreadData {
        /// owner pointer
        SearchDataHandler* this_ptr;
        /// sql table name
        std::string table;
        /// thread id
        pthread_t thread_id;
        /// status of thread, true : running, false : stopped
        bool is_run;
    };

    // ---------------------------------------
    // state data
    // ---------------------------------------

    /// current state, which is changed in real time
    StatePtr m_cur_state_ptr;
    /// stable state, which is certified adn has been writen back to disk
    StatePtr m_stable_state_ptr;
    /// the status of reading threads, true : run, false : stopp
    bool m_is_running;

    // ---------------------------------------
    // config data
    // ---------------------------------------

    /// module path, the module's file path, for getting self location
    std::string m_module_path;
    /// config data in struct
    HandlerConfig m_config;

    // ------------------------------------------
    // cache data
    // ------------------------------------------

    /// the size of a row data in data table
    int m_data_size;
    /// the map of field to table, [field, table_nth]
    vector<vector<int> > m_field_map_table;
    /// the map of table to field, [table, field_list]
    vector<vector<int> > m_table_map_field;
    /// the map of field to data, [field, data_table_offset]
    vector<int> m_field_map_data;
    /// the len of field map to data, [field, lens in data table]
    vector<int> m_field_len_data;
    /// the offset of table mark flags
    int m_flag_offset;
    /// the inc reading threads
    std::vector<_ThreadData> m_read_threads;
    
    /// the data table of all products, saving the number fields and 
    /// MD5s of string fields
    DataTablePtr m_data_table_ptr;
    /// the text db of illegal products' string fields, which is stored for
    /// it to be a legal product
    TextDB  m_text_db;
    /// the inc data messages, order by inc number
    IncDataPtr m_inc_data_ptr;
    /// the business layer for handling dangdang business
    void* m_layer_handle;
    /// function to check if data is legal or not
    CheckFunc m_check_func;

    /// Disallow copy and assign defaultly
    DISALLOW_COPY_AND_ASSIGN(SearchDataHandler);
};

#endif // ~>.!.<~ 
