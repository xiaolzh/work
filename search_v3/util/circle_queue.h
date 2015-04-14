#ifndef CIRCLE_QUEQUE_H
#define CIRCLE_QUEQUE_H
/*
*by liugang 2012.4.5
*circle file queue ,store anything
*one read one write lockfree
*/
#include <string>
using std::string;

#define  RET_OK             0
#define  OPEN_FILE_FAIL     -6

#define  BUFFER_EPMTY           -101
#define  BUFFER_FULL            -102
#define  TIME_AGAIN             -103

#define  ALARM_RATE              10
#define  BUFFER_HEADER_LEN      (3*sizeof(int))
#define  STORE_HEADER_LEN  (sizeof(time_t)+sizeof(int))


/**
* read from CircleQueue mmap buffer; 
* @param p buffer ptr;
* @param retPtr the url ptr will be modify to net url.
* @param expectInterval the after the span we can purge the url to cdn;
* @param pPassedInterVal the time span passed between  current time and the stamp of the url;
*/
int ReadFromCircleQueue(char* p, char** retPtr, time_t expectInterval, time_t* pPassedInterVal);

/**
* store to CircleQueue;
* @param p the buffer ptr;
* @param url the url will be stored;
* @param url_len the length of the url;
*/
int StoreToCircleQueue(char* p, const char* buf, int len);

/**
* initialize the CircleQueue;
* @param pName the file name
* @param bufLen the length of the buffer;
*/
char* InitializeCircleQueue(const char* pName, int bufLen);

/**
* release CircleQueue;
* @param p the buffer ptr;
* @param len the length of the buffer;
*/
int ReleaseCircleQuery(char* p, int len);

/**
* get the buffer left space rate;
* @param p the buffer ptr;
*/
int GetBufferLeftRate(char* p);


int IsCircleQueueEmpty(char* p);


//this function used for log system
int GetReadOff(char* p);
void SetReadOff(char* p, int off);

int GetWriteOff(char* p);
int FindLastZeroLenOff(char* p);
int PeekSessionsFromQueue(char* p, string& strOut, int tid, int& cnt);
int PeekAllFromQueue(char* p, string& strOut, int tid, int& cnt);

#endif




