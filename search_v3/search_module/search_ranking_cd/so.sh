g++ -O3 -fPIC -o search_ranking.so ddip.cpp SearchKeyRanking.cpp SearchKeyRankingCloth.cpp SearchKeyRankingOther.cpp SearchKeyRanking3C.cpp SearchKeyRankingPub.cpp  -liconv -I../../common -I../../util -shared 
#g++ -g -fPIC -o search_ranking.so ddip.cpp SearchKeyRanking.cpp SearchKeyRankingCloth.cpp SearchKeyRankingOther.cpp SearchKeyRanking3C.cpp SearchKeyRankingPub.cpp -liconv -I../../common -I../../util -shared 
