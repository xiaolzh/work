//
//  ddip.h
//  py-ddip
//
//  Created by KevinPan on 12-8-30.
//  Copyright (c) 2012年 Dangdang Inc. All rights reserved.
//

#ifndef __py_ddip_ddip_h__
#define __py_ddip_ddip_h__

#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>


/* 输出的IP地理信息 */
struct ddip_entry_st {
  char begin_ip[16];
  char end_ip[16];
  const char *loct[7];
};
typedef struct ddip_entry_st ddip_entry_t;


/* 地址文本对应的id，采用3个字节即可远远满足 */
struct ddipb_loctid_st {  // max num is 16777215
  unsigned char l1;
  unsigned char l2;
  unsigned char l3;
};
typedef struct ddipb_loctid_st ddipb_loctid_t;


struct ddipb_iprecord_entry_st {
  uint32_t begin_ip;
  uint32_t end_ip;
  uint32_t ofst_loct_entry;
};
typedef struct ddipb_iprecord_entry_st ddipb_iprecord_entry_t;


/* ddipb for ddip_bin */
struct ddipb_header_st {
  uint32_t ofst_loct_txt;       /* 文本区的起始位置 */
  uint32_t ofst_loct_idx;       /* 地区偏移索引的起始位置 */
  uint32_t ofst_loct_entry;     /* IP地理信息的起始位置 */
  
  uint32_t ofst_ip_records;     /* IP信息的起始位置 */
  
  int32_t  number_records;      /* 总记录数 */
  char     build_time_str[32];  /* 数据构建时间戳 */
  uint32_t file_size;           /* 数据文件完整长度，用于载入时校验大小 */
};


/* ddip */
struct ddip_st {
  int  fd;             // file hd
  char *m_ptr;         // for mmap()
  
  struct ddipb_header_st header;  // copy of header info
};
typedef struct ddip_st ddip_t;


/*
 -------------------------------------------------------------------------------
 Function Defination
 -------------------------------------------------------------------------------
 */

// API: ddip_*
ddip_t *ddip_new();
int ddip_load(ddip_t *ddip, const char *file_bin);
int ddip_find_iploc_by_str(ddip_t *ddip,
                           ddip_entry_t *entry, const char *ip_str);
int ddip_find_iploc_by_int(ddip_t *ddip,
                           ddip_entry_t *entry, const uint32_t ip_int);
void ddip_free(ddip_t *ddip);


// help functions
ddipb_loctid_t ddip_int2loctid(int i);
int ddip_loctid2int(ddipb_loctid_t loctid);
const char *ddip_int2ip(const uint32_t ip_int,
                        char *dest, const size_t dest_size);
uint32_t ddip_ip2int(const char *ip);


#endif /* defined(__py_ddip_ddip_h__) */
