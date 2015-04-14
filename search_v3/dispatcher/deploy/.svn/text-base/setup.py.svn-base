#!/usr/local/bin/python
#coding=utf-8

import sys
import os
from optparse import OptionParser

def get_segment_data():
    '''get the segment data'''
    seg_path = 'download_files/segment'
    if os.path.exists(seg_path):
        if os.path.exists(seg_path + '/.svn'):
            print "segment data is already exist, here will update it from "\
                "svn server"
            if( 0 != os.system('cd %s;svn up;cd -;'%seg_path)):
                print 'Fail to update segment data from 192.168.85.135'
                return False
            return True
        else:
            os.system('rm -rf %s'%seg_path)
    print 'download segment data from 192.168.85.135'
    if( 0 != os.system('svn co svn://192.168.85.135/data/search_v3_data/'
        'segment %s'%seg_path)):
        print 'Fail to download data from 192.168.85.135'
        return False
    return True

def get_updater_files():
    '''get the updater files, which is smelly'''
    up_path = 'download_files/updater'
    if os.path.exists(up_path):
        if os.path.exists(up_path + '/.svn'):
            print 'updater files is already exist, here will update it from' \
                'svn server'
            if( 0 != os.system('cd %s;svn up;cd -;'%up_path)):
                print 'Fail to update updater files from 192.168.85.135'
                return False
            return True
        else:
            os.system('rm -rf %s'%up_path)
    print 'download updater files from 192.168.85.135'
    if( 0 != os.system('svn co svn://192.168.85.135/projects/search_v3/'
        'updater %s'%up_path)):
        print 'Fail to download files from 192.168.85.135'
        return False
    return True

def get_searcher_files():
    '''get the updater files'''
    up_path = 'download_files/searcher'
    if os.path.exists(up_path):
        if os.path.exists(up_path + '/.svn'):
            print 'searcher files is alread exist, here will update it from' \
                'svn server'
            if( 0 != os.system('cd %s;svn up;cd -;'%up_path)):
                print 'Fail to update updater files from 192.168.85.135'
                return False
            return True
        else:
            os.system('rm -rf %s'%up_path)
    print 'download searcher files from 192.168.85.135'
    if( 0 != os.system('svn co svn://192.168.85.135/projects/search_v3/' \
        'deploy/ %s'%up_path)):
        print 'Fail to download files from 192.168.85.135'
        return False
    return True

def get_package_files():
    if( True != get_segment_data()):
        return False
    if( True != get_updater_files()):
        return False
    if( True != get_searcher_files()):
        return False
    return True

def setup(path, install_flag):
    print "setup dispatcher to [%s] ..."%path
    print install_flag
    work_base = path + '/dispatcher/' 
    if( os.path.exists(work_base)):
        print "work_base %s is exist, here remove it"%work_base
        os.system("rm -rf %s"%work_base)
    os.system("mkdir -p %s"%work_base)

    print 'get package files'
    if( True != get_package_files()):
        sys.exit(1)

    print 'deploy dispatcher/bin'
    bin_path = work_base + '/bin'
    os.system('mkdir -p %s/{tools,data,index,lib,modules}'%bin_path)
    os.system('cp ../dispatcher/%s/dispatcher %s/'%(install_flag, bin_path))
    os.system('cp ../dispatcher/dispatcher.conf %s/'%bin_path)
    os.system('cp ../searcher_checker/checker.py %s/'%bin_path)
    os.system('cp ../db_reader/%s/reader_for_search %s/'
        %(install_flag, bin_path))
    os.system('cp ../business_layer/%s/business_layer.so %s/'
        %(install_flag, bin_path))
    os.system('cp package_files/lib/libmysqlclient.so.16 %s/lib/'%bin_path)
    os.system('cp package_files/lib/libboost_system.so.1.40.0  %s/lib/'
        %bin_path)
    os.system('cp download_files/segment/ %s/ -r'%bin_path)
    os.system('cp download_files/searcher/conf/search.conf %s/'%bin_path)
    
    print 'deploy manage tools dispatcher/bin/tools/'
    os.system('cp package_files/deploy.py %s/tools/'%bin_path)
    os.system('cp package_files/cleanup.sh %s/tools/'%bin_path)
    os.system('cp package_files/log_cleaner.sh %s/tools/'%bin_path)
    os.system('cp package_files/dispatcher_nginx.conf %s/tools'%bin_path)
    os.system('cp ../db_reader/db_reader.sh %s/tools/'%bin_path)
    if(install_flag=="debug"):
        os.system('cp ../dispatcher/debug/test_dispatcher.exe %s/tools/'
            %bin_path)
        os.system('cp ../common_data_handler/debug/' \
            'test_common_data_handler.exe %s/tools/'%bin_path)
        os.system('cp ../data_handler/debug/test_searcher.exe %s/tools/'
            %bin_path)
    
    print 'deploy dispatcher/bin/modules/'
    md_path = bin_path + '/modules'
    os.system('mkdir -p %s/{searcher,common}'%md_path)
    os.system('mkdir -p %s/searcher/module_data/'%md_path)
    os.system('mkdir -p %s/common/module_data/'%md_path)
    os.system('cp ../data_handler/%s/searcher.so %s/searcher'
        %(install_flag, md_path))
    os.system('cp ../data_handler/searcher.conf %s/searcher'%md_path)
    os.system('cp ../common_data_handler/%s/common_data_handler.so %s/common'
        %(install_flag, md_path))
    os.system('cp ../common_data_handler/common_data_handler.conf %s/common'
        %md_path)

    print 'deploy searcher'
    sc_path = work_base + '/search_v3'
    os.system('mkdir -p %s/{data,index,modules,tools}'%sc_path)
    os.system('cp download_files/segment/ %s/ -r'%sc_path)
    os.system('cp download_files/searcher/conf/search.conf %s/'%sc_path)
    os.system('cp ../db_reader/%s/reader_for_search %s/'%
        (install_flag, sc_path))
    os.system('cp download_files/updater/updater.py %s/ -f'%sc_path)
    os.system('cp download_files/updater/util.py %s/ -f'%sc_path)
    os.system('cp download_files/updater/updater.conf %s/ -f'%sc_path)
    os.system('cp download_files/searcher/tools %s/ -rf'%sc_path)
    
    print 'deploy dispatcher/data/'
    data_path = work_base + '/data'
    os.system('mkdir -p %s/{cache_data,main_data}'%data_path)
    os.system('mkdir -p %s/cache_data/{dispatcher,searcher,common}'%data_path)
    os.system('mkdir -p %s/main_data/{searcher,common}'%data_path)

    print 'deploy dispatcher/log/'
    log_path = work_base + '/log/'
    os.system('mkdir -p %s/'%log_path)

    print 'clean up useless files'
    os.system("find %s -type d -name '.svn' | xargs rm -rf "%work_base)

    print "done to setup"

def init_opt_parser():
    parser = OptionParser()
    parser.add_option("-p", "--path", action="store", dest='path',
        default='./dispatcher_install', help='the install path')
    parser.add_option("-f", "--install_flag", action="store", 
        dest='flag', default='release', help="install flag(debug, release)")
    return parser

if __name__ == "__main__":
    parser = init_opt_parser()
    (options, args) = parser.parse_args()
    setup(options.path, options.flag)
