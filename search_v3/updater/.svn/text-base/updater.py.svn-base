#!/usr/local/bin/python
#coding=utf-8

import os
import time
import datetime
import logging
import commands
import pdb
import ConfigParser
import socket
import fcntl
import struct
import random
from random import Random,seed,choice,randint

from os.path import isfile, isdir,exists
from urllib import urlencode
from multiprocessing import Process,Lock
from util import *
import commands
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Global:val~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
OK = 0
FAIL = -1
WARNING = 1
OTHER = 2

GLOBAL_MONITOR_SIGNAL=True
#set connection time out.
socket.setdefaulttimeout(15)

#~~~~~~~~~~~~~random sleep time~~~~~~~~~~~~~~~
def get_ip_address(name="eth0"):
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        return socket.inet_ntoa(fcntl.ioctl(s.fileno(),0x8915,struct.pack('256s', name[:15]))[20:24])
    except Exception,e:
        logger.exception(e)
        return None

def random_delay():
    try:
        ip_str = get_ip_address("eth0")
        if ip_str is None:
            logger.warn("can not get ip address.")
            return random.randint(0,60)*30
        ip_val = int(ip_str.replace(".",""))
        ran = Random()
        ran.seed(ip_val)
        # [0~1]hour.
        return ran.randint(0,60)*30
    except Exception,e:
        logger.exception(e)
        return None
#~~~~~~~~~~~~~~check update pid~~~~~~~~~~
#base on running pid get it's pwd.
def get_pidcwd(pid):
    try:
        dir = None
        cmd = "ls -l /proc/%s/cwd"%(str(pid))
        status,output = commands.getstatusoutput(cmd)
        if(output.find("->") < 0):
            return True,None
        dir = (output.split("->")[-1]).strip()
        return True,dir
    except Exception,e:
        logger.exception(e)
        return False,None

def get_pids(cmd):
    try:
        p = os.popen(cmd)
        tmp_data = p.readlines()
        p.close()
        pids ={}
        for iterm in tmp_data:
            pid = iterm.strip()
            pids[pid] = pid
        return pids
    except Exception,e:
        logger.exception(e)
        return {}

def check_updaterpids():
    try:
        pwd = os.getcwd()
        pids_file = "%s/pid.updater"%pwd
        if( not exists(pids_file)):
            print "file[%s] not exists."%pids_file
            return True
        #Get pids in file 'pid.updater'
        cmd = "grep '[\\d+]*' %s|awk '{print $1}' "%pids_file
        pids = get_pids(cmd)
        #Get running updater's pids
        #cmd =  "ps -x|grep  -w '%s/updater\\.py' |awk '{print $1}' "%pwd
        cmd =  "ps x|grep  -w 'updater\\.py' |awk '{print $1}' "
        run_pids =get_pids(cmd) #最少有一个updater进程,即当前运行的updater
        #check pid
        for pid in run_pids:
            if(pids.has_key(pid)):
                flag,dir = get_pidcwd(pid) #get running pid's pwd.
                if flag and dir is not None:
                    if (dir == pwd):
                        print "can't run two updater.under the dir[%s]"%pwd
                        return False
        return True
    except Exception,e:
        logger.exception(e)
        return False

def update_pid():
    try:
        pid = os.getpid()
        pwd = os.getcwd()
        pids_file = "%s/pid.updater"%pwd
        #print "start pid:%d"%pid
        fout = open(pids_file,"a")
        fout.write(str(pid) + "\n")
        fout.close()
        return True
    except Exception,e:
        logger.exception(e)
        return False
#~~~~~~~~~~~~query~~~~~~~~~
def query(url):
    retry = 3
    while(retry > 0):
        open_error, page = url_open(url)
        try:
            if page:
                data = page.read()
                page.close()
                return data
        except Exception,e:
            logger.info(e)
        retry = retry -1
        time.sleep(2<<retry)
    logger.warning('query failed:' + url)
    return None

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Function:start~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
def request_parser(request_domain,request_port,module,request_params):
    """
    get request
    """
    try:
    #get update request data
        request_par = urlencode(request_params)
        logger.debug("http://%s:%s/?%s"%(request_domain,request_port,request_par))
        data = query("http://%s:%s/?%s"%(request_domain,request_port,request_par))
        if data is None: return {}

        #parse request data for update info like time_stamp ...
        source_data = data.split("><")
        result = {} 
        #find the module info which we need
        for source in source_data:
            if source.find("module") != -1 and source.find(module) != -1:
                tmp = source.replace("module name","modulename")
                values = tmp.split()
                for item in values:
                    key =item.split("=")[0]
                    value = (item.split("=")[-1]).strip('"')
                    if key is None:
                        continue
                    result[key]=value
                break 
        return result
    except Exception,e:
        logger.exception(e)
        return {}

