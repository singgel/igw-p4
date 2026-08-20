/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __EGRESS_ACL_TABLE_H__
#define __EGRESS_ACL_TABLE_H__

#include "jd_bfrt.h"

typedef struct egressAclKey_s {
  uint64_t priority;
  uint64_t ipv4_isvalid;
  uint64_t ipv4_isvalid_mask;
  uint64_t meter_packet_color;
  uint64_t meter_packet_color_mask;
  uint64_t need_drop;
  uint64_t need_drop_mask;
} egressAclKey;

extern void egress_acl_table_setup();
extern int  add_with_system_acl_drop_packet(egressAclKey *key);
#endif

