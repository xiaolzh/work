ifeq ($(dd),debug) 
BIN_DIR=./debug
CPPLAGS=-g -fPIC -Wall
LIB_PATH=../libs/debug/
BUILDS=$(DIRS) build test
else  #release
BIN_DIR=./release
CPPLAGS=-O3 -fPIC -Wall
LIB_PATH=../libs/release/
BUILDS=$(DIRS) build
endif

DIRS=$(BIN_DIR)

SRCS=$(wildcard ./*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

INC_PATH=-I../libs/ -I/usr/local/include/stlport -I/usr/include/boost
LIB=$(LIB_PATH)/libdispatcher.a -lpthread -lboost_system -lstlport -ldl
DY_LINK=-Wl,-rpath,./lib/ -Wl,-rpath-link,./lib/

.SECONDARY :%.o %(OBJS)

%.o:%.cpp
	@echo "Compile $@ ($<)."
	@g++ $(CPPLAGS) $(INC_PATH) -c $< -o $@ 

all: $(BUILDS)

$(DIRS):
	mkdir -p $@

build: $(OBJS)
	@echo 'build dispatcher'
	g++ $(CPPLAGS) -o dispatcher $(OBJS) $(INC_PATH) $(LIB) $(DY_LINK)
	mv *.o dispatcher $(BIN_DIR)

test:
	@echo 'build unit test'
	g++ -DDEBUG -DTEST_DISPATCHER $(CPPLAGS) -o test_dispatcher.exe $(SRCS) $(INC_PATH) $(LIB) $(DY_LINK)
	mv test_dispatcher.exe $(BIN_DIR)

clean:
	@echo 'clean up the generated files'
	@rm ./debug -rf;
	@rm ./release -rf;

.PHONY: all build test clean $(DIRS)
