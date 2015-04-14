#include "data_handler.h"
#include <stdio.h>
#include <vector>
#include <string>
#include "logger.h"
#include "global_define.h"
#include "configer.h"
#include "common_data_handler.h"
using namespace std;

CommonDataHandler common_hd;

int init(const char* config) {
    LOG(LOG_DEBUG, "search data handler initialize...");
    if ( ! common_hd.Init(config) ) 
        return -1;
    return 0;
}

int co_init() {
    LOG(LOG_DEBUG, "search data handler co-initialize...");
    if ( ! common_hd.CoInit() )
        return -1;
    return 0;
}

int process_data(const char* config) {
    LOG(LOG_DEBUG, "search data handler process data...");
    if ( ! common_hd.ProcessData(config) )
        return -1;
    return 0;
}

int get_inc_data(int inc_stamp, void* data_buf, int data_len) {
    LOG(LOG_ERROR, "No implement yet for getting inc data");
    return -1;
}

int get_full_data(const char* key, void* data_buf, int data_len) {
    LOG(LOG_ERROR, "No implement yet for getting full data");
    return -1;
}

int get_inc_stamp() {
    LOG(LOG_DEBUG, "get inc stamp");
    return common_hd.GetIncStamp();
}

int get_update_status() {
    LOG(LOG_DEBUG, "get update status");
    return common_hd.GetUpdateStatus();
}

int get_data_path(char* path_buffer, int buffer_len) {
    LOG(LOG_DEBUG, "get data path");
    string data_path;
    if ( ! common_hd.GetDataPath(data_path) ) return -1;
    int len = snprintf(path_buffer, buffer_len, "%s", data_path.c_str());
    return len;
}

#if UNIT_TEST

/// include head files for unit test
#include <iostream>
#include <string>
using namespace std;

bool test_data_handler() {
    cout<<"Unit Test: data_handler"<<endl;
    {
        bool ret = true;
        cout<<"usecase: process data in main type"<<endl;
        ret &= ( 0 == process_data("main") );
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }

    {
        bool ret = true;
        cout<<"usecase: process data in inc type"<<endl;
        ret &= ( 0 == process_data("inc") );
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }

    cout<<"Done unit test - zipper"<<endl;
    return true;
}

#if TEST_DATA_HANDLER
int main() {
    if(!test_data_handler())
        return -1;
    return 0;
}
#endif // TEST_DATA_HANDLER

#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
