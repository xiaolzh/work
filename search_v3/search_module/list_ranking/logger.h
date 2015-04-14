#ifndef LOGGER_H
#define LOGGER_H
#include "swstd.h"
#include <stdio.h>
#include <string>
#include <map>

enum Level {
    LOG_ALL = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL,
    LOG_OFF
};

class Logger{
public:
    Logger(const std::string& logger_name="");
    ~Logger();

    static Logger* GetLogger(const std::string& logger_name="");

    bool SetHandler(const std::string& filename);
    bool SetHandler(int fd);
    bool SetLevel(Level lvl);

    void Log(Level lvl, const char* format, ...);
    void Debug(const char* format, ...);
    void Info(const char* format, ...);
    void Warn(const char* format, ...);
    void Error(const char* format, ...);
    void Fatal(const char* format, ...);

private:
    void _Log(Level lvl, const char* log_info);
    
private:
    static std::map<std::string, Logger*> ms_loggers;
    Level m_lvl;
    FILE* m_file;
    std::string m_name;
};

bool SetLogger(Level lvl, const std::string& log_file = "",
    const std::string& logger = "");

#define LOG_BASE(logger, level, format, ...) \
    do { \
        Logger::GetLogger(logger)->Log(level, "[%s@%s,%d] " format"", \
            __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
    } while (0) 

#define LOG(level, format, ...) LOG_BASE("", level, format, ##__VA_ARGS__ )

#endif // ~>.!.<~
