ifeq ($(dd),debug) 
BIN_DIR=./debug
CPPLAGS=-g -fPIC -Wall -D_FILE_OFFSET_BITS=64
else  #release
BIN_DIR=./release
CPPLAGS=-O3 -fPIC -Wall -D_FILE_OFFSET_BITS=64
endif

SRCS=$(wildcard ./*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))
DIRS=$(BIN_DIR)

INC_PATH=-I/usr/local/include/stlport/ -I/usr/include/boost
LIB=#-lstlport

.SECONDARY :%.o %(OBJS)

%.o:%.cpp
	@echo "Compile $@ ($<)."
	@g++ $(CPPLAGS) $(INC_PATH) $(LIB) -c $< -o $@ 

all: $(DIRS) lib

$(DIRS):
	@mkdir -p $@

lib: $(OBJS)
	@echo 'build lib'
	@ar -rc libdispatcher.a $(OBJS) 
	@mv *.o libdispatcher.a $(BIN_DIR)

clean:
	@echo 'clean up the generated files'
	@rm ./debug -rf;
	@rm ./release -rf;

.PHONY: all so test clean $(DIRS)
