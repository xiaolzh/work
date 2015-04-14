g++ -O3 -fPIC -o key_ranking.so KeyRanking.cpp  -I../common -I../util -shared 
cd list_ranking; make clean;make;make install;cd ..;
#cd example; sh so.sh;
cd query_pack; sh so.sh; cd ..;
cd search_ranking; sh so.sh; cd ..;
#g++ -g -fPIC -o key_ranking.so KeyRanking.cpp  -I../common -I../util -shared 
