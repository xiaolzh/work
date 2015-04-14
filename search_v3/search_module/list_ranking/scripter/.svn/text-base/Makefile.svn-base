ifeq ($(dd),debug) 
BIN_DIR=./debug
CPPLAGS=-g -fPIC -Wall -Wno-deprecated -D_FILE_OFFSET_BITS=64
LIB_PATH=../libs/debug/
BUILDS=$(DIRS) build test
else  #release
BIN_DIR=./release
CPPLAGS=-O3 -fPIC -Wall -Wno-deprecated -D_FILE_OFFSET_BITS=64
LIB_PATH=../libs/release/
BUILDS=$(DIRS) build
endif

DIRS=$(BIN_DIR)

SRCS=main.cpp
OBJS=$(patsubst %.cpp,%.o,$(SRCS))

.SECONDARY :%.o %(OBJS)

%.o:%.cpp
	@echo "Compile $@ ($<)."
	@g++ $(CPPLAGS) -c $< -o $@ 

all: $(BUILDS)

$(DIRS):
	mkdir -p $@

build: $(OBJS)
	@echo 'build formula_analyzer'
	g++ $(CPPLAGS) -o formula_analyzer $(OBJS) -ldl
	mv *.o formula_analyzer $(BIN_DIR)

test:
	@echo 'TODO: build unit test'

clean:
	@echo 'clean up the generated files'
	rm ./debug/ -rf;
	rm ./release/ -rf;

.PHONY: all build test clean $(DIRS)
