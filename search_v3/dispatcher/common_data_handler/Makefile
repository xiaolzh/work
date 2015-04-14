ifeq ($(dd),debug) 
BIN_DIR=./debug
CPPLAGS=-g -fPIC -Wall
LIB_PATH=../libs/debug/
BUILDS=$(DIRS) so test
INSTALL_FLAG=debug
else  #release
BIN_DIR=./release
CPPLAGS=-O3 -fPIC -Wall
LIB_PATH=../libs/release/
BUILDS=$(DIRS) so
INSTALL_FLAG=release
endif

DESTDIR=dispatcher_install/

DIRS=$(BIN_DIR)

SRCS=$(wildcard ./*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

INC_PATH=-I../libs/ -I/usr/local/include/stlport
LIB=$(LIB_PATH)/libdispatcher.a -lstlport -lpthread

.SECONDARY :%.o %(OBJS)

%.o:%.cpp
	@echo "Compile $@ ($<)."
	@g++ $(CPPLAGS) $(INC_PATH) -c $< -o $@ 

all: $(BUILDS)

$(DIRS):
	mkdir -p $@

so: $(OBJS)
	@echo 'build so'
	g++ $(CPPLAGS) -o common_data_handler.so $(OBJS) -shared $(INC_PATH) $(LIB)
	mv *.o common_data_handler.so $(BIN_DIR)

test:
	@echo 'build unit test'
	g++ -DDEBUG -DTEST_COMMON_DATA_HANDLER $(CPPLAGS) -o test_common_data_handler.exe $(SRCS) $(INC_PATH) $(LIB)
	mv test_common_data_handler.exe $(BIN_DIR)

install: install-bin

install-bin:
	@echo 'install dispatcher data modules to ${DESTDIR}'
	cd ../deploy;python setup_modules.py -p $(DESTDIR) -f $(INSTALL_FLAG);cd -;

clean:
	@echo 'clean up the generated files'
	@rm ./debug -rf;
	@rm ./release -rf;

.PHONY: all so test install install-bin clean $(DIRS)