def response_parser(response_domain,response_port,module,response_params):
    """
    get response 
    """
    try:
        #get update response data
        response_par = urlencode(response_params)
        logger.info("request: http://%s:%s/?%s"%(response_domain,response_port,response_par))
        data = query("http://%s:%s/?%s"%(response_domain,response_port,response_par))
        if data is None: return []

        #parse response data to build up a list
        results = []
        result = {}
        source_head = ""
        source_data = data.split("\0")
        #need judge if data in a row and log here
        #if not in a row write a log
        index = 0
        key_val = None
        for source in source_data:
            if source:
                #print "{%s:%s}"%([key_val],[source])
                if key_val:
                    if key_val.find("//") == 0:
                        source_head = key_val 
                    #elif key_val.find("@") == 0:
                    #    result.setdefault(key_val.replace("@","").replace(" ","_"),source)
                    elif key_val.find("#") == 0:
                         pass
                    elif key_val == "\n":
                        if result:
                            result.setdefault("source_head",source_head) 
                            #print result
                            results.append(result)
                            result = {}
                    else: 
                         result.setdefault(key_val.replace(" ","_"),source)
                    key_val = None
                else:
                    key_val = source
        return results
    except Exception,e:
        logger.exception(e)
        return []

def check_result(e_domain,e_port,module,post):
    """
    check search engine service
    """
    try:
        #create url params
        get_params = {}
        get_params['_show_module'] = 1
    
        #check reload or searcher engine restart result
        reload_result = request_parser(e_domain,e_port,module,get_params)
        if reload_result and reload_result.has_key("full_tm") and reload_result["full_tm"] == post["main_stamp"]:
            result = OK
        else:
            result = FAIL

        #check search key:linux
        if module == "searcher":
            check_params = {"q":"linux", 'st':'full'}
            check_par = urlencode(check_params)
            check_data = query("http://%s:%s/?%s"%(e_domain,e_port,check_par))
            if check_data is None:
                logger.warning("searcher engine restart fail")
                return FAIL

            s = check_data.find("<TotalCnt>")
            e = check_data.find("</TotalCnt>")
            if e > s:
                total_cnt = int(check_data[(s+10):e])
                if total_cnt > 10:
                    result = OK
                else:
                    logger.warning("can't find totalcnt in page http://%s:%s/?%s"%(e_domain,e_port,check_par))
                    result = FAIL
            else:
                logger.warning("can't find totalcnt in page http://%s:%s/?%s"%(e_domain,e_port,check_par))
                result = FAIL

        return result        
    except Exception,e:
        logger.exception(e)
        return FAIL

def check_md5(folder):
    try:
        if not isdir(folder):
            logger.warning("data path is not a folder [%s]"%folder)
            return False
        md5file_name = folder + '/_md5'
        if not isfile(md5file_name):
            logger.warning("md5 file is not found [%s]"% md5file_name)
            return False
        md5file = open(md5file_name, 'r')
        doc = md5file.readlines()
        md5file.close()
        for line in doc:
            filename = folder + '/' + line.split()[1]
            status, output = commands.getstatusoutput('md5sum %s |awk \'{print $1}\''%filename)
            if( output != line.split()[0]):
                logger.warning("md5sum of %s is not equal"%filename)
                return False
        return True
    except Exception,e:
        logger.exception(e)
        return False
    
