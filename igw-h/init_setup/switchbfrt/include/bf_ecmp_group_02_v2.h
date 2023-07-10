/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __P02_ECMP_GROUP_TABLE_H__
#define __P02_ECMP_GROUP_TABLE_H__

#include "jd_bfrt.h"

typedef struct ecmpGroupKey_s {
  uint64_t priority;
  uint16_t egr_pipeline;  
  uint64_t have_shared_bd;
  uint64_t have_shared_bd_mask;
} ecmpGroupKey;

extern void p02_ecmp_group_table_setup();
extern int p02_ecmp_group_entry_add(ecmpGroupKey *key,
                                      uint16_t selector_group_id);
#endif

