#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <mysql/mysql.h>
#include <vector>
#include <string>
#include <fstream>
#include "util.h"
#include "common.h"
using namespace std;

int main(int argc,char* argv[]) {
    ReaderConfig cfg;
    /// default config
    cfg.data_path = "./";
    cfg.log_path = "./";
    /// - parse the opts
    if( ! ParseOpt(argc, argv, cfg)) {
        ERROR("Fail to pars opt");
        return -1;
    }
    /// - init logger file
    string log_file = cfg.log_path + string("reader_for_search.log");
    if( ! InitLogger(log_file) ) {
        ERROR("Fail to init logger");
        return -1;
    }
    /// - make sure the data path is existed
    if( -1 == access(cfg.data_path.c_str(), F_OK)) {
        string cmd = string("mkdir -p ") + cfg.data_path;
        int ret = system(cmd.c_str());
        if( 0 != ret) {
            ERROR("Fail to create data path [%s]", cmd.c_str());
            return -1;
        }
    }
    /// - make sure the log path is existed
    if( -1 == access(cfg.log_path.c_str(), F_OK)) {
        string cmd = string("mkdir -p ") + cfg.log_path;
        int ret = system(cmd.c_str());
        if( 0 != ret) {
            ERROR("Fail to create log path [%s]", cmd.c_str());
            return -1;
        }
    }
    if( ! ReadMysql(cfg.sql_cfg, cfg.data_path.c_str())) {
        ERROR("Fail to read data from mysql");
        return -1;
    }
    /// - rewrite some special fields
    if( ! RewriteFields(cfg.data_path)) {
        ERROR("Fail to rewrite fields");
        return -1;
    }
    /// - check the fields status and mark it
    if( ! CheckStatus(cfg.data_path)) {
        ERROR("Fail to check status");
        return -1;
    }
	return 0;
}