def back_data(module):
    try:
        if module=="searcher":
            if( os.path.exists("./index_backend")):
                os.system("rm -r index_backend")
            if( os.path.exists("./index_back")):
                os.system("mv ./index_back ./index_backend")
            if( os.path.exists("./index")):
                os.system("mv ./index ./index_back")
            if( os.path.exists("./index_new")):
                os.system("mv ./index_new ./index")
        else:
            modules_path="./modules_backend/"+module+"_backend"
            if( os.path.exists(modules_path)): 
                os.system("rm -r %s"%(modules_path))
            if( os.path.exists("./modules_back/%s_back"%(module))):
                #make modules_backend dir.
                if (not os.path.exists("./modules_backend/")):
                    os.system("mkdir ./modules_backend/")
                os.system("mv ./modules_back/%s_back  ./modules_backend/%s_backend"%(module,module))
            if( os.path.exists("./modules/%s"%(module))):
                #make modules_back dir.
                if (not os.path.exists("./modules_back/")):
                    os.system("mkdir ./modules_back/")
                os.system("mv ./modules/%s ./modules_back/%s_back"%(module,module))
            if( os.path.exists("./modules_new/%s_new"%(module))):
                os.system("mv ./modules_new/%s_new ./modules/%s"%(module, module))

    except Exception,e:
        logger.exception(e)

def rollback_data(module):
    try:
        if module=="searcher":
            path = "./index"  
            if( os.path.exists(path)):
                os.system("rm -rf %s"%(path))
            if( os.path.exists(path+"_back")):
                os.system("mv %s_back %s"%(path, path))
            if( os.path.exists(path+"_backend")):
                os.system("mv %s_backend %s_back"%(path, path))
        else:
            path = module
            if( os.path.exists("./modules/%s"%(path))):
                os.system("rm -rf ./modules/%s"%(path))
            if( os.path.exists("./modules_back/%s_back"%(path))):    
                os.system("mv ./modules_back/%s_back ./modules/%s"%(path, path))
            if( os.path.exists("./modules_backend/%s_backend"%(path))):
                if (not os.path.exists("./modules_back/")):
                    os.system("mkdir ./modules_back/")
                os.system("mv ./modules_backend/%s_backend ./modules_back/%s_back"%(path, path))

    except Exception,e:
        logger.exception(e)

def stop_searcher(thread_num,e_port):
    try:
        os.system("Searcher -n %s -p %s -k stop"%(thread_num,e_port))
        os.system("""index=0;\
                     kflag=0;\
                     while [ $index -lt 20 ];do\
                         pid=`ps awx -o '%p %P'|grep -w -f masterDaemon.pid.Searcher`;\
                         if [ "x$pid" = "x" ];then\
                             kflag=1;\
                             index=20;\
                         else
                             kflag=0;\
                         fi;\
                         sleep 1;\
                         ((index++));\
                         echo $index;\
                     done;\
                     if [ $kflag = 0 ];then\
                         ps awx -o '%p %P'|grep -w -f masterDaemon.pid.Searcher| awk '{ print $1 }'|xargs kill -9;\
                     fi;""")
        return OK
    except Exception,e:
        logger.exception(e)
        return FAIL

def full_update(e_domain,e_port,module,data,thread_num):
    """
    full update from data_path
    """
    try:
        #init check parameter
        get_params = {}
        get_params['_show_module'] = 1

        #module full update except searcher
        if module != "searcher":
            #wget full data from dispathcer
            #logger.info('wget -r -np -nd -q -P ./modules/%s_new -R "index.html*" %s'%(module, data["data_path"]))
            if os.path.exists("./modules_new/%s_new"%(module)):
                os.system('rm -rf ./modules_new/%s_new'%(module))
            if (not os.path.exists("./modules_new")):
                os.system("mkdir ./modules_new")
            os.system('mkdir -p ./modules_new/%s_new;'%(module))
            wget_info = os.system(('wget -r --limit-rate=10M --no-dns-cache -np -nd -q -P ./modules_new/%s_new -R "index.html*" %s')\
                             %(module,data["data_path"]))
            if wget_info:
                logger.warning(" %s module:main stamp wget error:%s. wget failed  from %s"%(module,wget_info,data["data_path"]))
                return WARNING 
            #need check md5 here
            if not check_md5("./modules_new/%s_new"%module):
                #logger.warning("%s md5 check failed"%module)
                return WARNING

            if module=="list_ranking":
                os.system("chmod +x ./modules_new/list_ranking_new/formula_analyzer")

            #back data
            back_data(module)
        
            #restart module
            page = query("http://%s:%s/?_reload_module=%s"%(e_domain,e_port,module))
            if page is None:
                return WARNING
            if page.find('OK') < 0: return WARNING
            # a patch to avoid not reload yet
            #result = check_result(e_domain,e_port,module,data)
        #searcher module full update need restart engine
        else:
            #wget data from dispatcher
            #logger.info('wget -r -np -nd -q -P ./index_new -R "index.html*" %s'%data["data_path"]) 
            if os.path.exists("./index_new"):
                os.system('rm -r ./index_new;')
            os.system('mkdir ./index_new;')
            wget_info = os.system(('wget -r --limit-rate=10M --no-dns-cache -nd -np -q -P ./index_new -R "index.html*" %s')%(data["data_path"]))
            if wget_info:
                logger.warning("main stamp wget error:%s wget failed from %s"%(wget_info,data["data_path"]))
                return WARNING
            #need check md5 here
            if not check_md5("./index_new"):
                #logger.warning("index md5 check failed")
                return WARNING

            #back data & restart engine
            stop_flag = stop_searcher(thread_num,e_port)
            if stop_flag <0:
                logger.error("Stop Searcher fail.")
                return FAIL
            back_data(module)
            os.system("nohup Searcher -n %s -p %s -k start >>supplier_search.out &"%(thread_num,e_port))

        result = check_result(e_domain,e_port,module,data)
        return result
    except Exception,e:
        logger.exception(e)
        return FAIL
   
