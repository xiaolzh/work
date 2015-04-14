ifeq ($(dd),debug) 
BIN_DIR=./debug
CPPLAGS=-g -fPIC -Wall
LIB_PATH=../libs/debug/
BUILDS=$(DIRS) so test
else  #release
BIN_DIR=./release
CPPLAGS=-O3 -fPIC -Wall
LIB_PATH=../libs/release/
BUILDS=$(DIRS) so
endif

DIRS=$(BIN_DIR)

SRCS=$(wildcard ./*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

INC_PATH=-I../libs/ -I/usr/local/include/stlport
LIB=$(LIB_PATH)/libdispatcher.a -lpthread -lboost_system -lstlport -ldl

.SECONDARY :%.o %(OBJS)

%.o:%.cpp
	@echo "Compile $@ ($<)."
	@g++ $(CPPLAGS) $(INC_PATH) -c $< -o $@ 

all: $(BUILDS)

$(DIRS):
	mkdir -p $@

so: $(OBJS)
	@echo 'build so'
	g++ $(CPPLAGS) -o searcher.so $(OBJS) -shared $(INC_PATH) $(LIB)
	mv *.o searcher.so $(BIN_DIR)

test:
	@echo 'build unit test'
	g++ -DDEBUG -DTEST_SEARCH_DATA_HANDLER $(CPPLAGS) -o test_searcher.exe $(SRCS) $(INC_PATH) $(LIB)
	g++ -DDEBUG -DTEST_TEXT_DB $(CPPLAGS) -o test_text_db.exe $(SRCS) $(INC_PATH) $(LIB)
	mv *.exe $(BIN_DIR)

clean:
	@echo 'clean up the generated files'
	@rm ./debug -rf;
	@rm ./release -rf;

.PHONY: all so test clean $(DIRS)
