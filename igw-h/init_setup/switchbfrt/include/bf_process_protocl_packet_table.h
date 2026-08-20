/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __PROCESS_PROTOCOL_PACKET_TABLE_H__
#define __PROCESS_PROTOCOL_PACKET_TABLE_H__

#include "jd_bfrt.h"

typedef struct protocolPacketKey_s {
  uint64_t priority;
  uint64_t ipv4_isvalid;
  uint64_t ipv4_isvalid_mask;
  uint64_t vxlan_isvalid;
  uint64_t vxlan_isvalid_mask;
  uint64_t ethertype;
  uint64_t ethertype_mask;
  uint64_t dip;
  uint64_t dip_mask;
  uint64_t ingress_port;
  uint64_t ingress_port_mask;
  uint64_t ipv6_isvalid;
  uint64_t ipv6_isvalid_mask;
  uint8_t dip6[16];
  uint8_t dip6_mask[16];
} protocolPacketKey;

typedef struct copy_tocpuData_s {
  uint16_t egress_port;
} copy_tocpuData;

extern void protocol_packet_table_setup(void);
extern void protocol_packet_table_teardown(void);
extern int entry_add_with_copy_to_cpu_nos(const protocolPacketKey *key, 
	const copy_tocpuData *data);
extern int entry_add_with_receive_from_cpu_nos(const protocolPacketKey *key);
extern int entry_add_with_receive_from_cpu_lldp(const protocolPacketKey *key);

#endif
