ifeq ($(dd),debug) 
BIN_DIR=./debug
CPPLAGS=-g
MACRO=-DDEBUG
else  #release
BIN_DIR=./release
CPPLAGS=-O3
MACRO=
endif

DIRS=$(BIN_DIR)

all: $(DIRS) build

$(DIRS):
	@mkdir -p $@

build:
	@echo 'build db_reader'
	g++ $(CPPLAGS) $(MACRO) -o reader_for_search reader.cpp common.cpp util.cpp /usr/local/mysql/lib/mysql/libmysqlclient.a -I/usr/include/mysql -lz
	mv reader_for_search $(BIN_DIR)

clean:
	@echo 'clean up the generated files'
	rm ./debug -rf;
	rm ./release -rf;

.PHONY: all build clean $(DIRS)
