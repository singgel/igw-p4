/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __BF_MIRROR_METER_H__
#define __BF_MIRROR_METER_H__

#include "jd_bfrt.h"

typedef struct mirror_copy_tocpuData_s {
  	uint64_t cir_pps;
  	uint64_t pir_pps;  
  	uint64_t cbs_pkts;
  	uint64_t pbs_pkts;
} mirror_copy_tocpuData;

extern void mirror_meter_table_setup();
extern int entry_add_with_mirror_clone_to_cpu(uint8_t flag, mirror_copy_tocpuData *data);

#endif

