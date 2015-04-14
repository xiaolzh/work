#ifndef UTILITIES_H
#define UTILITIES_H
#include "swstd.h"
#include <string>
#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>

bool ExeProcess(const char* path, char * argv[], char ** environ);

bool GetFileList(const std::string& dir, std::vector<std::string>& file_list);

bool KeepLatest(const std::string& dir, int file_num = 3);

/// return files' number of the folder, if 'show_hidden' is set by true,
/// files contains the hiddens, this func return -1 if any error occurs
int GetFileNum(const std::string& folder, bool show_hidden = false);

std::vector<std::string> Split(const std::string& str, 
    const std::string& split_str);

bool CreatePath(const std::string& path);

static inline void AddStrToVec(const std::string& str, 
    std::vector<char>& vec) {
    std::copy(str.begin(), str.end(), std::back_inserter(vec));
    vec.push_back('\0');
}

static inline void AddPairToVec(const std::string& key, 
    const std::string& val, std::vector<char>& vec) {
    if("" == val) {
        AddStrToVec(std::string("#")+key, vec);
        AddStrToVec("NULL", vec);
    }else{
        AddStrToVec(key, vec);
        AddStrToVec(val, vec);
    }
    //AddStrToVec("\n", vec);
}

static inline std::string ParseUrl(const std::string& url, 
    const std::string& field){
    std::string field_val;
    std::string field_key = field+"=";
    std::string::size_type pos = url.find(field_key);
    if(std::string::npos != pos) {
        field_val = url.substr(pos+field_key.size());
        pos = field_val.find("&");
        if(std::string::npos != pos) {
            field_val = field_val.substr(0, pos);
        }
    }
    return field_val;
}

static inline bool IsNumStr(const std::string& str) {
    for(size_t i=0; i < str.size(); ++i) {
        if( str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

static inline std::string GetCurrentTime(
    const std::string& format= "%Y-%m-%d %H:%M:%S") {
    time_t now;
    struct tm timenow;
    time(&now);
    localtime_r(&now, &timenow);
    char buf[255];
    strftime(buf, 255, format.c_str(), &timenow);
    return &buf[0];
}

static inline std::string TrimLeft(const std::string& str, 
    const std::string& trim_list = " \n\r\t") {
    size_t pos = str.find_first_not_of(trim_list);
    if(pos != std::string::npos)
        return str.substr(pos);
    return str;
}

static inline std::string TrimRight(const std::string& str, 
    const std::string& trim_list = " \n\r\t") {
    size_t pos = str.find_last_not_of(trim_list);
    if( pos != std::string::npos)
        return str.substr(0, pos+1);
    return str;
}

static inline std::string TrimBoth(const std::string& str, 
    const std::string& trim_list = " \n\r\t") {
    return TrimLeft(TrimRight(str, trim_list), trim_list);
}

std::string& Format(std::string& str, const char* format_str, ...);

std::string GetLocalIP();

std::string GetSysTime(const std::string& time, int day,
    const std::string& format = "%Y-%m-%d %H:%M:%S");

template<typename _T>
INLINE
int SerializeData(void* val_field, _T val) {
    memcpy(val_field, (void*)&val, sizeof(val));
    return sizeof(val);
}

template < typename _T1, typename _T2 >          
INLINE        
bool ConvertArray(_T1 c_array[], int len, std::vector<_T2>& cpp_array ) {
    std::copy(c_array, c_array+len, std::back_inserter(cpp_array));           
    return true;                                                              
}

template < typename _T1, typename _T2>
INLINE
bool ConvertType(const _T1& val, _T2& convertedVal) {
    try {
        convertedVal = boost::lexical_cast<_T2>(val);
        return true;
    }catch(boost::bad_lexical_cast& e) {
        return false;
    }
}

template < typename _T >
void Backup(_T* a, _T*& b) {
    if ( NULL == a ) {
        if ( NULL != b) {
            delete b;
        }
        b = NULL;
        return;
    }
    _T* tmp = new _T(*a);
    if ( NULL != b ) {
        _T* tmp2 = b;
        b = tmp;
        delete tmp2;
    } else {
        b = tmp;
    }
}

#endif // ~>.!.<~
