#include <math.h>
#include <dlfcn.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "common_func.h"
using namespace std;

/// error log, format like [func@file, 124, log info]                         
#define ERROR(format, ...) \
    do{ \
        fprintf(stderr, "[%s@%s,%d] " format "\n", \
            __func__, __FILE__, __LINE__, ##__VA_ARGS__ ); \
    }while(0)

bool ReadFile(const string& file, string& file_str) {
    ifstream in(file.c_str(), ios::in);
    if(!in) return false;
    istreambuf_iterator<char> beg(in), end;
    file_str.assign(beg, end);
    in.close();
    return true;
}

bool CreateSo(const string& script, const string& fm, const string& path,
    const vector<string>& fields) {
    char buffer[102400];
    string head_str, cpp_str;
    /// create hxx file
    if( ! ReadFile(path + "script.h_tmpl", head_str)) {
        ERROR("Fail to read file script.h_tmpl");
        return false;
    }
    ofstream fout( (path + script + string(".hxx")).c_str() );
    if( !fout ) {
        ERROR("Fail to open [%s.hxx]", script.c_str());
        return false;
    }
    fout<<head_str;
    fout.close();
    if( ! ReadFile(path + "script.cpp_tmpl", cpp_str)) {
        ERROR("Fail to read file script.cpp_tmpl");
        return -1;
    }
    fout.open( (path + script + string(".cxx")).c_str() );
    if( !fout) {
        ERROR("Fail to open [%s.cxx]", script.c_str());
        return false;
    }
    ostringstream sout;
    for(size_t i=0; i < fields.size(); ++i) {
        sout<<"\tint "<<fields[i]<<"_ = field_values["<<i<<"];"<<endl;
    }
    snprintf(buffer, sizeof(buffer)-1, cpp_str.c_str(), script.c_str(), 
        sout.str().c_str(), fm.c_str());
    /// avoid string overflow
    buffer[102400-1] = '\0';
    fout<<buffer;
    fout.close();
    string compile_cmd = string("cd ") + path 
        + string(";g++ -O3 -fPIC -o %s.so %s.cxx -shared;cd -;");
    snprintf(buffer, sizeof(buffer)-1, compile_cmd.c_str(),
        script.c_str(), script.c_str());
    int ret = system(buffer);
    if( 0 != ret) {
        ERROR("Fail to create so [%s], ret [%d]", buffer, ret);
        return false;
    }
    return true;
}

int main(int argc, char** argv) {
    if(argc < 4) {
        cerr<<"./exe [script_name] [formula_string] [work_path]"<<endl;
        return -1;
    }
    printf("script_name:[%s], formula_string:[%s], work_path:[%s]\n", 
        argv[1], argv[2], argv[3]);
    string script(argv[1]);
    string fm(argv[2]);
    string path(argv[3]);
    path += "/";
    vector<string> fields;
    if( ! ParseFormula(fm, fields)) return -1;
    if( ! CreateSo(script, fm, path, fields)) return -1;
    /// test
    void* dl_handle = dlopen(string(path + script+".so").c_str(), 
        RTLD_LAZY);
    if( NULL == dl_handle) {
        ERROR("Fail to load so, [%s]", dlerror());
        return -1;
    }
    typedef int (*Func)(int*, int);
    Func dl_func = (Func)dlsym(dl_handle, "compute_weight");
    char *error = dlerror();
    if( NULL != error) {
        ERROR("Fail to load func, [%s]", error);
        return -1;
    }
    //int field[] = {1, 2};
    vector<int> field(10, 1);
    cout<<dl_func(&field[0], field.size())<<endl;
    dlclose(dl_handle);
    return 0;
}
