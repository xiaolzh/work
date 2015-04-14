ifeq ($(dd),debug) 
export dd := debug
BIN_DIR=./debug
CPPLAGS=-g -fPIC -Wall
BUILDS=$(DIRS) so test
else  #release
export dd := release
BIN_DIR=./release
CPPLAGS=-O3 -fPIC -Wall
BUILDS=$(DIRS) so
endif

DIRS=$(BIN_DIR)

SRCS=$(wildcard ./*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRCS)) ddip.o

#LIB=../../lib/libutil.a
INC_PATH=-I../../util/ -I../../common

.SECONDARY :%.o %(OBJS)

.cpp.o:
	@echo "Compile $@ ($<)."
	@g++ $(CPPLAGS) $(INC_PATH) -c $< -o $@

.c.o:
	@echo "Compile $@ ($<)."
	@gcc $(CPPLAGS) $(INC_PATH) -c $< -o $@

all: $(BUILDS)
	$(MAKE) -C scripter;
	mv list_ranking.so ../

$(DIRS):
	mkdir -p $@

so: $(OBJS)
	@echo 'build so'
	g++ $(CPPLAGS) -o list_ranking.so $(OBJS) -shared $(INC_PATH) $(LIB)
	mv *.o $(BIN_DIR)

test:
	@echo 'TODO: build unit test'

install:
	@echo 'install list_ranking'
	@sh install.sh $(dd);

clean:
	@echo 'clean up the generated files'
	$(MAKE) -C scripter clean;
	rm ./debug/ -rf;
	rm ./release/ -rf;

.PHONY: all so test install clean $(DIRS)
