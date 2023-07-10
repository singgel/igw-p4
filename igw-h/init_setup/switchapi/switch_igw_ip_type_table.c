/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "utils.h"
#include "switch_hostif.h"
#include "switch_device.h"
#include "switch_packet_table.h"
#include "switch_igw_ip_type_table.h"
#include "bf_igw_ip_type_table.h"
#include "switch_config.h"
#include "bf_pipeline_fix.h"

//FF::0
static uint32_t ipv6_multicast[4] = {0x000000FF, 0, 0, 0};
static uint32_t ipv6_multicast_mask[4] = {0x000000FF, 0, 0, 0};

static void internet_out_entry_init() {
	igwIpTypeKey key;

	//1. dst ip is vip, std vxlan, inner_ipv4, tof=0
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.vip;
	key.ipv4_dstaddr_mask = 0xFFFFFFFF;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 1; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv6_isvalid = 0; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_STD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = 0;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_out_hit(&key,EGR_PIPELINE_2) == 0);

	//2. dst ip is vip, std vxlan, inner_ipv6, tof=0
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.vip;
	key.ipv4_dstaddr_mask = 0xFFFFFFFF;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 0; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv6_isvalid = 1; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_STD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = 0;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_out_hit(&key,EGR_PIPELINE_0) == 0);
}

static void internet_out_between_cluster_dl_entry_init() {
	igwIpTypeKey key;

	//1. dst ip is vip, jd vxlan, inner_ipv4, tof=TOF_AZ_OUT
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.vip;
	key.ipv4_dstaddr_mask = 0xFFFFFFFF;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 1; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv6_isvalid = 0; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_AZ_OUT;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_out_dl_hit(&key,EGR_PIPELINE_2) == 0);

	//2. dst ip is vip, jd vxlan, inner_ipv6, tof=TOF_AZ_OUT
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.vip;
	key.ipv4_dstaddr_mask = 0xFFFFFFFF;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 0; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv6_isvalid = 1; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_AZ_OUT;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_out_dl_hit(&key,EGR_PIPELINE_0) == 0);
}

static void internet_out_within_cluster_dl_entry_init() {
	igwIpTypeKey key;
	uint32_t mask;

	//dst ip is dl ip, jd vxlan, inner_ipv4, tof=TOF_EIP_OUT
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.dl_ip;		
	mask = get_mask_by_prefix_len(switch_cfg.dl_ip_prefix_len);
	key.ipv4_dstaddr_mask = mask;  
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 1; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv6_isvalid = 0; 
	key.inner_ipv6_isvalid_mask = 0;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_EIP_OUT;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_out_dl_hit(&key,EGR_PIPELINE_2) == 0);

	//dst ip is dl ip, jd vxlan, inner_ipv6, tof=TOF_EIP_OUT
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.dl_ip;		
	mask = get_mask_by_prefix_len(switch_cfg.dl_ip_prefix_len);
	key.ipv4_dstaddr_mask = mask;  
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 0; 
	key.inner_ipv4_isvalid_mask = 0;
	key.inner_ipv6_isvalid = 1; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_EIP_OUT;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_out_dl_hit(&key,EGR_PIPELINE_0) == 0);
}

static void internet_in_entry_init() {
	igwIpTypeKey key;
	uint8_t *ip6_ptr;

	//1. underlay ipv4, LOWEST_PRI
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = 1;
	key.ipv4_dstaddr_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_hit(&key,EGR_PIPELINE_1) == 0);

	//2. underlay ipv4, LOWEST_PRI
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = 0;
	key.ipv4_dstaddr_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_hit(&key,EGR_PIPELINE_3) == 0);

	//3. underlay ipv6, 0xFF  need drop
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 1;
	key.ipv6_isvalid_mask = 0x1;
	ip6_ptr = (uint8_t *)ipv6_multicast;
	memcpy(key.ipv6_dstaddr, ip6_ptr, 16);
	ip6_ptr = (uint8_t *)ipv6_multicast_mask;
	memcpy(key.ipv6_dstaddr_mask, ip6_ptr, 16);
	assert(entry_add_with_need_drop(&key) == 0);

	//4. underlay ipv6, LOWEST_PRI
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 1;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_hit(&key,EGR_PIPELINE_1) == 0);
}

