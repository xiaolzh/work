#g++ -g  -fPIC -o query_pack.so QueryResultPacker.cpp  MyDefine.h price_span.h -I../../common -I../../util -L../../lib -lutil -shared 
#g++ -g  -fPIC  -o  query_pack.so QueryResultPacker.cpp ErrorMessage.cpp PackMallInfo.cpp PackShopInfo.cpp CustomProcess.cpp AbstractInfoShow.cpp PublicFunc.cpp ResourceUtil.cpp ../../util/TaDicW.cpp ../../util/MMapFile.cpp  ../../util/MMapFile.h ../../util/XmlHttp.cpp  -I../../common -I../../util  -shared 

g++  -O3  -fPIC -o query_pack.so QueryResultPacker.cpp ErrorMessage.cpp PackMallInfo.cpp PackShopInfo.cpp CustomProcess.cpp AbstractInfoShow.cpp PublicFunc.cpp ResourceUtil.cpp ../../util/TaDicW.cpp ../../util/MMapFile.cpp  ../../util/MMapFile.h ../../util/XmlHttp.cpp  -I../../common -I../../util  -shared 
