#!/bin/sh
path="/data01/paichong/log"
cmd=`du -b $path`
size=`echo $cmd|awk '{print $1}'`
while :
do
	if [[ $size -gt 8*1024*1024*1024 ]]; then
		cd $path
#		$size
		rm $(ls -rt $path | head -1)
	fi
	sleep 3600
done

