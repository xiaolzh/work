#!/usr/local/bin/python
#coding=utf-8

import sys
import os


def init_env():
    '''initial the enviroment after installing'''
    nginx_path = os.popen('whereis nginx|awk \'{print $2}\'').read().strip()
    conf_path = nginx_path + '/conf'
    print conf_path
    if( os.path.exists(conf_path)):
        if( 0 != os.system('cp -f /usr/local/dispatcher/bin/tools/'\
            'dispatcher_nginx.conf %s/sites-enabled/'%conf_path)):
            print 'Fail to cp deploy dispatcher_nginx.conf'
            return False
        os.system('chmod +x /usr;chmod +x /usr/local;'\
            'chmod +x /usr/local/dispatcher;'\
            'chmod +x /usr/local/dispatcher/data;');
    else:
        print 'Fail to find nginx/conf'
        return False
    return True

def co_init_env():
    '''co-initial the enviroment before uninstall'''
    nginx_path = os.popen('whereis nginx|awk \'{print $2}\'').read().strip()
    conf_path = nginx_path + '/conf'
    print conf_path
    if( os.path.exists(conf_path)):
        if( 0 != os.system('rm -f %s/sites-enabled/dispatcher_nginx.conf'
            %conf_path)):
            print 'Fail to cp remove dispatcher_nginx.conf'
            return False
    else:
        print 'Fail to find nginx/conf'
        return False
    return True

def deploy(arg):
    if( arg == 'init'):
        if( True != init_env()):
            print 'Fail to init env'
    elif(arg == 'co_init'):
        if( True != co_init_env()):
            print 'Fail to co_init env'
    else:
        print 'illegal args [%s]'%arg

if __name__ == "__main__":
    if(len(sys.argv) != 2):
        print 'deploy.py [init/co_init]'
    else:
        deploy(sys.argv[1])