static void internet_in_between_cluster_dl_entry_init() {
	igwIpTypeKey key;

	//1. dst ip is vip, jd vxlan, inner_ipv4, tof=TOF_AZ_IN
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.vip;
	key.ipv4_dstaddr_mask = 0xFFFFFFFF;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 1; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv4_dstaddr= 1; 
	key.inner_ipv4_dstaddr_mask = 0x1;
	key.inner_ipv6_isvalid = 0; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_AZ_IN;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_dl_hit(&key,EGR_PIPELINE_1) == 0);

	//2. dst ip is vip, jd vxlan, inner_ipv4, tof=TOF_AZ_IN
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.vip;
	key.ipv4_dstaddr_mask = 0xFFFFFFFF;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 1; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv4_dstaddr= 0; 
	key.inner_ipv4_dstaddr_mask = 0x1;
	key.inner_ipv6_isvalid = 0; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_AZ_IN;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_dl_hit(&key,EGR_PIPELINE_3) == 0);

	//3. dst ip is vip, jd vxlan, inner_ipv6, tof=TOF_AZ_IN
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.vip;
	key.ipv4_dstaddr_mask = 0xFFFFFFFF;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 0; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv6_isvalid = 1; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_AZ_IN;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_dl_hit(&key, EGR_PIPELINE_1) == 0);
}

static void internet_in_within_cluster_dl_entry_init() {
	igwIpTypeKey key;
	uint32_t mask;
		
	//1. dst ip is dl ip, jd vxlan, inner_ipv4, tof=TOF_EIP_IN
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.dl_ip;		
	mask = get_mask_by_prefix_len(switch_cfg.dl_ip_prefix_len);
	key.ipv4_dstaddr_mask = mask;  
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 1; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv4_dstaddr= 1; 
	key.inner_ipv4_dstaddr_mask = 0x1;
	key.inner_ipv6_isvalid = 0; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_EIP_IN;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_dl_hit(&key,EGR_PIPELINE_1) == 0);	

	//2. dst ip is dl ip, jd vxlan, inner_ipv4, tof=TOF_EIP_IN
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.dl_ip;		
	mask = get_mask_by_prefix_len(switch_cfg.dl_ip_prefix_len);
	key.ipv4_dstaddr_mask = mask;  
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 1; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv4_dstaddr = 0; 
	key.inner_ipv4_dstaddr_mask = 0x1;
	key.inner_ipv6_isvalid = 0; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_EIP_IN;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_dl_hit(&key,EGR_PIPELINE_3) == 0);	

	//3. dst ip is dl ip, jd vxlan,inner_ipv6, tof=TOF_EIP_IN
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv4_dstaddr = switch_cfg.dl_ip;		
	mask = get_mask_by_prefix_len(switch_cfg.dl_ip_prefix_len);
	key.ipv4_dstaddr_mask = mask;  
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.inner_ipv4_isvalid = 0; 
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv6_isvalid = 1; 
	key.inner_ipv6_isvalid_mask = 0x1;
	key.vxlan_type = VXLAN_TYPE_JD;
	key.vxlan_type_mask = 0x3;
	key.vxlan_tof = TOF_EIP_IN;
	key.vxlan_tof_mask = 0xF;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	assert(entry_add_with_ip_from_internet_in_dl_hit(&key,EGR_PIPELINE_1) == 0);	
}

static void pipelinefix_init() {
	setPipelineKey key;
	pipeline_fix_table_setup();

	key.priority = 0;
	key.inner_ipv4_isvalid = 1;
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv4_srcaddr = 1;
	key.inner_ipv4_srcaddr_mask = 0x1;
	assert(entry_add_with_setpipeline(&key, EGR_PIPELINE_0) == 0);
	
	key.priority = 0;
	key.inner_ipv4_isvalid = 1;
	key.inner_ipv4_isvalid_mask = 0x1;
	key.inner_ipv4_srcaddr = 0;
	key.inner_ipv4_srcaddr_mask = 0x1;
	assert(entry_add_with_setpipeline(&key, EGR_PIPELINE_2) == 0);
}

void igw_ip_type_table_init() {

	igw_ip_type_table_setup();

	//internet out
	internet_out_entry_init();
	internet_out_between_cluster_dl_entry_init();
	internet_out_within_cluster_dl_entry_init();
	
	//internet in
	internet_in_entry_init();
	internet_in_between_cluster_dl_entry_init();
	internet_in_within_cluster_dl_entry_init();

	//pipelinefix init
	pipelinefix_init();
}
