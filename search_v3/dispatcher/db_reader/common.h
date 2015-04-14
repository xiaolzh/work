#ifndef DB_READER_COMMON_H
#define DB_READER_COMMON_H

#include <string>

/**
 *  Common definition in db_reader
 */

/// sql config paras
struct SQLConfig {
    char* host;
    int port;
    char* db;
    char* user;
    char* pwd;
    char* sql;
};

struct ReaderConfig {
    SQLConfig sql_cfg;
    std::string data_path;
    std::string log_path;
};

/// read data with sql config to data path, return true if succeed
bool ReadMysql(const SQLConfig& cfg, const std::string& data_path); 

/// parse options to get reader's config
bool ParseOpt(int argc, char* argv[], ReaderConfig& cfg);

/// rewrite some special fields 
bool RewriteFields(const std::string& data_path);

/// check the fields status and mark it
bool CheckStatus(const std::string& data_path);

#endif // ~>.!.<~
