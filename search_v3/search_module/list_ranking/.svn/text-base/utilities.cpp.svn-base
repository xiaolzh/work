#include "utilities.h"
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <errno.h> 
#include <iterator>
#include "logger.h"
using namespace std;

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
    string tmp_file = "tmp.xx";
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

bool CreatePath(const string& path) {
    if( -1 == access(path.c_str(), F_OK)) { 
        if( -1 == mkdir(path.c_str(), 0777)){
            LOG(LOG_ERROR, "Fail to create path [%s], error info [%s]",
                path.c_str(), strerror(errno));
            return false;
        }
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
