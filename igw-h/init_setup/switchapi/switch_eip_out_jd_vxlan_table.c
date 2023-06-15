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
#include "switch_eip_out_jd_vxlan_table.h"
#include "bf_eip_out_jd_vxlan_table.h"
#include "switch_igw_ip_type_table.h"
#include "switch_config.h"


static void eip_out_entry_init() {
	uint32_t srcip;
	eipOutJdVxlanKey key;

	memset(&key, 0, sizeof(eipOutJdVxlanKey));
	key.priority = HIGHEST_PRI;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.vxlan_tof = 0;
	key.vxlan_tof_mask = 0xF;
	key.between_cluster = 0;
	key.between_cluster_mask = 0x1;
	key.within_cluster = 1;
	key.within_cluster_mask = 0x1;
	srcip = switch_cfg.vip;
	assert(entry_add_with_rewrite_eip_out_jd_vxlan(&key,srcip) == 0);

	memset(&key, 0, sizeof(eipOutJdVxlanKey));
	key.priority = HIGHEST_PRI;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.vxlan_tof = TOF_AZ_OUT;
	key.vxlan_tof_mask = 0xF;
	key.between_cluster = 0;
	key.between_cluster_mask = 0;
	key.within_cluster = 1;
	key.within_cluster_mask = 0x1;
	srcip = switch_cfg.vip;
	assert(entry_add_with_rewrite_eip_out_jd_vxlan(&key,srcip) == 0);
}

static void az_out_entry_init() {
	uint32_t shared_bw_vip;
	eipOutJdVxlanKey key;

	memset(&key, 0, sizeof(eipOutJdVxlanKey));
	key.priority = HIGHEST_PRI;
	key.vxlan_isvalid = 1; 
	key.vxlan_isvalid_mask = 0x1;
	key.vxlan_tof = 0;
	key.vxlan_tof_mask = 0xF;
	key.between_cluster = 1;
	key.between_cluster_mask = 0x1;
	shared_bw_vip = switch_cfg.shared_bw_vip;
	assert(entry_add_with_rewrite_az_out_jd_vxlan(&key,shared_bw_vip) == 0);
}

void eip_out_jd_vxlan_table_init() {
	eip_out_jd_vxlan_table_setup();
	eip_out_entry_init();
	az_out_entry_init();
}

