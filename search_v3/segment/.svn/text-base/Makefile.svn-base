INCLUDE= -I../util

#CFLAG = -c -g -pg -Wall -fPIC 
CFLAG = -c -O3 -fPIC 

GCC = g++
CC = gcc
.SUFFIXES: .o .cpp .c

OBJS = ta_dic_w.o
	

all: ../lib/libsegment.a 

../lib/libsegment.a: $(OBJS)
	 ar -crv $@ $(OBJS);


.cpp.o:
	$(GCC) $(CFLAG) -o $@ $< $(INCLUDE) 

.c.o:
	$(CC) $(CFLAG) -o $@ $< $(INCLUDE) 

clean:
	rm -f *.o
	rm -f ../lib/libsegment.a

