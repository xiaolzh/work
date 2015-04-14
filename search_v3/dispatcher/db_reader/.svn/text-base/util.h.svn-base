#ifndef DB_READER_UTIL_H
#define DB_READER_UTIL_H 
#include <stdio.h>
#include <sys/time.h>
#include <string>

static inline std::string GetCurrentTime(
    const std::string& format = "%Y-%m-%d %H:%M:%S") {
    time_t now;
    struct tm timenow;
    time(&now);
    /// locatime() is not thread-safe
    localtime_r(&now, &timenow);
    char buf[255];
    strftime(buf, 255, format.c_str(), &timenow);
    return &buf[0];
}

#define ERROR(format, ...) \
    do{ \
        fprintf(stderr, "%s ERROR [%s@%s,%d] "format"\n", \
            GetCurrentTime().c_str(), __func__, __FILE__,\
            __LINE__, ##__VA_ARGS__ ); \
    }while(0)

bool InitLogger(const std::string& log_file = "");

/// convert isbn13 to isbn10
std::string ISBN10(const std::string& isbn);
/// convert isbn10 to isbn13
std::string ISBN13(const std::string& isbn);

void RemoveHtmlStr(std::string& src);
#endif // ~>.!.<~
