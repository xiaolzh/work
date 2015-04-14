#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <iterator>
#include <fstream>
#include <map>
#include <set>
#include <locale.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include<dirent.h>
#include <mysql/mysql.h>
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "logger.h"
#include "dynamic_hash.h"
#include "utilities.h"
#include "configer.h"
using namespace std;
using boost::lexical_cast;

std::string LOG_NAME = "";

string _GetDbTime() {
    string time_str = string("CURRENT_TIMESTAMP_") + lexical_cast<string>(getpid()) + string("_") + lexical_cast<string>((long)(pthread_self()));
    string read_cmd = string("./reader -H 192.168.85.158 -P 33062 -D prodviewdb -u readuser -p password -d ./ -q \"select CURRENT_TIMESTAMP() as ") +
        time_str + string("\""); 
    int ret = system(read_cmd.c_str());
    ifstream fin(time_str.c_str());
    string str;
    if(fin)getline(fin, str, '\0');
    return str;
}

struct TS {
    int i;
    short i1;
    char i2[10];
    double i3;
};

void test_vector() {
    vector<int> vec(10, 5);
    copy(vec.begin(), vec.end(), ostream_iterator<int>(cout, "\n"));
    int* vec_pos = &vec[0];
    memset(vec_pos, 0, sizeof(int)*10);
    copy(vec.begin(), vec.end(), ostream_iterator<int>(cout, "\n"));
}

#define FPOS( type, field ) ( (size_t) &(( type *) 0)-> field ) 

void test_memcpy() {
    vector<char> buf;
    buf.push_back('a');
    const char* str = "hello world";
    copy(str, str+strlen(str)+1, back_inserter(buf));
    copy(str, str+strlen(str)+1, back_inserter(buf));
    //cout<<buf.size()<<endl<<string(&buf[0])<<endl;
    size_t pos = 0;
    for(size_t i=0;i<2;i++) {
        string str(&buf[pos]);
        cout<<str<<endl;
        cout<<str.size()<<endl;
        pos += str.size() + 1;
    }
    return;
    buf.resize(100);
    memcpy((void *)&buf[0], str, strlen(str)+1);
    cout<<string(&buf[0])<<endl;
}

void test_char(){
    vector<char> str1;
    string str2 = "1970-01-01 00:00:00";
    copy(str2.begin(), str2.end(), back_inserter(str1));
    str1.push_back('\0');
    const char* data_ptr = &str1[0];
    string str3 = string(data_ptr);

    cout<<str3.size()<<endl<<(string(data_ptr) == str2)<<endl;
}

void test_time() {
    struct   tm     *ptm; 
    long       ts; 
    int         y,m,d,h,n,s; 

    ts   =   time(NULL); 
    ptm   =   localtime(&ts); 

    y   =   ptm-> tm_year+1900;     //年 
    m   =   ptm-> tm_mon+1;             //月 
    d   =   ptm-> tm_mday;               //日 
    h   =   ptm-> tm_hour;               //时 
    n   =   ptm-> tm_min;                 //分 
    s   =   ptm-> tm_sec;                 //秒
    printf("%d-%d-%d %d:%d:%d\n", y, m, d, h, n, s);
}

void test_5file() {
    /// clean up the oldest data to keep lastest 5 data    
    if( ! KeepLatest("./data")) 
        LOG(LOG_ERROR, "fail");
}

void RemoveHtmlStr(std::string& src) {
    std::string::size_type pos_beg, pos_end;
    pos_beg = src.find('<'); 
    while(std::string::npos != pos_beg) {
        pos_end = src.find('>', pos_beg + 1);
        if(std::string::npos != pos_end) {
            src.replace(pos_beg, pos_end + 1 - pos_beg, "");    
            pos_beg = src.find('<', pos_beg);
        } else {
            break;
        }
    }
}

void test_html() {
    ifstream fin("./data");
    ofstream fout("./data.out");
    string str;
    while(!fin.eof()) {
        getline(fin, str);
        printf("%s\n", str.c_str());
        RemoveHtmlStr(str);
        printf("%s\n", str.c_str());
        fout<<str<<endl;
    }
}

void test_sizeof() {
    int a[20];
    cout<<sizeof(a)<<endl;
}

void test_systime() {
    string t1 = GetSysTime("2012-09-01 12:01:01", 2);
    string t2 = GetSysTime("20120310000000", 10, "%Y%m%d%H%M%S");
    cout<<t1<<" - "<<t1.size()<<endl;
    cout<<t2<<" - "<<t2.size()<<endl;
}

void test_pointer() {
    vector<string> vec1;
    vec1.resize(3);
    vec1[0] = "str1";
    vec1[1] = "str2";
    vec1[2] = "str3";
    string* ptr = &vec1[0];
    cout<<*ptr++<<endl;
    cout<<*ptr++<<endl;
    cout<<*ptr<<endl;
}

template<class T>
class TmpClass {
public:
    void test() {
        printf("var1:[%ld], var2:[%ld], var3:[%ld], var4:[%ld]\n",
            &var1, &var2, &var3, &var4);
    }
    T var5;
    vector<T>  var1;
    map<T, T> var2;
    T var3;
    vector<int> var4;
};

void test_template() {
    TmpClass<int> c;
    c.var1.push_back(1);
    c.var2[2] = 3;
    c.var3 = 4;
    c.var4.push_back(5);
    c.test();
    cout<<c.var1[0]<<endl;
    cout<<c.var4[0]<<endl;
    TmpClass<char>* c2 = (TmpClass<char>*)(&c);
    c2->test();
    TmpClass<char> c3;
    c3.test();
    cout<<c2->var1[0]<<endl;
    cout<<c2->var4[0]<<endl;
}

int main( int argc, char *argv[] ) {
    //test_time();
    //test_5file();
    //test_html();
    //test_sizeof();
    //test_systime();
    //test_pointer();
    test_template();
    return 0;
    LOG(LOG_ERROR, "Hello");
    cout<<TrimBoth(" hello ")<<endl;
    //cout<<FPOS(TS, i)<<endl<<FPOS(TS, i1)<<endl<<FPOS(TS, i2)<<endl<<FPOS(TS, i3)<<endl<<FPOS(TS, i4)<<endl;
    printf("%d %d\n", FPOS(TS, i2),sizeof(size_t));
    //cout<<_GetDbTime()<<endl;
    return 0;
}



