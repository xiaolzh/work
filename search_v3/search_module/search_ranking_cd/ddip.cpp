//
//  ddip.c
//  py-ddip
//
//  Created by KevinPan on 12-8-30.
//  Copyright (c) 2012年 Dangdang Inc. All rights reserved.
//
//  tmp build cmd:
//    g++ -o ddip_maker ddip.c ddip_bin_maker.cc -Wall -O3
//

#include "ddip.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>


ddip_t *ddip_new()
{
  ddip_t *ddip = (ddip_t *)calloc(1, sizeof(ddip_t));
  if (ddip == NULL) {
    return NULL;
  }
  return ddip;
}


int ddip_load(ddip_t *ddip, const char *file_bin)
{
  assert(ddip);
  
  size_t file_size;
  
  if (ddip->fd > 0) {
    return -1;  // already loaded
  }
  ddip->fd = open(file_bin, O_RDONLY);
  if (ddip->fd == -1) {
    return -2;  // load failure, can't open file
  }
  file_size = (size_t)lseek(ddip->fd, 0, SEEK_END);
  
  ddip->m_ptr = (char *)mmap(NULL, file_size, PROT_READ, MAP_SHARED, ddip->fd, 0);
  if (ddip->m_ptr == NULL) {
    close(ddip->fd);
    ddip->fd = 0;
    return -3;  // mmap failure
  }
  
  // check header info
  memcpy(&ddip->header, ddip->m_ptr, sizeof(struct ddipb_header_st));
  if (ddip->header.file_size != file_size) {
    close(ddip->fd);
    munmap(ddip->m_ptr, file_size);
    ddip->fd    = 0;
    ddip->m_ptr = NULL;
    memset(&ddip->header, 0, sizeof(struct ddipb_header_st));
    return -4;  // file size check failure
  }
  
  return 0;  // success
}


void ddip_free(ddip_t *ddip)
{
  assert(ddip);
  if (ddip->fd != 0) {
    close(ddip->fd);
  }
  if (ddip->m_ptr != NULL) {
    munmap(ddip->m_ptr, ddip->header.file_size);
  }
  ddip->fd    = 0;
  ddip->m_ptr = NULL;
  memset(&ddip->header, 0, sizeof(struct ddipb_header_st));
  
  free(ddip);
}


int ddip_find_iploc_by_str(ddip_t *ddip,
                           ddip_entry_t *entry, const char *ip_str)
{
  return ddip_find_iploc_by_int(ddip, entry, ddip_ip2int(ip_str));
}


int ddip_find_iploc_by_int(ddip_t *ddip,
                           ddip_entry_t *entry, const uint32_t ip_int)
{
  memset(entry, 0, sizeof(ddip_entry_t));
  
  if (ddip->m_ptr == NULL) {
    return -1;  // data file not load
  }

  // binary search
  int32_t low = 0, high = ddip->header.number_records - 1, middle;
  ddipb_iprecord_entry_t *iprecord_begin =
      (ddipb_iprecord_entry_t *)(ddip->m_ptr + ddip->header.ofst_ip_records);
  ddipb_iprecord_entry_t *r = NULL, *t = NULL;  // record, target pointer
  
  while (low < high) {
    middle = (low + high) / 2;
    r = iprecord_begin + middle;
    if (r->begin_ip <= ip_int) {
      if (r->end_ip >= ip_int) {
        t = r;
        break;
      }
      low = middle + 1;
    } else {
      high = middle;
    }
  }
  
  if (!t) {
    return -2;  // not found
  }
  
  // get info out
  ddip_int2ip(t->begin_ip, entry->begin_ip, sizeof(entry->begin_ip));
  ddip_int2ip(t->end_ip,   entry->end_ip,   sizeof(entry->end_ip));
  
  char *p = ddip->m_ptr + ddip->header.ofst_loct_entry + t->ofst_loct_entry;
  
  unsigned char flag = *p++;  // read flag
  
  if (flag == 0) {
    return -3; // flag is ZERO: get record but no location info, should not happend
  }
  
  int j;
  for (j = 0; j < 7; ++j) {
    if ((((flag << j) >> 7) & 0x01) != 0x01) {
      continue;
	  printf("tiao %d\n",j);
    }
    
    int name_idx = ddip_loctid2int(*(ddipb_loctid_t *)p);
    uint32_t name_offset = *((uint32_t *)(ddip->m_ptr + ddip->header.ofst_loct_idx)
                             + name_idx);
    entry->loct[j] = ddip->m_ptr + ddip->header.ofst_loct_txt + name_offset;
    p += sizeof(ddipb_loctid_t);
  }
  
  return 0;
}



/*
 -------------------------------------------------------------------------------
 Functions
 -------------------------------------------------------------------------------
 */

ddipb_loctid_t ddip_int2loctid(int i)
{
  ddipb_loctid_t loctid;
  loctid.l1 = (unsigned char)(i >> 16) & 0xff;
  loctid.l2 = (unsigned char)(i >> 8) & 0xff;
  loctid.l3 = (unsigned char)i & 0xff;
  
  return loctid;
}

int ddip_loctid2int(ddipb_loctid_t loctid)
{
  return ((int)loctid.l1 << 16) + ((int)loctid.l2 << 8) + (int)loctid.l3;
}


const char *ddip_int2ip(const uint32_t ip_int,
                        char *dest, const size_t dest_size)
{
  struct in_addr ina;
  ina.s_addr = ntohl(ip_int);
  
  unsigned char *ucp = (unsigned char *)&ina;
  snprintf(dest, dest_size, "%d.%d.%d.%d",
           ucp[0] & 0xff,
           ucp[1] & 0xff,
           ucp[2] & 0xff,
           ucp[3] & 0xff);
  return dest;
}

uint32_t ddip_ip2int(const char *ip)
{
  return ntohl(inet_addr(ip));
}


