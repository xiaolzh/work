#include "logger.h"
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <stdarg.h>
#include <errno.h>
using std::string;
using std::map;

map<string, Logger*> Logger::ms_loggers;

Logger::Logger(const std::string& logger_name/*=""*/) {
    m_name = logger_name;
    SetHandler("");
    SetLevel(LOG_DEBUG);
}

Logger::~Logger() {
    if(stderr != m_file) fclose(m_file);
}

Logger* Logger::GetLogger(const string& logger_name/*=""*/) {
    if(ms_loggers.end() != ms_loggers.find(logger_name)) {
        return ms_loggers[logger_name];
    }
    Logger* lg_ptr = new Logger(logger_name);
    ms_loggers[logger_name] = lg_ptr;
    return lg_ptr;
}

bool Logger::SetHandler(const string& filename) {
    if("" == filename) {
        //int fd = 0;
        //dup2(fileno(stderr), fd);
        //m_file = fdopen(fd, "a+");
        m_file = stderr;
        return true;
    }
    FILE* f = fopen(filename.c_str(), "a+");
    if(NULL == f) {
        Error("Fail to open file [%s]", filename.c_str());
        return false;
    }
    if( stderr != m_file) fclose(m_file);
    m_file = f;
    return true;
}

bool Logger::SetHandler(int fd) {
    FILE* f = fdopen(fd, "a+");
    if(NULL == f) {
        Error("Fail to open fd [%d], err [%s]", fd, strerror(errno));
        return false;
    }
    if(stderr != m_file) fclose(m_file);
    m_file = f;
    /// fcntl()
    return true;
}

bool Logger::SetLevel(Level lvl) {
    m_lvl = lvl;
    return true;
}

void Logger::Log(Level lvl, const char* format, ...) {
    if(lvl >= m_lvl) {
        char buffer[20480] = {0};
        va_list args;                                                         
        va_start(args, format);                                               
        vsnprintf(buffer, sizeof(buffer), format, args);   
        _Log(lvl, buffer);               
        va_end(args);                                                         
    }// else do nothing
}

void Logger::Debug(const char* format, ...) {
    if(LOG_DEBUG >= m_lvl) {
        char buffer[20480] = {0};
        va_list args;                                                         
        va_start(args, format);                                               
        vsnprintf(buffer, sizeof(buffer), format, args);   
        _Log(LOG_DEBUG, buffer);               
        va_end(args);                                                         
    }// else do nothing
}

void Logger::Info(const char* format, ...) {
    if(LOG_INFO >= m_lvl) {
        char buffer[20480] = {0};
        va_list args;                                                         
        va_start(args, format);                                               
        vsnprintf(buffer, sizeof(buffer), format, args);   
        _Log(LOG_INFO, buffer);               
        va_end(args);                                                         
    }// else do nothing
}

void Logger::Warn(const char* format, ...) {
    if(LOG_WARN >= m_lvl) {
        char buffer[20480] = {0};
        va_list args;                                                         
        va_start(args, format);                                               
        vsnprintf(buffer, sizeof(buffer), format, args);   
        _Log(LOG_WARN, buffer);               
        va_end(args);                                                         
    }// else do nothing
}

void Logger::Error(const char* format, ...) {
    if(LOG_ERROR >= m_lvl) {
        char buffer[20480] = {0};
        va_list args;                                                         
        va_start(args, format);                                               
        vsnprintf(buffer, sizeof(buffer), format, args);   
        _Log(LOG_ERROR, buffer);               
        va_end(args);                                                         
    }// else do nothing
}

void Logger::Fatal(const char* format, ...) {
    if(LOG_FATAL >= m_lvl) {
        char buffer[20480] = {0};
        va_list args;                                                         
        va_start(args, format);                                               
        vsnprintf(buffer, sizeof(buffer), format, args);   
        _Log(LOG_FATAL, buffer);               
        va_end(args);                                                         
    }// else do nothing
}

void Logger::_Log(Level lvl, const char* log_info) {
    /// get current time
    struct timeval tv;
    struct tm ptm = {0};
    char time_string[40];
    long milliseconds;
    gettimeofday (&tv, NULL);
    localtime_r (&tv.tv_sec, &ptm);
    strftime(time_string, sizeof (time_string), "%Y-%m-%d %H:%M:%S", &ptm);
    milliseconds = tv.tv_usec / 1000;

    static const char* level_str[] = {
        "", "DEBUG", "INFO", "WARN", "ERROR", "FATAL", ""};
    fprintf(m_file, "%s.%03ld %s %s\n", &time_string[0], milliseconds,
        level_str[lvl], log_info);
    fflush(m_file);
}

bool SetLogger(Level lvl, const string& log_file/*=""*/,
    const string& logger/*=""*/) {
    bool ret = true;
    Logger* lg_ptr = Logger::GetLogger(logger);
    ret &= lg_ptr->SetHandler(log_file);
    ret &= lg_ptr->SetLevel(lvl);
    return ret;
}


#if UNIT_TEST

void test_Logger(){
    {
        Logger lg;
        lg.Log(LOG_WARN, "Test logger");
        lg.Debug("logger debug");
        lg.Fatal("Fatal test [%s] [%d]", "args", 1);
        lg.SetHandler("test_log");
        lg.SetLevel(LOG_ALL);
        lg.Info("info test [%s]", "args");
        Logger* lg_ptr = Logger::GetLogger("test");
        lg_ptr->Error("You can see me in file test");
        Logger* lg_ptr2 = Logger::GetLogger();
        lg_ptr2->Error("You can see me in console");
    }
    {
        LOG(LOG_DEBUG, "log debug info: %s", "Hello"); 
    }    
    {
        SetLogger(LOG_WARN, "test_logger.log");
        LOG(LOG_DEBUG, "DEBUG NO SEE, you should not see this");
        LOG(LOG_ERROR, "ERROR, you should know");
        LOG(LOG_WARN, "WARN, you should see this");
    }    
    {
        SetLogger(LOG_ALL);
        LOG(LOG_DEBUG, "DEBUG [%d], you should see this", LOG_DEBUG);
        LOG(LOG_WARN,  "LOG_WARN [%d], you should see this", LOG_WARN);
        LOG(LOG_ERROR, "LOG_ERROR [%d], you should see this", LOG_ERROR);
        LOG(LOG_FATAL, "LOG_FATAL [%d], you should see this", LOG_FATAL);
    }
}

#if TEST_LOGGER
int main() {
    test_Logger();
    return 0;
}
#endif // TEST_LOGGER

#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
