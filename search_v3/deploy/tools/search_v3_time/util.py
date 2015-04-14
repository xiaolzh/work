#!usr/bin/env python
#codeing=utf-8i

import ConfigParser

class Config:
   def __init__(self,data_path):
      self.config =  ConfigParser.ConfigParser()
      self.config.read(data_path)

   def getsections(self):
      return self.config.sections()

   def getitems(self,section):
      return self.config.items(section)

   def getoptions(self,section):
      return self.config.options(section)

   def get(self,section,option):
      return self.config.get(section,option)



     


