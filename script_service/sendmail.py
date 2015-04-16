#!/usr/bin/env python
# -*- coding: utf-8 -*-  
import os
import sys
import json
import smtplib
from os.path import join, getsize
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.application import MIMEApplication

host = "smtp.163.com"
user = "shawlnzhao@163.com"
passwd = "noused1"

def send_mail(json_string):
	json_decode = json.loads(json_string)
	content = ""
	content += "客户端IP：%s\n"%(json_decode["ip"])
	content += "脚本名称：%s\n"%(json_decode["script"])
	content += "脚本参数：%s\n"%(json_decode["params"])
	content += "执行时刻：%s\n"%(json_decode["timestamp"])
	content += "执行时长：%s\n"%(json_decode["execution"])
	content += "数据目录：%s\n"%(json_decode["data_path"])
	content += "文件大小：%s\n"%(json_decode["data_size"])
	to = json_decode["emails"]
	content = content.decode('utf-8').encode('gb2312')
	
	msg = MIMEMultipart()
	#msg['Subject'] = "General service script execution result notification"
	head = "通用脚本服务执行结果"
	msg['Subject'] = head.decode('utf-8').encode('gb2312')
	msg['From'] = user
	msg['To'] = ';'.join(to)

	part = MIMEText(content)
	msg.attach(part)
	#res = "res.dat"
	res = json_decode["data_path"]
	size = getsize(res)
	if (size < 1048576):
		part = MIMEApplication(open(res, 'rb').read())
		part.add_header('Content-Disposition', 'attachment', filename="res.dat")
		msg.attach(part)

	try:
		s = smtplib.SMTP(host, timeout=30)
		s.login(user, passwd)
		s.sendmail(user, to, msg.as_string())
		s.close()
	except Exception, e:
		print str(e)

if __name__ == '__main__':
	reload(sys)
	sys.setdefaultencoding("utf-8")
	send_mail(sys.argv[1])

