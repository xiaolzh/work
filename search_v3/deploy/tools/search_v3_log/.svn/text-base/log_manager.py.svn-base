#!usr/bin/env python
#coding=utf-8

import os
import time
import datetime
import glob
import re
import traceback
from util import Config
from optparse import OptionParser

def _read_config(data_path):
    '''读取配置文件。'''
    try:
       if(not os.path.exists(data_path)):
          return None
       conf_parse = Config(data_path)
       sections = conf_parse.getsections()
       path = '.'
       for section in sections:
           if (section != "log_path"):
               continue
           path = conf_parse.get("log_path","path")
       return path
    except Exception,e:
         print e
         return None

def read_data(data_path):
    '''获取data_path目录下的所有文件名。'''
    try:
       if (not data_path):  
           print "log目录不存在."
           return None
       return  glob.glob(data_path + '/*')
    except Exception,e:
       print e
       return None


def main(log_path,before_date):
    '''删除log_path目录下的时间在before_date前的log文件'''
    try:
       data = read_data(log_path)
       if (not data):
          return None
       cmd = "rm -rf "
       for item in data:
            if item.find('log-') == -1:
                  continue
            tmp = item.split('log-')[-1]
            tmp = tmp[0:8]
            date = time.strptime(tmp,"%Y%m%d")            
            if (date < before_date):
               cmd = cmd + ' ' + item
       if cmd.find('log-') != -1:
          print cmd
          os.system(cmd)  
       return True
    except Exception,e:
        print e
	traceback.print_exc()

def OptParse():
    try:
       parser = OptionParser()
       parser.add_option('-f','--file',action='store',dest='file_value')#配置文件
       parser.add_option('-t','--time',action='store',dest='date_value')#日期时间2012-07-06
       parser.add_option('-d','--day',action='store',dest='day_value')#天数
       parser.add_option('-w','--week',action='store',dest='week_value')#周数
       (options,args) = parser.parse_args()
       now = time.strftime("%Y-%m-%d", time.localtime())
       now_time =  time.strptime(now,"%Y-%m-%d")
       path = './log.conf'
       if options.date_value:
            value=""
            if not re.match(r'[\d+]{4}-[\d+]{2}-[\d+]{2}$',options.date_value):
                print "输入%Y-%m-%d格式的日期"
                return None,None
            value = options.date_value
            now_time =  time.strptime(value,"%Y-%m-%d")                                          
       elif options.day_value:
            value=""
            if not re.match(r'[\d+]{1,2}$',options.day_value):
                print "输入至少一位最多两位数字（天数）"
                return None,None
            
            value = options.day_value
            if int(value) <=0:
                print "输入正整数！"
                return None,None
            result = datetime.datetime.now()-datetime.timedelta(days=int(value))
            tmp = result.strftime("%Y-%m-%d")
            now_time =  time.strptime(tmp,"%Y-%m-%d")
       elif options.week_value:
            if not re.match(r'[\d+]{1,2}$',options.week_value):
                print "输入至少一位最多两位数字（周数）"
                return None,None
            value = options.week_value
            if int(value) <=0:
                print "输入正整数！"
                return None,None
            result = datetime.datetime.now()-datetime.timedelta(weeks=int(value))
            tmp = result.strftime("%Y-%m-%d")
            now_time =  time.strptime(tmp,"%Y-%m-%d")
       else:
            now = time.strftime("%Y-%m-%d", time.localtime())
            now_time =  time.strptime(now,"%Y-%m-%d")
       if options.file_value:
            path = options.file_value
            if not os.path.exists(path):
                print "配置文件不存在！"
                return None,None
       return path,now_time
    except Exception,e:
         print e
         return None,None

if __name__ == "__main__":
   path,last_date = OptParse()
   if (not last_date or not path):
      exit(0)
   log_path = _read_config(path)
   if not os.path.isdir(log_path):
       print "log目录不存在！"
   else:
       log_path = os.path.abspath(log_path)
       print "log目录路径:",log_path
       main(log_path,last_date)

