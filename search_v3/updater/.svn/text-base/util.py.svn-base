#!/usr/local/bin/python

import logging
import logging.handlers
from urllib2 import urlopen



#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~function:start~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

def url_open(url_addr):
    """
    open a http url safely
    """
    try:
        page = urlopen(url_addr)
        return "ok",page
    except Exception,e:
        logger.exception("Open exception. %s"%(url_addr))
        return e,None

def init_logger(logfile,loglevel):
    """
    init a logger for updater
    """
    #get a logger 
    logger = logging.getLogger()

    #create a handler to deal logger's info
    #handler = logging.FileHandler(logfile)
    handler = logging.handlers.TimedRotatingFileHandler(logfile,"MIDNIGHT",1,15)
    handler.suffix = "%Y-%m-%d"
    formatter = logging.Formatter('[%(asctime)s] [%(levelname)s] [%(module)s-%(funcName)s] %(message)s')
    handler.setFormatter(formatter)

    #init logger setting
    logger.addHandler(handler)
    logger.setLevel(loglevel)

    return logger

#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~updater:conf~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#logger init
LogLevel = logging.INFO
logger = init_logger("./log/updater_log",LogLevel)
#test   
