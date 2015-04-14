MACRO="-DDEBUG"
INCLUDE="-I/usr/local/include/stlport -I../libs"
LIB="-L /usr/local/myodbc/lib"
LINK="../libs/debug/libdispatcher.a -ldl -liconv -lmyodbc3 -lboost_regex -lboost_system -lboost_thread -lstlport"
SOURCE="test_main.cpp"
SUPPORT_SOURCE= #"../libs/*.cpp"
BIN="test"
g++ $MACRO -g $SUPPORT_SOURCE $SOURCE -o $BIN $INCLUDE $LIB $LINK
