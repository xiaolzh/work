#!/bin/sh

DATA_PATH="../data/cache_data/list_ranking/data/"

WORK_PATH="./modules/list_ranking/"  # data handler's path

echo "list_ranking: process main data"

mkdir -p $DATA_PATH

cd $WORK_PATH
sh c.sh
cd -
mv $WORK_PATH/sort_priority $DATA_PATH/

#touch $DATA_PATH/list_ranking_sample.txt

echo "Done"