def full_update_rollback(e_domain,e_port,module,data,thread_num):
    """
    rollback data when full update has some error
    """
    try:
        if module == "searcher":
            path_con = os.listdir("index_back")
            stop_flag = stop_searcher(thread_num,e_port)
            if stop_flag <0:
                logger.error("Stop Searcher fail.")
                return FAIL
            if path_con:
                rollback_data(module)
            else:
                logger.error("can't rollback data ,index_back dir is empty.")
                return FAIL
            os.system("nohup Searcher -n %s -p %s -k start >>supplier_search.out &"%(thread_num,e_port))
        else:
            path_con = os.listdir("./modules_back/%s_back"%module)
            if path_con:
                rollback_data(module)
            else:
                logger.error("can't rollback data ,%s_back dir is empty"%module)
                return FAIL

    
        #check rollback result
        retry =1
        data=None
        # retry 10 times to check if the searcher is restart sucessfully or not.
        while(True):
            data = query("http://%s:%s/?_reload_module=%s"%(e_domain,e_port,module))
            if data or retry > 10:
                break
            else:
                retry = retry+1
                time.sleep(10)
        if data is None:
            return FAIL
        if data.find('OK') < 0: return FAIL
        return OK
    except Exception,e:
        logger.exception(e)
        return FAIL

def inc_update(e_domain,e_port,module,data,full_timestamp):
    """
    inc update from data
    """
    try:
        del(data["source_head"])
        if data.has_key("@info"):
            del(data["@info"])
        index_type = data.pop("@type")
    
        #add modify_ for modify request
        for k,v in data.items():
            if k.find("@") == 0:
                nk = k[1:]
                nv = v
                del(data[k])
                data.setdefault(nk,nv)
            elif index_type == "modify":
                nk = "_modify_"+k
                nv = v
                del(data[k])
                data.setdefault(nk,nv)

        #add some other info for response url
        if index_type == "delete":
            data.setdefault("_index_del","1")
        else:
            data.setdefault("_index_"+index_type,"1")

        data.setdefault("_inc_num",data["stamp"])
        del(data["stamp"])
        data.setdefault("_full_timestamp",full_timestamp)
    
        #add modify_ for modify request
        inc_par = urlencode(data)
        #logger.info("inc_update: http://%s:%s/?%s"%(e_domain,e_port,inc_par))
        inc_data = query("http://%s:%s/?%s"%(e_domain,e_port,inc_par))
        #if inc_data is None: return ""
        #if inc_data !="<result>OK</result>":
        #    url = "inc stamp:http://%s:%s/?%s"%(e_domain,e_port,inc_par)
        #    logger.warning("module[%s] inc update failed:%s. URL: %s"%(module,inc_data,url))
        cycleCnt =1
        url = "inc stamp:http://%s:%s/?%s"%(e_domain,e_port,inc_par)
        # it always does inc update  util inc update succeeded.
        while True:
            if inc_data is not None and inc_data == "<result>OK</result>":
                break;
            if cycleCnt%10 == 0:
                cycleCnt = 1
                logger.warning("module[%s] inc update failed:%s. URL: %s"%(module,inc_data,url))
            else: cycleCnt = cycleCnt +1
            inc_data = query("http://%s:%s/?%s"%(e_domain,e_port,inc_par))
        return inc_data
    except Exception,e:
        logger.exception(e)
        return e

