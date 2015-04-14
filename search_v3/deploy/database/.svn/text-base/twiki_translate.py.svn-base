#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import re

def translate():
    f = open('search_view.sql')
    twiki = open('search_fields.twiki', 'w+')
    twiki.write(u'|*字段名*|*说明*|*表名*|*备注*|\n'.encode('utf-8'))
    for line in f.readlines():
        sql = re.findall(ur'(\w+)\.(\w+) AS (\w+)', line)
        if len(sql) > 0: 
            #print note[0], sql[0]
            memo = u''
            if(sql[0][1] != sql[0][2]):
                memo = u'别名%s->%s'%(sql[0][1].decode('utf-8'), 
                    sql[0][2].decode('utf-8'))
            str=u'|%s|%s|%s|%s|\n'%(sql[0][2].decode('utf-8'), 
                note[0].decode('utf-8'), 
                sql[0][0].decode('utf-8'), memo)
            twiki.write(str.encode('utf-8'))
            note[0] = u"@TODO 未注释".encode('utf-8') # reset note
        else: note = re.findall(ur'/\*(.+)\*/', line)

if __name__=="__main__":
    translate()
