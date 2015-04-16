CC = g++
CFLAGS = -g -D __zxl_DEBUG__

DIR_SRC=src include/json/src/lib_json
DIR_OBJ=obj
TARGET=script_service
DIR_INC=-I./include -I./src 

LIBS = 
SRCS = $(foreach d, $(DIR_SRC), $(wildcard $(d)/*.cpp))
OBJS = $(patsubst %.cpp, $(DIR_OBJ)/%.o, $(SRCS))

default:init compile
compile:$(OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET) $(LIBS) -lpthread -uuid
$(OBJS):$(DIR_OBJ)/%.o:%.cpp
	$(CC) $(CFLAGS) -c $^ -o $@ $(DIR_INC)
init:
	$(foreach d, $(DIR_SRC), mkdir -p $(DIR_OBJ)/$(d);)
test:
	echo $(OBJS)
clean:
	rm -rf $(OBJS)
	rm -f $(TARGET)
rebuild: clean compile