def func(thread_num,e_module,d_module,lock,e_domain="",e_port="",d_domain="",d_port="",delay_time=1):
    """
    get module request from engine and post to dispather
    """
    #create url params
    get_params = {}
    get_params['_show_module'] = 1

    run_tag = True   
    while run_tag:
        inc_update_flag=False
        full_upate_flag=False
        time1 = time.time()
        #get module request from engine
        get_data = request_parser(e_domain,e_port,e_module,get_params)
        #post pathcer
        post_params = {}
        if get_data and get_data.has_key("modulename") and (get_data["modulename"] == e_module):
            #create post url params
            post_params['main_stamp'] = get_data["full_tm"]
            post_params['inc_stamp'] = get_data["inc_num"]
            #post_params['module'] = get_data["modulename"]
            post_params['module'] = d_module
            full_timestamp = get_data["full_tm"]
            logger.debug(get_data)
            #get response data from dispather
            post_data = response_parser(d_domain,d_port,d_module,post_params) 
            post_flag = -1
            #print post_data

            inc_cnt = 0
            for post in post_data:
                if post["source_head"] == "//Header":
                    if post["@status_code"] != "0":
                        if(post["@status_code"] == "2"):
                            logger.info("module[%s-%s] need do main stamp [%s]"%(e_module,d_module,post_params['main_stamp']))
                        elif(post["@status_code"] == "4"):
                            logger.debug("dispacther module[%s] no inc data yet [%s]"%(d_module,post_params['inc_stamp']))
                        else:
                            logger.warning("dispatcher %s:%s"%(post["@status_code"],post["@status_info"]))
                else:
                    #do full update
                    if post.has_key("data_path"):
                        #Searcher random sleep.
                        full_upate_flag=True
                        time.sleep(delay_time)
                        lock.acquire()
                        GLOBAL_MONITOR_SIGNAL=False
                        full_tag = full_update(e_domain,e_port,e_module,post,thread_num)
                        if full_tag < 0:
                            logger.warning("module[%s-%s] do full_update fail:%s"%(e_module,d_module,full_tag))
                            rollback_tag = full_update_rollback(e_domain,e_port,e_module,post,thread_num)
                            if rollback_tag < 0:
                                logger.error("rollback error %s module service down!"%e_module)
                                run_tag = False
                        GLOBAL_MONITOR_SIGNAL=True
                        lock.release()
                    #do inc update
                    else:
                        if post_flag > 0:
                            conti_flag = int(post["@stamp"]) - int(post_flag)
                            if conti_flag != 1:
                                logger.warning("dispacther module[%s] inc update data not continue from %s to %s"%(d_module,post["@stamp"],post_flag))
                        post_flag = post["@stamp"]
                        inc_update(e_domain,e_port,e_module,post,full_timestamp)
                        inc_update_flag=True
                        inc_cnt= inc_cnt + 1
                      
            cost = (time.time() - time1)*1000
            if (inc_update_flag):
                if (full_upate_flag):
                    logger.debug("module[%s-%s] do full_update and inc_update.deal data:[%d],cost:[%sms]"%(e_module,d_module,inc_cnt,str(cost)))
                    full_upate_flag=False
                else:
                    logger.debug("module[%s-%s] inc_update deal data:[%d],cost:[%sms]"%(e_module,d_module,inc_cnt,str(cost)))
                inc_cnt=0
            if (full_upate_flag):
                logger.debug("module[%s-%s] full_update cost:[%sms]"%(e_module,d_module,str(cost)))
                    
        else:
            if get_data:
                #engine error
                logger.warning("module[%s] get data from engine error:%s"%(e_module,get_data))
        time.sleep(1)
    return         


#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Class:start~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class Worker(Process):
    """
    multiprocess extend
    """
    def __init__(self,e_module,d_module,thread_num,e_domain="",e_port="",d_domain="",d_port="",delay_time=1,**kwargs):
        super(Worker,self).__init__(**kwargs)
        self.e_module = e_module 
        self.e_domain = e_domain
        self.e_port = e_port
        self.d_module = d_module
        self.d_domain = d_domain
        self.d_port = d_port
        self.thread_num = thread_num
        self._lock = Lock()
        self.delay_time = delay_time

    def run(self):
        """
        override run function
        """
        #write pid to file [pid.updater]
        update_pid()
        logger.info("Process:[%s-%s] start"%(self.e_module,self.d_module))
        func(self.thread_num,self.e_module,self.d_module,self._lock,e_domain=self.e_domain,e_port=self.e_port\
            ,d_domain=self.d_domain,d_port=self.d_port,delay_time=self.delay_time)

