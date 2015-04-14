#! /bin/bash
echo 'Back up the target log folder or log file and clean up daily'

if [ $# -lt 2 ]; then
	echo "usage ./log_cleaner.sh [log_path] [bak_path]"
	echo "   log_path:       the log path or file to clean up"
	echo "   bak_path:       the path to save back up log"
	exit 
fi

LOG=$1
BAK_PATH=$2
LOG_NAME=`basename $LOG`
DATE=`date +%Y%m%d-%H-%M-%S`

for_each_dir() {
    dir=$1
    func=$2
    for file in $dir/* ; do
        if [ -d $file ];then
            for_each_dir $file $func
        elif [ -f $file ];then
            $func $file 
        fi
    done
}

cleanup() {
    echo > $1
}

if [ ! -d $BAK_PATH ];then
    echo $BAK_PATH' is not existed, here create the dir'
    mkdir -p $BAK_PATH
fi

echo "Backup the $LOG to $BAK_PATH as $BAK_PATH/$LOG_NAME-$DATE"
cp -rf $LOG $BAK_PATH/$LOG_NAME-$DATE 

echo 'clean up the current log ...'
for_each_dir $LOG cleanup

echo 'Done'
