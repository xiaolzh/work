#ifndef UTILITIES_H
#define UTILITIES_H
#include "swstd.h"
#include <string>
#include <vector>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "logger.h"

bool ExeProcess(const char* path, char * argv[], char ** environ);

bool GetFileList(const std::string& dir, std::vector<std::string>& file_list);

std::vector<std::string> Split(const std::string& str, 
    const std::string& split_str);

/// file(dir, link) is exist or not, true means exist, false means not
static inline bool FileExist(const char* path) {
    if( -1 == access(path, F_OK)) 
        return false;
    return true;
}

bool CreatePath(const std::string& path);

inline void AddStrToVec(const std::string& str, std::vector<char>& vec) {
    std::copy(str.begin(), str.end(), std::back_inserter(vec));
    vec.push_back('\0');
}

inline void AddPairToVec(const std::string& key, const std::string& val,
    std::vector<char>& vec) {
    if("" == val) {
        AddStrToVec(std::string("#")+key, vec);
        AddStrToVec("NULL", vec);
    }else{
        AddStrToVec(key, vec);
        AddStrToVec(val, vec);
    }
    //AddStrToVec("\n", vec);
}

inline std::string ParseUrl(const std::string& url, const std::string& field){
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

inline bool IsNumStr(const std::string& str) {
    for(size_t i=0; i < str.size(); ++i) {
        if( str[i] < '0' || str[i] > '9')
            return false;
    }
    return true;
}

inline std::string GetCurrentTime(
    const std::string& format= "%Y-%m-%d %H:%M:%S") {
    time_t now;
    struct tm* timenow;
    time(&now);
    timenow = localtime(&now);
    char buf[255];
    strftime(buf, 255, format.c_str(), timenow);
    return &buf[0];
}

inline std::string TrimLeft(const std::string& str, 
    const std::string& trim_list = " \n\r\t") {
    size_t pos = str.find_first_not_of(trim_list);
    if(pos != std::string::npos)
        return str.substr(pos);
    return str;
}

inline std::string TrimRight(const std::string& str, 
    const std::string& trim_list = " \n\r\t") {
    size_t pos = str.find_last_not_of(trim_list);
    if( pos != std::string::npos)
        return str.substr(0, pos+1);
    return str;
}

inline std::string TrimBoth(const std::string& str, 
    const std::string& trim_list = " \n\r\t") {
    return TrimLeft(TrimRight(str, trim_list), trim_list);
}


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


#endif // ~>.!.<~
