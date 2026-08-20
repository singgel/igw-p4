/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __BF_EIP_OUT_JD_VXLAN_H__
#define __BF_EIP_OUT_JD_VXLAN_H__

#include "jd_bfrt.h"

typedef struct eipOutJdVxlanKey_s {
  uint64_t priority;
  uint64_t vxlan_isvalid;
  uint64_t vxlan_isvalid_mask;
  uint64_t vxlan_tof;
  uint64_t vxlan_tof_mask;
  uint64_t between_cluster;
  uint64_t between_cluster_mask;
  uint64_t within_cluster;
  uint64_t within_cluster_mask;
} eipOutJdVxlanKey;

extern void eip_out_jd_vxlan_table_setup(void);
extern int entry_add_with_rewrite_az_out_jd_vxlan(const eipOutJdVxlanKey *key,
                                         uint32_t shared_bw_vip);
extern int entry_add_with_rewrite_eip_out_jd_vxlan(const eipOutJdVxlanKey *key,
                                         uint32_t srcip);
#endif

