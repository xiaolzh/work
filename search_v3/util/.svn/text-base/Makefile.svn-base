INCLUDE= \
	 -I../util 

#CFLAG = -c -g -pg -Wall -fPIC 
CFLAG = -c -O2 -pipe 


GCC = g++
CC = gcc
.SUFFIXES: .o .cpp .c

OBJS = Daemon.o\
	Dic.o\
	FileOperation.o\
	InvertReader.o\
	ServerFrameLockAccept.o\
	StringTrim.o\
	TaDicW.o\
	MMapFile.o\
	BitMapFile.o\
	circle_queue.o\
	session_log_c.o\
	class_tree_nolock.o\
	XmlHttp.o

all:  ../lib/libutil.a 

../lib/libutil.a: $(OBJS)
	 ar -crv $@ $(OBJS);

.cpp.o:
	$(GCC) $(CFLAG) -o $@ $< $(INCLUDE) -D_FILE_OFFSET_BITS=64  -DUSEBDB

.c.o:
	$(GCC) $(CFLAG) -o $@ $< $(INCLUDE) -D_FILE_OFFSET_BITS=64  -DUSEBDB

clean:
	rm -f *.o
	rm -f ../lib/libutil.a

