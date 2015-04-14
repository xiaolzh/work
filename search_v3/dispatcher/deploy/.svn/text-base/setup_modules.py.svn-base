#!/usr/local/bin/python
#coding=utf-8

import sys
import os
from optparse import OptionParser

def svn_rev(path):
    fp = os.popen('svn info %s|grep "Last Changed Rev"|cut -d \' \' -f 4'
        %path)
    return fp.readlines()[0].strip()

def get_search_module_package():
    up_path = '../../search_module/'
    if( 0 != os.system('cd %s;svn up;sh so.sh;cd -;'%up_path)):
        print 'Fail to make search_v3 modules'
        return False
    print 'OK to make search_v3 modules'
    return True

def get_query_pack_data():
    '''get data of query pack module'''
    data_path = 'download_files/query_pack'
    if( os.path.exists(data_path)):
        print 'updater files is already exist, here will update it from' \
            'svn server'
        if( 0 != os.system('cd %s;svn up;cd -;'%data_path)):
            print 'Fail to update query_pack data files from 192.168.85.135'
            return False
        return True
    print 'download query_pack data files from 192.168.85.135'
    if( 0 != os.system('svn co svn://192.168.85.135/data/search_v3_data/' \
        'query_pack %s'%data_path)):
        print 'Fail to download files from 192.168.85.135'
        return False
    return True

def get_search_ranking_data():
    '''get data of search_ranking module'''
    data_path = 'download_files/search_ranking/'
    if( os.path.exists(data_path)):
        print 'updater files is already exist, here will update it from' \
            'svn server'
        if( 0 != os.system('cd %s;svn up;cd -;'%data_path)):
            print 'Fail to update query_pack data files from 192.168.85.135'
            return False
        return True
    print 'download query_pack data files from 192.168.85.135'
    if( 0 != os.system('svn co svn://192.168.85.135/data/search_v3_data/' \
        'search_ranking/ %s'%data_path)):
        print 'Fail to download files from 192.168.85.135'
        return False
    return True

def get_package_files(build_type):
    if( True != get_search_module_package()):
        return False
    if (build_type & 0x4) != 0:
        if( True != get_query_pack_data()):
            return False
    if (build_type & 0x2) != 0:
        if( True != get_search_ranking_data()):
            return False
    return True

def setup(path, install_flag, module):
    print "setup dispatcher data modules to [%s] ..."%path
    print 'mode: [%s], module: [%s]'%(install_flag, module)
    work_base = path + '/data_modules' 
    if( os.path.exists(work_base)):
        print "work_base %s is exist, here remove it"%work_base
        os.system("rm -rf %s"%work_base)
    os.system("mkdir -p %s"%work_base)

    if( module == 'all'): build_type = 0x7
    if( module == 'list_ranking'): build_type = 0x1
    if( module == 'search_ranking'): build_type = 0x2
    if( module == 'query_pack'): build_type = 0x4

    print 'get package files'
    if( True != get_package_files(build_type)):
        sys.exit(1)

    md_path = '../../search_module/'

    if (build_type & 0x1) != 0:
        print 'deploy list_ranking module'
        lr_path = work_base + '/list_ranking/'
        os.system('mkdir -p %s'%lr_path)
        os.system('cp ../common_data_handler/%s/common_data_handler.so ' \
            '%s/list_ranking.so'%(install_flag, lr_path))
        os.system('cp ../common_data_handler/data_list_ranking.conf ' \
            '%s/common_data_handler.conf'%lr_path)
        os.system('cp ../common_data_handler/process_list_ranking_data.sh ' \
            '%s/'%lr_path)
        os.system('cp -rf %s/list_ranking/list_ranking %s/module_data'
            %(md_path, lr_path))
        print 'clean up useless files'
        os.system("find %s -type d -name '.svn' | xargs rm -rf "%work_base)
        data_rev = svn_rev('../common_data_handler')
        so_rev = svn_rev('%s/list_ranking'%md_path)
        os.system('cd %s;tar zcvf list_ranking_%s_d%s_s%s.tar.gz ' \
            'list_ranking/;cd -'%(work_base, install_flag, data_rev, so_rev))
        
    if (build_type & 0x4) != 0:
        print 'deploy query_pack module'
        qp_path = work_base + '/query_pack/'
        os.system('mkdir -p %s/module_data'%qp_path)
        os.system('cp ../common_data_handler/%s/common_data_handler.so ' \
            '%s/query_pack.so'%(install_flag, qp_path))
        os.system('cp ../common_data_handler/data_query_pack.conf ' \
            '%s/common_data_handler.conf'%qp_path)
        os.system('cp %s/query_pack/query_pack.so %s/module_data'
            %(md_path, qp_path))
        os.system('cp %s/query_pack/show_result.conf %s/module_data'
            %(md_path, qp_path))
        os.system('cp download_files/query_pack/static_data/* ' \
            '%s/module_data -rf'%qp_path)
        os.system('cp download_files/query_pack/pack_data/ %s/ -rf'%qp_path)
        print 'clean up useless files'
        os.system("find %s -type d -name '.svn' | xargs rm -rf "%work_base)
        data_rev = svn_rev('../common_data_handler')
        so_rev = svn_rev('%s/query_pack'%md_path)
        os.system('cd %s;tar zcvf query_pack_%s_d%s_s%s.tar.gz query_pack/;' \
            'cd -'%(work_base, install_flag, data_rev, so_rev))

    if (build_type & 0x2) != 0:
        print 'deploy search_ranking module'
        sr_path = work_base + '/search_ranking/'
        os.system('mkdir -p %s/module_data'%sr_path)
        os.system('cp ../common_data_handler/%s/common_data_handler.so ' \
            '%s/search_ranking.so'%(install_flag, sr_path))
        os.system('cp ../common_data_handler/data_search_ranking.conf ' \
            '%s/common_data_handler.conf'%sr_path)
        os.system('cp ../common_data_handler/process_search_ranking_data.sh ' \
            '%s/'%sr_path)
        os.system('cp %s/search_ranking/search_ranking.so %s/module_data'
            %(md_path, sr_path))
        os.system('cp download_files/search_ranking/static_data/* ' \
            '%s/module_data -rf'%sr_path)
        os.system('cp download_files/search_ranking/FB/ %s/ -rf'%sr_path)
        print 'clean up useless files'
        os.system("find %s -type d -name '.svn' | xargs rm -rf "%work_base)
        data_rev = svn_rev('../common_data_handler')
        so_rev = svn_rev('%s/search_ranking'%md_path)
        os.system('cd %s;tar zcvf search_ranking_%s_d%s_s%s.tar.gz ' \
            'search_ranking;cd -'%(work_base, install_flag, data_rev, so_rev))

    print "done to setup"

def init_opt_parser():
    parser = OptionParser()
    parser.add_option("-p", "--path", action="store", dest="path",
        default='./dispatcher_install', help="the install path")
    parser.add_option("-f", "--install_flag", action="store", 
        dest="flag", default='release', help="install flag(debug, release)")
    parser.add_option('-m', '--module_name', action='store',
        dest='module', default='all', 
        help='module name(all, list_ranking, search_ranking, query_pack)')
    return parser

if __name__ == "__main__":
    parser = init_opt_parser()
    (options, args) = parser.parse_args()
    setup(options.path, options.flag, options.module)
