#!/usr/local/bin/python
#coding=utf-8

import sys
import os
import time
import urllib
import socket
import logging
import re
from optparse import OptionParser

def init_logger(logfile,loglevel):
    """
    init a logger for checker
    """
    #get a logger 
    logger = logging.getLogger()
    #create a handler to deal logger's info
    handler = logging.FileHandler(logfile)
    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
    handler.setFormatter(formatter)
    #init logger setting
    logger.addHandler(handler)
    logger.setLevel(loglevel)
    return logger

'''
Global variables
'''
#logger init
#g_logger = init_logger("../log/checker.log", logging.NOTSET)
g_logger = init_logger("../log/checker.log", logging.INFO)
g_port = 13780

''' return code '''
RET_OK = 0
RET_SERVER_DOWN = 1
RET_QUERY_ERROR = 2
RET_LOAD_FAIL = 3
RET_DATA_ERROR = 4
RET_TEST_FAIL = 5

def query(url):
    g_logger.debug(url)
    cnt = 3
    while(cnt):
        try:
            resp = urllib.urlopen(url)
            data = ''
            data = resp.read()
            resp.close()
            return data
        except Exception,e:
            g_logger.info("Fail to open url: [%s] temporarily"%url)
        time.sleep(cnt)
        cnt = cnt -1
    return ''

def check_service_ok():
    cnt = 15
    while(cnt > 0):
        if( len(query('http://127.0.0.1:%s/?_show_module=1'%g_port)) > 0):
            return True
        time.sleep(cnt)
        cnt = cnt - 1
    return False

def parse_module(module_info):
    print module_info
    module_list = re.findall(r"<module (.*?)></module>", module_info)
    modules = {}
    for module in module_list:
        #(name, full_tm, inc_num, static_doc_cnt, total_doc_cnt)
        list = re.findall(r"\"(.*?)\"", module)
        if(len(list) != 5):
            g_logger.error("Fail to parse module %s"%moudle)
        else:
            modules[list[0]] = list[1:5]
    return modules

def query_test():
    """
    Test the searcher from net query, here should be more comprehensive.
    It could be a group of unit test of modules, I'll think about it.
    """
    resp_data = query("http://127.0.0.1:%s/?q=1"%g_port)
    if(len(resp_data) < 200):
        return False
    g_logger.debug(resp_data)
    return True

def reload_module(module):
    if(module == "searcher"):
        # need no reload
        return True
    reload_info = query("http://127.0.0.1:%s/?_reload_module=%s"
        %(g_port, module))
    if(reload_info.find("OK") != -1):
        return True
    else:
        return False

def check_return(ret_code):
    sys.exit(ret_code)

def check_searcher(module, timestamp):
    if( check_service_ok() != True) :
        g_logger.error("Fail to query searcher")
        check_return(RET_SERVER_DOWN)
    if( reload_module(module) != True):
        g_logger.info("Fail to reload module data [%s]"%module)
        check_return(RET_LOAD_FAIL)
    if( check_service_ok() != True) :
        g_logger.error("Fail to query searcher")
        check_return(RET_SERVER_DOWN)
    module_info = query("http://127.0.0.1:%s/?_show_module=1"%g_port)
    if( '' == module_info):
        g_logger.error("Fail to query searcher")
        check_return(RET_QUERY_ERROR)
    modules = parse_module(module_info)
    #check if the modules data is updated rightly
    if( modules.has_key(module) != True):
        g_logger.info("Fail to load module data [%s]"%module)
        check_return(RET_LOAD_FAIL)
    if(modules[module][0] != timestamp):
        g_logger.info("full timestamp is different [%s] vs [%s]"
            %(modules[module][0], timestamp))
        check_return(RET_DATA_ERROR)
    if(query_test() != True):
        g_logger.info("Fail to pass test")
        check_return(RET_TEST_FAIL)
    check_return(RET_OK)
    
def init_opt_parser():
    parser = OptionParser() 
    parser.add_option("-m", "--module", action="store_true", dest="module", 
        default=False, help="module name of searcher") 
    parser.add_option("-f", "--full_timestamp", action="store_true", 
        dest="full_timestamp", default=False, 
        help="full timestamp of module of searcher")
    parser.add_option("-p", "--port", action="store_true", 
        dest="port", default=False, 
        help="listen port of searcher")
    return parser

if __name__ == "__main__":
    parser = init_opt_parser()
    (options, args) = parser.parse_args() 
    if(len(args) != 3):
        g_logger.info("Wrong optins parsed in")
        parser.print_help()
    else:
        g_port = args[2]
        check_searcher(args[0], args[1])
