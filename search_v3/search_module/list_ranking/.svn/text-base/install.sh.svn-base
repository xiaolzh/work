#!/bin/sh

if [ $# -lt 1 ];then
    echo "usage sh install.sh mode(debug/release)"
    exit
fi

MODE=$1
NUM=`svn info |grep "Last Changed Rev"|cut -d ' ' -f 4`

rm ./list_ranking -rf;mkdir ./list_ranking;

cp _full_timestamp ./list_ranking;

cp ../list_ranking.so ./list_ranking;

cp list_ranking.conf ./list_ranking;

cp ./scripter/$MODE/formula_analyzer ./list_ranking;

cp ./scripter/script.h_tmpl ./list_ranking;

cp ./scripter/script.cpp_tmpl ./list_ranking;

tar zcvf list_ranking_${MODE}_${NUM}.tar.gz ./list_ranking/;
