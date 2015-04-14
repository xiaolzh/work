#include "data_handler.h"
#include <vector>
#include <string>
#include "logger.h"
#include "global_define.h" 
#include "search_data_handler.h"
#include "configer.h"
using namespace std;

SearchDataHandler search_hd;

int init(const char* config) {
    LOG(LOG_DEBUG, "search data handler initialize...");
    if ( ! search_hd.Init(config) )
        return -1;
    return 0;
}

int co_init() {
    LOG(LOG_DEBUG, "search data handler co-initialize...");
    if ( ! search_hd.CoInit() )
        return -1;
    return 0;
}

int process_data(const char* config) {
    LOG(LOG_DEBUG, "search data handler process data...");
    if ( ! search_hd.ProcessData(config) ) 
        return -1;
    return 0;
}

int get_inc_data(int inc_time_stamp, void* data_buf, int data_len) {
    LOG(LOG_DEBUG, "get the processed inc data back");
    if (NULL == data_buf) 
        return search_hd.GetIncDataLen(inc_time_stamp);
    int len = search_hd.GetIncData(inc_time_stamp, data_buf);
    return len;
}

int get_full_data(const char* key, void* data_buf, int data_len) {
    LOG(LOG_DEBUG, "get the full data back by key");
    int len = search_hd.GetFullData(key, data_buf, data_len);
    return len;
}

int get_inc_stamp() {
    LOG(LOG_DEBUG, "get inc stamp");
    return search_hd.GetIncStamp();
}

int get_update_status() {
    LOG(LOG_DEBUG, "get update status");
    return search_hd.GetUpdateStatus();
}

int get_data_path(char* path_buffer, int buffer_len) {
    LOG(LOG_DEBUG, "get data path");
    string data_path;
    if ( ! search_hd.GetDataPath(data_path) ) return -1;
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
        ret &= process_data("main");
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }

    {
        bool ret = true;
        cout<<"usecase: process data in inc type"<<endl;
        ret &= process_data("inc");
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
