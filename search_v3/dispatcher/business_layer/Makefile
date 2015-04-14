ifeq ($(dd),debug) 
BIN_DIR=./debug
CPPLAGS=-g -fPIC -Wall
BUILDS=$(DIRS) so test
else  #release
BIN_DIR=./release
CPPLAGS=-O3 -fPIC -Wall
BUILDS=$(DIRS) so
endif

DIRS=$(BIN_DIR)

all: $(BUILDS)

$(DIRS):
	@mkdir -p $@
so:
	@echo 'build so'
	@g++ $(CPPLAGS) -o business_layer.so business_layer.cpp -I../libs/ -shared
	@mv business_layer.so $(BIN_DIR)

test:
	@echo 'build unit test'
	g++ -DUNIT_TEST -DTEST_BUSINESS_LAYER $(CPPLAGS) -o test_business_layer.exe business_layer.cpp -I../libs/
	@mv test_business_layer.exe $(BIN_DIR)

clean:
	@echo 'clean up the generated files'
	@rm ./debug -rf;
	@rm ./release -rf;

.PHONY: all so test clean $(DIRS)
