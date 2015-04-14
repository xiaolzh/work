#ifndef GLOBAL_DEFINE_H
#define GLOBAL_DEFINE_H
#include "swstd.h"
#include <string>
#include "logger.h"

enum IncType {
    /// unchanged
    TYPE_UNCHANGED = 0,
    /// a new product, tag as add
    TYPE_NEW = 0x1,
    /// number changed, tag as modify
    TYPE_NUM = 0x2,
    /// string changed, tag as delete & add
    TYPE_STR = 0x4,
    /// display status changed to 0, tag as delete
    TYPE_STATUS_0 = 0x8,
    /// display status changed to 1, tag as recover
    TYPE_STATUS_1 = 0x10
};

static const char* CONFIG_FILE = "searcher.conf";

#endif // ~>.!.<~
