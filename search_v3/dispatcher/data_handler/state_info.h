#ifndef STATE_INFO_H
#define STATE_INFO_H
#include "swstd.h"
#include <map>
#include <string>
#include <vector>

/// the struct of sql table state informations
struct TableStateInfo{
    /// sql table name
    std::string table;
    /// the inc update sql time 
    std::string inc_update_time;
    /// 1: no data  0: data ready
    int status;        
};

/// the struct of state informations
struct StateInfo{
    //  ----------------------------------
    //  datas
    //  ----------------------------------

    /// neee do main update, true : need, false : no
    bool need_main;
    /// main update timestamp
    int main_time_stamp;
    /// main update time
    std::string main_update_time;
    /// main data path, if no data, the path is ""
    std::string main_data_dir;
    /// inc update timestamp
    int inc_time_stamp;
    /// the number of modify inc messages
    int inc_modify_num;
    /// the number of add inc messages 
    int inc_add_num;
    /// the number of delete inc messages
    int inc_delete_num;
    /// the number of recover inc messages
    int inc_recover_num;
    /// the number of new products
    int new_product_num;
    /// total data number in data table
    int total_data_num;
    /// the infos of sql tables
    std::map<std::string, TableStateInfo> tables;

    //  -----------------------------------
    //  functions
    //  -----------------------------------
    
    bool LoadState(const std::string& state_file, 
        const std::vector<std::string>& table_names);
    bool SaveState(const std::string& state_file);
};

#endif // ~>.!.<~