class Frame:
    """
    multiprocess server Frame
    """
    def __init__(self,engine_domain,engine_port,moudules_info,modules_pair_info,thread_num,delay_time):
        """
        init server Frame
        """
        self.e_domain = engine_domain
        self.e_port = engine_port
        self.module_info = moudules_info
        self.modules_pair_info = modules_pair_info
        self.thread_num = thread_num
        self.delay_time = delay_time

    def run(self):
        """
        run server 
        """
        #write pid to file [pid.updater]
        update_pid()
        #for process in self.process_list:
        for  process in self.module_info:
            dispatcher_domain = self.module_info[process]["d_domain"]
            dispatcher_port = self.module_info[process]["d_port"]
            e_module = self.modules_pair_info.get(process,"")
            if(e_module == ""):
                logger.warn("There's no module match dispatcher module[%s]"%process)
                continue
            p = Worker(e_module,process,self.thread_num,e_domain = self.e_domain, e_port = self.e_port,\
                d_domain=dispatcher_domain, d_port = dispatcher_port,delay_time=self.delay_time)
            p.start()


def updater():
    """
    updater main function
    """
    #check updater pids:
    is_run = check_updaterpids()
    pwd = os.getcwd()
    if not is_run:
        print "There's updater running under the dir[%s]."%pwd
        exit(0)
    pids_file = "%s/pid.updater"%pwd
    if(exists(pids_file)):
        cmd = "rm -f %s"%pids_file
        os.system(cmd)

    #init connection data
    # config from ini file
    conf = ConfigParser.ConfigParser()
    conf.read("updater.conf")
    engine_domain = conf.get("BASE", "engine_domain")
    engine_port = conf.get("BASE", "engine_port")
    dispatcher_domain = conf.get("BASE", "dispatcher_domain")
    dispatcher_port = conf.get("BASE", "dispatcher_port")
    thread_num = conf.get("BASE", "thread_num")
    sleep_time = conf.get("BASE","sleep_time")
    delay_time = 1
    if sleep_time=="" or sleep_time is None:
         t = random_delay()
         if t:
             delay_time=t
             conf.set("BASE","sleep_time",str(delay_time))
             conf.write(open("updater.conf","w"))
    else:
        delay_time = int(sleep_time)
    modules = conf.get("BASE", "process_list")
    process_list_pair = modules.split()
    process_list = []#dispacther module
    modules_pair_info ={}#dispacther module - search_v3 module
    for process in process_list_pair:
        values = process.split(":")
        if(len(values) == 2):
            d_module = values[1]
            e_module = values[0]
            process_list.append(d_module)
            modules_pair_info[d_module] = e_module
        else:
            e_module = values[0]
            d_module = values[0]
            process_list.append(d_module)
            modules_pair_info[d_module] = e_module
    sections=conf.sections()
    modules_info={}
    for section in sections:
        if section =="BASE":
            continue
        domain = conf.get(section, "dispatcher_domain")
        port =  conf.get(section, "dispatcher_port")
        #check wether process_list has this module
        if not modules_pair_info.has_key(section):#section base on dispatcher module in process list. 
            continue
        modules_info[section]={"d_domain":domain,"d_port":port}

    for module in process_list:
        if modules_info.has_key(module):#use other dispatcher_domain and port.
            continue
       
        #use base dispatcher_domain and port.
        modules_info[module]={"d_domain":dispatcher_domain,"d_port":dispatcher_port}

    logger.info("engine_domain:[%s],port:[%s]"%(engine_domain,engine_port))
    logger.info("modules:[%s],thread number:[%s]"%(modules,thread_num))
    for module in modules_info:
        logger.info("search module[%s] dispatcher_domain:[%s],port:[%s]"%(modules_pair_info[module]\
        ,modules_info[module]["d_domain"],modules_info[module]["d_port"]))
    logger.info("Before full_update should sleep %d seconds."%(delay_time))
    #start service
    frame = Frame(engine_domain,engine_port,modules_info,modules_pair_info,thread_num,delay_time)
    frame.run()

    #test 
    #_lock = Lock()
    #func(thread_num, "searcher", _lock, engine_domain, engine_port, dispatcher_domain, dispatcher_port)

    #request_parser(engine_domain,engine_port,"searcher",{'_show_module':'1'})
    #response_parser(dispatcher_domain,dispatcher_port,"searcher",{'module':'searcher','main_stamp':'1331189486','inc_stamp':'0'})

