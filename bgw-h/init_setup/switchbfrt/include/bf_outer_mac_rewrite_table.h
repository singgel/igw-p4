/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __OUTER_MAC_REWRITE_TABLE_H__
#define __OUTER_MAC_REWRITE_TABLE_H__

#include "jd_bfrt.h"

typedef struct macRewriteKey {
  uint16_t egress_port;
} macRewriteKey;

typedef struct macWrite_actionData {
  uint64_t srcMac;
  uint64_t dstMac;
} macWrite_actionData;

extern void mac_rewrite_table_setup(void);
extern void mac_rewrite_table_teardown(void);
extern void perform_driver_func(void);
extern int mac_rewrite_table_entry_add(uint16_t egress_port, char *smac, char *dmac);

#endif

