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

static void internet_out_entry_init() {
	igwIpTypeKey key;
	struct in_addr addr;
	
	memset(&key, 0, sizeof(igwIpTypeKey));
	key.priority = 0;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	inet_aton("0.0.0.0",&addr);	
	key.ipv4_dstaddr = addr.s_addr;
	key.ipv4_dstaddr_mask = 0;
	key.vxlan_type = VXLAN_TYPE_STD;
	key.vxlan_type_mask = 0x3;
	assert(entry_add_with_ip_from_internet_in_hit(&key) == 0);
}

static void internet_out_between_cluster_dl_entry_init() {

}

static void internet_out_within_cluster_dl_entry_init() {
	
}

static void internet_in_entry_init() {

}

static void internet_in_between_cluster_dl_entry_init() {

}

static void internet_in_within_cluster_dl_entry_init() {
	
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

}
