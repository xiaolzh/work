echo "search_ranking: process main data"

get_dir() {
    dir=`echo $0 | grep "^/"`
    if test "${dir}"; then
        dirname $0
    else
        dirname `pwd`/$0
    fi
}

BASE_PATH=`get_dir`
LOG_PATH=$BASE_PATH/../log/

cd ./modules/search_ranking/FB/bin
./AllServer >> $LOG_PATH/FB.log
#./AllServer.sample
ret=`echo $?`
if [ $ret -ne 0 ];then
    echo "Fail to do search_ranking main data"
    exit $ret
fi
cd -

echo "Done"