updater()

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~Function:listen Searcher~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
def check_searcher(request_domain,request_port,param):
    try:
        time1 = time.time()
        request_param = urlencode(param)
        url = "http://%s:%s/?%s"%(request_domain,request_port,request_param)   
        logger.info("monitor searcher.URL:%s"%(url))
        check_data = query(url)
        if check_data is None:
            logger.error("Searcher work not well.")
            return None
        s= check_data.find("<TotalCnt>")
        e = check_data.find("</TotalCnt>")
        cost = (time.time()-time1)*1000 
        if e > s:
            total_cnt = int(check_data[(s+10):e])
            if total_cnt > 1:
                return [param,cost]
            else:
                logger.warning("can't find totalcnt in page http://%s:%s/?%s"%(request_domain,request_port,request_param))
        else:
            logger.warning("can't find totalcnt in page http://%s:%s/?%s"%(request_domain,request_port,request_param))
        return None
    except Exception,e:
        logger.exception(e)
        return None

def work_searchermonitor(request_domain,request_port,params):
    try:
        while(True):
            #time.sleep(100) 
            key = 0
            result = {}
            for k,param in params.items():
                while(GLOBAL_MONITOR_SIGNAL==False):
                    time.sleep(3)
                data = check_searcher(request_domain,request_port,param)
                if data is None:
                    time.sleep(1)
                    continue
                result[key] = data
                key = key + 1
            if result:
                ms_cnt = 0
                s_cnt = 0
                for k,kv in result.items():
                    cost = kv[1]
                    if cost >= 100: #100ms
                        ms_cnt = ms_cnt + 1
                    if cost >= 1000: #1s
                        s_cnt = s_cnt + 1
                    if s_cnt >=3:
                        logger.warning("search takes more than 1000ms. ") 
                    elif ms_cnt >= 3:
                        logger.warning("search takes more than 100ms. ")
                    else:
                        pass
            time.sleep(180)
        return OK
    except Exception,e:
        logger.exception(e)
        return FAIL

class SearcherMonitor(Process):
    def __init__(self,engine_domain,engine_port,params,**kwargs):
        self.engine_domain=engine_domain
        self.engine_port=engine_port
        self.params = params

        super(SearcherMonitor,self).__init__(**kwargs)#
    def run(self):
        #write pid to file [pid.updater]
        update_pid()
        work_searchermonitor(self.engine_domain,self.engine_port,self.params)


def monitor():
    try:
        #check monitor pids:
        #pwd = os.getcwd()
        #is_run = check_updaterpids()
        #if not is_run:
        #    print "There's updater running under the dir[%s]."%pwd
        #    exit(0)

        conf = ConfigParser.ConfigParser()
        conf.read("updater.conf")
        engine_domain = conf.get("BASE", "engine_domain")
        engine_port = conf.get("BASE", "engine_port")
        need_monitor = int(conf.get("BASE","need_monitor"))
        logger.info ("listen Searcher.engine_domain:[%s],port:[%s]."%(engine_domain,engine_port))

        #handle query and extra parameter.
        mall_querys=[u"耳麦",u"玩具"]#,u"mp3",u"u盘"]
        pub_querys=[u"考研英语"]#,u"瑜伽",u"linux",u"红楼梦"]
        full_querys=[u"鼠标"]#,u"纸巾",u"qq卡",u"手饰"]
        key = 0
        params={}
        for q in  mall_querys:
            params[key]={"q":q.encode("gbk"),"st":"mall"}
            key = key + 1
        for q in  pub_querys:
            params[key]={"q":q.encode("gbk"),"st":"pub"}
            key = key + 1
        for q in  full_querys:
            params[key]={"q":q.encode("gbk"),"st":"full"}

        #start Searcher monitor.
        if need_monitor>=1:
            p = SearcherMonitor(engine_domain,engine_port,params)
            p.start()
        return True
    except Exception,e:
       logger.exception(e)
       return False

monitor()
