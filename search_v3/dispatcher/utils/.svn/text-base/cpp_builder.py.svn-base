#!/usr/bin/env python
#coding=utf-8

import sys
import re
import traceback
import pdb
import os

head_suffix="h"
source_suffix="cpp"

head_file_include_tpl="""\
#include "%s"
%s
"""

class_define_tpl="""\
class %s {
    public:
        %s();
        ~%s();

        /// member functions define
        // bool func(); like this
    private:
        /// members define
        //int m_val; like this
        /// Disallow copy and assign defaultly
        DISALLOW_COPY_AND_ASSIGN(%s);
};
"""

class_source_tpl="""\
/// %s construction
%s::%s() {
    // TODO: initialize
}

/// %s destruction
%s::~%s() {
    // TODO: co-initialize
}
"""

unit_test_tpl="""\
#ifdef UNIT_TEST
/// include head files for unit test
%s

bool test_%s() {
    cout<<"Unit test - %s"<<endl;
    {
        bool ret = true;
        cout<<"usecase 1: "<<endl;
        // TODO: add your test code here
        cout<<boolalpha<<ret<<endl;
        if( !ret)
            return false;
    }
    cout<<"Done unit test - %s"<<endl;
    return true;
}

#ifdef TEST_%s
int main() {
    if(!test_%s())
        return -1;
    return 0;
}
#endif // TEST_%s
#endif // UNIT_TEST

// I wanna sing a song for you, dead loop no escape
"""

head_tpl="""\
#ifndef %s
#define %s
/// head files include
%s
/// namespace to limit the scope
%s
#endif // ~>.!.<~ 
"""

def write_head_file(filename):
    file_lst=[filename]*10
    f=open(filename+'.'+head_suffix, "w")
    file_code=""
    include_code = head_file_include_tpl%("swstd.h","")
    class_code = class_define_tpl%tuple(file_lst[0:4])
    define_guard_code = (filename+'_'+head_suffix).upper()
    file_code = head_tpl%(define_guard_code, define_guard_code, include_code, class_code)
    f.write(file_code)
    f.close()

def write_source_file(filename):
    file_lst=[filename]*10
    f=open(filename+'.'+source_suffix, 'w')
    file_code=''
    include_code = head_file_include_tpl%(filename+'.'+head_suffix, '')
    class_code = class_source_tpl%tuple(file_lst[0:6])
    test_include_code = head_file_include_tpl%('iostream', 'using namespace std;')
    test_code = unit_test_tpl%(test_include_code, filename, filename, filename, filename.upper(), filename, filename.upper())
    file_code = include_code + class_code + test_code
    f.write(file_code)
    f.close()

def main(filename):
    write_head_file(filename)
    write_source_file(filename)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print "Wrong Parameters, please check!"
    else:
        main(sys.argv[1])
