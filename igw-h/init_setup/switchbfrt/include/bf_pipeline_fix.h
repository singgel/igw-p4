/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __PIPELINE_FIX_H__
#define __PIPELINE_FIX_H__

#include "jd_bfrt.h"

typedef struct setPipelineKey_s {
  uint64_t priority;
  uint64_t inner_ipv4_isvalid;
  uint64_t inner_ipv4_isvalid_mask;
  uint64_t inner_ipv4_srcaddr;
  uint64_t inner_ipv4_srcaddr_mask;
} setPipelineKey;

extern void pipeline_fix_table_setup(void);
extern int entry_add_with_setpipeline(setPipelineKey *key,
                                         uint16_t egr_pipeline);

#endif

