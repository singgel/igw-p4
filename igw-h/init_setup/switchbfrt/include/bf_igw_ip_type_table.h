/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __BF_IGW_IP_TYPE_H__
#define __BF_IGW_IP_TYPE_H__

#include "jd_bfrt.h"

typedef struct igwIpTypeKey_s {
  uint64_t priority;
  uint64_t ipv4_isvalid;
  uint64_t ipv4_isvalid_mask;
  uint64_t ipv4_dstaddr;
  uint64_t ipv4_dstaddr_mask;
  uint64_t vxlan_isvalid;
  uint64_t vxlan_isvalid_mask;
  uint64_t inner_ipv4_isvalid;
  uint64_t inner_ipv4_isvalid_mask;
  uint64_t inner_ipv6_isvalid;
  uint64_t inner_ipv6_isvalid_mask;
  uint64_t vxlan_type;
  uint64_t vxlan_type_mask;
  uint64_t vxlan_tof;
  uint64_t vxlan_tof_mask;
  uint64_t ipv6_isvalid;
  uint64_t ipv6_isvalid_mask;
  uint8_t ipv6_dstaddr[16];
  uint8_t ipv6_dstaddr_mask[16];
} igwIpTypeKey;

extern void igw_ip_type_table_setup(void);
extern void igw_ip_type_table_teardown();

extern int entry_add_with_ip_from_internet_in_hit(const igwIpTypeKey *key);
extern int entry_add_with_ip_from_internet_in_dl_hit(const igwIpTypeKey *key);
extern int entry_add_with_ip_from_internet_out_hit(const igwIpTypeKey *key);
extern int entry_add_with_ip_from_internet_out_dl_hit(const igwIpTypeKey *key);
extern int entry_add_with_need_drop(const igwIpTypeKey *key);

#endif
