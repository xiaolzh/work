#include "utilities.h"
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <errno.h> 
#include <stdarg.h>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include "logger.h"
using namespace std;
using boost::lexical_cast;

bool ExeProcess(const char* path, char * argv[], char ** environ) {
    /// @todo get exe return value
    pid_t pid;
    int stat_val = -1;
    pid = fork();
    switch(pid) {
        case -1:
            LOG(LOG_ERROR, "Process Creation failed\n");
            return false;
        case 0:
            LOG(LOG_DEBUG, 
                "child process is running! My pid = %d,parentpid = %d\n",
                getpid(), getppid()
                );
            execve(path,argv,environ);
            LOG(LOG_WARN, "process never go to here!\n");
            return true;
        default:
            LOG(LOG_DEBUG, "Parent process is running\n");
            break;
    }
    LOG(LOG_DEBUG, "Wait for son process quit");
    wait(&stat_val);
    LOG(LOG_DEBUG, "OK, finished now!");
    return true;
}

bool GetFileList(const string& dir, vector<string>& file_list) {
    /// @todo Smelly Implement
    string tmp_file = "tmp_" + lexical_cast<string>(getpid()) 
        + string("_") + lexical_cast<string>(pthread_self());
    string cmd = string("ls ") + dir + string(" >> ") + tmp_file;
    system(cmd.c_str());

    ifstream fin;
    fin.open(tmp_file.c_str());
    string field;
    while(getline(fin, field)) {
        file_list.push_back(field);
    }
    fin.close();
    cmd = string("rm ") + tmp_file;
    system(cmd.c_str());
    return true;
}

bool KeepLatest(const string& dir, int file_num /*= 5*/) {
    vector<string> file_list;
    if ( ! GetFileList(dir, file_list) ) return false;
    /// no need to rm old file
    int file_size = (int)file_list.size();
    if ( file_size < file_num ) return true;
    /// @see let's guess the (file_list.size() - file_num)'s result!
    for ( int i=0; i < file_size - file_num; ++i) {
        string rm_cmd = string("rm -rf ") + dir + string("/") + file_list[i];
        int ret = system(rm_cmd.c_str());
        if( 0 != ret) {
            LOG(LOG_ERROR, "Fail to rm old data [%s]", rm_cmd.c_str());
            return false;
        }
    }
    return true;
}

/// return files' number of the folder, if 'show_hidden' is set by true,
/// files contains the hiddens, this func return -1 if any error occurs
int GetFileNum(const string& folder, bool show_hidden/* = false*/) {
    if( 0 != access(folder.c_str(), F_OK)) {
        LOG(LOG_ERROR, "Fail to acess folder [%s]", folder.c_str());
        return -1;
    }
    int num = 0;
    string cmd = string("ls ") + folder;
    if( show_hidden ) cmd += " -A";
    cmd += "|wc -l";
    FILE* stream = popen(cmd.c_str(), "r");
    if( NULL == stream) {
        LOG(LOG_ERROR, "Fail to open [%s], err [%s]", cmd.c_str(), 
            strerror(errno));
        return -1;
    }
    char buf[1024];
    fread( buf, sizeof(char), sizeof(buf),  stream); 
    num = atoi(buf);
    int ret = pclose(stream);
    if( 0 != ret ) {
        LOG(LOG_ERROR, "Fail to cmd [%s], ret [%d]", cmd.c_str(), ret);
        return -1;
    }
    return num;
}

bool CreatePath(const string& path) {
    string cmd = string("mkdir -p ") + path;
    int ret = system(cmd.c_str());
    if( ret != ret) {
        LOG(LOG_ERROR, "Fail to create path [%s]", cmd.c_str());
        return false;
    }
    return true;
}

vector<string> Split(const string& str, const string& split_str) {
    vector<string> split_vec;
    size_t pos = 0;
    size_t start = 0;
    while((pos=str.find_first_of(split_str,start))!=string::npos){
        split_vec.push_back(TrimBoth(str.substr(start, pos-start)));
        start = pos + split_str.size();
    }
    split_vec.push_back(TrimBoth(str.substr(start)));
    return split_vec;
}

string& Format(string& str, const char* format_str, ...) {
    va_list args;
    int sz = 1024;
    while(true) {
        str.resize(sz);
        /// try to print in the 1024 allocated space
        va_start(args, format_str);
        int n = vsnprintf(&str[0], sz, format_str, args);
        va_end(args);
        /// if that worked, return the string
        if( n > -1 && n < sz) {
            str.resize(n);
            return str;
        }
        if( n > -1 )
            /// resize the space precisely what is needed
            sz = n+1;
        else
            /// twice the old size
            sz *= 2;
    }
} 

string GetLocalIP() {
    string cmd = "/sbin/ifconfig eth0 | grep 'inet addr:'|"
        "awk -F: '{print $2}'|awk '{print $1}'";
    FILE* stream = popen(cmd.c_str(), "r");
    if( NULL == stream) {
        LOG(LOG_ERROR, "Fail to open [%s], err [%s]", cmd.c_str(), 
            strerror(errno));
        return "";
    }
    char buf[1024] = {0};
    size_t n = fread( buf, sizeof(char), sizeof(buf),  stream); 
    if(n == 0) {
        LOG(LOG_ERROR, "None to read");
        return "";
    }
    buf[n-1] = '\0'; /// remove the '\n', which is just ok in this case
    int ret = pclose(stream);
    if( 0 != ret ) {
        LOG(LOG_ERROR, "Fail to cmd [%s], ret [%d]", cmd.c_str(), ret);
        return "";
    }
    return string(&buf[0]);
}

string GetSysTime(const string& time, int day,
    const string& format /*= "%Y-%m-%d %H:%M:%S"*/ ) {
    struct tm t;
    if ( NULL == strptime(time.c_str(), format.c_str(), &t)) {
        LOG(LOG_ERROR, "Fail to convert time [%s]", time.c_str());
        return "";
    }
    time_t tc = mktime(&t);
    tc >= day*86400 ? tc -= day*86400 : 0;
    struct tm tnew;
    localtime_r(&tc, &tnew);
    string new_time;
    new_time.resize(2*format.size(), '\0');
    size_t len = strftime(&new_time[0], new_time.size(), format.c_str(), &tnew);
    new_time.resize(len);
    return new_time;
}

#if UNIT_TEST
bool test_utilities() {
    
    return true;
}

#if TEST_UTILITIES
int main() {
    if( !test_utilities()) {
        return -1;
    }
    return 0;
}
#endif // TEST_UTILITIES

#endif // UNIT_TEST
// I wanna sing a song for you, dead loop no escape
