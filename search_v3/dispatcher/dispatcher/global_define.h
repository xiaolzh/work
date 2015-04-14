#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H
#include "swstd.h"
#include <string>

enum QueryStatus {
    OK = 0,
    ILLEGAL_MODULE = 1,
    ILLEGAL_MAIN_TIME_STAMP = 2,
    ILLEGAL_INC_TIME_STAMP = 3,
    NO_DATA = 4,
    ERROR = 5,
    UNDEFINED
};

static const char* QUERY_STATUS_INFO[] = {
    "OK",
    "Illegal module type",
    "Illegal main time stamp",
    "Illegal inc time stamp",
    "Data temporarily unused",
    "Error",
    "Unknow error"
    };

static const char* GetStatus(int st) {
    if( st >= OK && st <= ERROR)
        return QUERY_STATUS_INFO[st];
    return "Unknow error";
};

enum UpdateStatus {
    /// need no update
    NO_UPDATE,
    /// need total main process
    NEED_PROCESS_MAIN,
    /// need total inc process
    NEED_PROCESS_INC,
    /// need reload main data
    NEED_RELOAD_MAIN
};

static const char* CONFIG_FILE = "dispatcher.conf";

#endif // ~>.!.<~
