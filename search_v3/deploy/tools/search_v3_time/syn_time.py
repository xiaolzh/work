#!usr/bin/env python
#coding=utf-8

import os
import datetime
import ConfigParser
from optparse import OptionParser

def read_config(filepath):
    '''读取配置文件。'''
    try:
       conf = ConfigParser.ConfigParser()
       conf.read(filepath)
       db = {}
       
       host = conf.get("dbserver","host")
       host = host.strip()
       user = conf.get("dbserver","user")
       user= user.strip()
       passwd = conf.get("dbserver","passwd")
       passwd = passwd.strip()
       port =  conf.get("dbserver","port")
       port = port.strip()
       db = {'host':host,"user":user,"passwd":passwd,"port":port}
       print ("数据信息: host[%s] port[%s] user[%s] passwd[%s]"%(host,port,user,passwd))
       return db
    except Exception,e:
         print e
         return None

def read_dbtime(db):
    '''读取数据库时间.'''
    try:
        #cmd = '''mysql -uroot -proot -h192.168.85.132 -P 3306 -e"select current_timestamp();"'''
        cmd = '''mysql -u%s -p%s -h%s -P %s -e"select current_timestamp();"'''%(db['user'],db['passwd'],db['host'],db['port'])
        p = os.popen(cmd)
        data = p.readlines()
        p.close()
        ret = data[1].strip()
        print "数据库时间:", ret
        return ret#eg:2012-08-08 10:10:52
    except Exception,e:
        print e
        return None


def syn_datetime(db_str_time):
    try:
        if not db_str_time:
           return None
        sys_datetime = (datetime.datetime.now()).strftime("%Y-%m-%d %H:%M:%S")
        if (sys_datetime == db_str_time):
           return (datetime.datetime.now()).strftime("%Y-%m-%d %H:%M:%S")
        cmd = "date -s '%s'"%(db_str_time)
        os.system(cmd) # set date time.
        return (datetime.datetime.now()).strftime("%Y-%m-%d %H:%M:%S")
    except Exception,e:
        print e
        return None


def main(filepath):
    try:
        db=read_config(filepath)
        if not db:
            print "读取配置文件db.conf失败!"
            return -1
        db_str_time =read_dbtime(db)
        if not db_str_time:
            print "获取数据库时间失败!"
            return -1
        ret = syn_datetime(db_str_time)
        if not ret:
            print "同步时间失败!"
            return -1
        print "当前时间:", ret
        return 0
    except Exception,e:
        print e
        return -1

def OptParse():                                                                                                                                             
    try:
       parser = OptionParser()
       parser.add_option('-f','--file',action='store',dest='file_value')#配置文件
       (options,args) = parser.parse_args()
       path = None
       if options.file_value:
            path = options.file_value
       return path
    except Exception,e:
         print e
         return None

if __name__ == "__main__":
   try:
       filepath = OptParse()
       if not filepath:
           print "需一参数，配置文件"
           exit(0)
       main(filepath) 
   except Exception,e:
      print e
