#ifndef HANDLER_CONFIG_H
#define HANDLER_CONFIG_H
#include "swstd.h"
#include <string>
#include <vector>
#include <set>

/// the struct of sql table configure informations
struct TableConfig {
    //  ----------------------------------
    //  datas
    //  ----------------------------------

    /// data table name
    std::string table;
    /// number fields list
    std::vector<std::string> num_fields;
    /// number fields' length in data table
    std::vector<int> num_field_lens;
    /// string fields list
    std::vector<std::string> str_fields;
    /// timestamp fields
    std::vector<std::string> timestamp_fields;
    /// pre_read days of first inc update
    int pre_read;
    /// the flag of important number fields, 1 : important, 0 : no
    std::vector<int> mark_num_fields;
    /// the flag of important string fields, 1 : important, 0 : no
    std::vector<int> mark_str_fields;
    /// the start offset in big table
    int offset;
    /// the start offset of bit flag in big table
    int flag_offset;
    
    //  -----------------------------------
    //  functions
    //  -----------------------------------

    /// empty the config 
    void Empty();
};

/// the struct of searcher data handler configure informations
struct HandlerConfig {
    //  ----------------------------------
    //  datas
    //  ----------------------------------

    /// main data path, saving main data for remote getting
    std::string main_data_path;
    /// raw data path, saving raw data reading from database
    /// @todo it is needed for backing up the raw data to check data error,
    ///       now the func is no completed yet.
    std::string raw_data_path;
    /// cache data path, saving cache data of module
    std::string cache_data_path;
    /// read command, used for reading data from database
    std::string read_cmd;
    /// main search view, a sql view for main update
    std::string search_view;
    /// sql query template
    std::string sql_tmpl;
    /// index command, used for indexing
    std::string index_cmd;
    /// db table name list
    std::vector<std::string> table_names;
    /// database tables, the table list for inc update
    std::vector<TableConfig> db_tables;
    /// key field name
    std::string key_field;
    /// main update threshold, which is for controlling main update
    int main_update_threshold;
    /// force main update
    int force_update_threshold;
    /// lower limit of main update products num
    int main_num_lower;
    /// higher limit of main update products num
    int main_num_higher;
    /// main update start time
    int main_update_start_time;
    /// main update stop time
    int main_update_stop_time;
    /// main update interval time
    int main_update_interval;
    /// bakup file's number
    int bak_file_num;

    //  -----------------------------------
    //  functions
    //  -----------------------------------

    /// empty the config
    void Empty();
    /// load config from config file
    bool LoadConfig(const std::string& config_file);
};

#endif // ~>.!.<~
