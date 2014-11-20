#!/bin/sh
path1="/data01/paichong/data/main/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

path1="/data01/paichong/data/backup/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

path1="/data01/paichong/data/doc_store_data/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

path1="/data01/paichong/data/doc_store_index/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

path1="/data01/paichong/data/dump/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

path1="/data01/paichong/data/recv/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

path1="/data01/paichong/data/send/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1


path1="/data01/paichong/data/store_backup/data/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

path1="/data01/paichong/data/store_backup/idx/"
if [ -f $path1 ]; then
	rm -rf $path
fi
mkdir -p $path1

