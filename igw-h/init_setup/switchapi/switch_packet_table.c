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
#include "bf_process_protocl_packet_table.h"
#include "switch_config.h"

static void receive_from_cpu_lldp_init() {
	protocolPacketKey key;
	copy_tocpuData data;

	memset(&key, 0, sizeof(protocolPacketKey));
	//receive_from_cpu_lldp 
	key.priority = HIGHEST_PRI;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	key.vxlan_isvalid = 0;
	key.vxlan_isvalid_mask = 0x1;
	key.ethertype = ETHERTYPE_LLDP;
	key.ethertype_mask = 0xFFFF;
	key.dip = 0;
	key.dip_mask = 0;
	key.ingress_port = g_cpu_pcie_port;
	key.ingress_port_mask = 0x1FF;
	
	if (entry_add_with_receive_from_cpu_lldp(&key) != 0) {
		SETUP_PANIC("receive_from_cpu_lldp_init fail\n");
	}

	//lldp packet copy to cpu
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	key.vxlan_isvalid = 0;
	key.vxlan_isvalid_mask = 0x1;
	key.ethertype = ETHERTYPE_LLDP;
	key.ethertype_mask = 0xFFFF;
	key.dip = 0;
	key.dip_mask = 0;
	key.ingress_port = 0;
	key.ingress_port_mask = 0;
	data.egress_port = g_cpu_pcie_port;
		
	if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
		SETUP_PANIC("lldp packet copy to cpu init fail\n");
	}
}

static void receive_from_cpu_nos_init() {
	protocolPacketKey key;
	copy_tocpuData data;
	
	memset(&key, 0, sizeof(protocolPacketKey));
	//receive_from_cpu_nos, include arp/ipv4/ipv6
	key.priority = MIDDLE_PRI;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0;
	key.vxlan_isvalid = 0;
	key.vxlan_isvalid_mask = 0;
	key.ethertype = 0;
	key.ethertype_mask = 0;
	key.dip = 0;
	key.dip_mask = 0;
	key.ingress_port = g_cpu_pcie_port;
	key.ingress_port_mask = 0x1FF;
	
	if (entry_add_with_receive_from_cpu_nos(&key) != 0) {
		SETUP_PANIC("receive_from_cpu_nos_init fail\n");
	}
}

static void arp_copy_to_cpu_nos_init() {
	protocolPacketKey key;
	copy_tocpuData data;
	
	memset(&key, 0, sizeof(protocolPacketKey));
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	key.vxlan_isvalid = 0;
	key.vxlan_isvalid_mask = 0x1;
	key.ethertype = ETHERTYPE_ARP;
	key.ethertype_mask = 0xFFFF;
	key.dip = 0;
	key.dip_mask = 0;
	key.ingress_port = 0;
	key.ingress_port_mask = 0;
	data.egress_port = g_cpu_pcie_port;
		
	if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
		SETUP_PANIC("arp_copy_to_cpu_nos_initfail\n");
	}
}

static void mgt_ip_copy_to_cpu_nos_init() {
	protocolPacketKey key;
	copy_tocpuData data;

	memset(&key, 0, sizeof(protocolPacketKey));
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	key.vxlan_isvalid = 0;
	key.vxlan_isvalid_mask = 0;
	key.ethertype = ETHERTYPE_IPV4;
	key.ethertype_mask = 0xFFFF;
	key.dip = switch_cfg.mgt_ip;
	key.dip_mask = 0xFFFFFFFF;
	key.ingress_port = 0;
	key.ingress_port_mask = 0;
	data.egress_port = g_cpu_pcie_port;
		
	if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
		SETUP_PANIC("mgt_ip_copy_to_cpu_nos_init\n");
	}
}

static void vip_copy_to_cpu_nos_init() {
	protocolPacketKey key;
	copy_tocpuData data;
	
	memset(&key, 0, sizeof(protocolPacketKey));
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	key.vxlan_isvalid = 0; //must be invalid
	key.vxlan_isvalid_mask = 0x1;
	key.ethertype = ETHERTYPE_IPV4;
	key.ethertype_mask = 0xFFFF;
	key.dip = switch_cfg.vip;
	key.dip_mask = 0xFFFFFFFF;
	key.ingress_port = 0;
	key.ingress_port_mask = 0;
	data.egress_port = g_cpu_pcie_port;
		
	if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
		SETUP_PANIC("vip_copy_to_cpu_nos_init\n");
	}
}

static void backup_vip_copy_to_cpu_nos_init() {
	protocolPacketKey key;
	copy_tocpuData data;

	if (switch_cfg.backup_vip == 0) {
		return ;
	}

	memset(&key, 0, sizeof(protocolPacketKey));
	key.priority = LOWEST_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.ipv6_isvalid = 0;
	key.ipv6_isvalid_mask = 0x1;
	key.vxlan_isvalid = 0; //must be invalid
	key.vxlan_isvalid_mask = 0x1;
	key.ethertype = ETHERTYPE_IPV4;
	key.ethertype_mask = 0xFFFF;
	key.dip = switch_cfg.backup_vip;
	key.dip_mask = 0xFFFFFFFF;
	key.ingress_port = 0;
	key.ingress_port_mask = 0;
	data.egress_port = g_cpu_pcie_port;
		
	if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
		SETUP_PANIC("backup_vip_copy_to_cpu_nos_init\n");
	}
}

static uint32_t all_ipv6_node[4] = {0xFF020000, 0, 0, 0x1};

static void hostif_copy_to_cpu_nos_init() {
	protocolPacketKey key;
	copy_tocpuData data;
	switch_hostif_t *hostif;
	int index;
	uint8_t *ip6_ptr;
		
	for (index = 0; index < g_hostif_info_array.hostif_num; index++) {
		hostif = &g_hostif_info_array.hostifs[index].hostif;
		
		memset(&key, 0, sizeof(protocolPacketKey));
		key.priority = LOWEST_PRI;
		key.ipv4_isvalid = 1;
		key.ipv4_isvalid_mask = 0x1;	
		key.ipv6_isvalid = 0;
		key.ipv6_isvalid_mask = 0x1;
		key.vxlan_isvalid = 0;
		key.vxlan_isvalid_mask = 0;
		key.ethertype = ETHERTYPE_IPV4;
		key.ethertype_mask = 0xFFFF;
		key.dip = hostif->v4addr.addr.ip4;
		key.dip_mask = 0xFFFFFFFF;
		key.ingress_port = hostif->dev_port;
		key.ingress_port_mask = 0x1FF;
		data.egress_port = g_cpu_pcie_port;
		if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
			SETUP_PANIC("entry_add_with_copy_to_cpu_nos fail\n");
		}

		if (!hostif->ipv6_enable) {
			continue;
		}
		
		memset(&key, 0, sizeof(protocolPacketKey));
		key.priority = LOWEST_PRI;
		key.ipv4_isvalid = 0;
		key.ipv4_isvalid_mask = 0x1;	
		key.ipv6_isvalid = 1;
		key.ipv6_isvalid_mask = 0x1;
		key.vxlan_isvalid = 0;
		key.vxlan_isvalid_mask = 0;
		key.ethertype = ETHERTYPE_IPV6;
		key.ethertype_mask = 0xFFFF;
		key.dip = 0;
		key.dip_mask = 0;
		ip6_ptr = (uint8_t *)&hostif->ip6.s6_addr;
		memcpy(key.dip6, ip6_ptr, 16);
		memset(key.dip6_mask, 0xFF, 16);
		key.ingress_port = hostif->dev_port;
		key.ingress_port_mask = 0x1FF;
		data.egress_port = g_cpu_pcie_port;
		if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
			SETUP_PANIC("entry_add_with_copy_to_cpu_nos fail\n");
		}

		memset(&key, 0, sizeof(protocolPacketKey));
		key.priority = LOWEST_PRI;
		key.ipv4_isvalid = 0;
		key.ipv4_isvalid_mask = 0x1;	
		key.ipv6_isvalid = 1;
		key.ipv6_isvalid_mask = 0x1;
		key.vxlan_isvalid = 0;
		key.vxlan_isvalid_mask = 0;
		key.ethertype = ETHERTYPE_IPV6;
		key.ethertype_mask = 0xFFFF;
		key.dip = 0;
		key.dip_mask = 0;
		ip6_ptr = (uint8_t *)&hostif->ip6_mc.s6_addr;
		memcpy(key.dip6, ip6_ptr, 16);
		memset(key.dip6_mask, 0xFF, 16);
		key.ingress_port = hostif->dev_port;
		key.ingress_port_mask = 0x1FF;
		data.egress_port = g_cpu_pcie_port;
		if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
			SETUP_PANIC("entry_add_with_copy_to_cpu_nos fail\n");
		}

		#if 0
		memset(&key, 0, sizeof(protocolPacketKey));
		key.priority = LOWEST_PRI;
		key.ipv4_isvalid = 0;
		key.ipv4_isvalid_mask = 0x1;	
		key.ipv6_isvalid = 1;
		key.ipv6_isvalid_mask = 0x1;
		key.vxlan_isvalid = 0;
		key.vxlan_isvalid_mask = 0;
		key.ethertype = ETHERTYPE_IPV6;
		key.ethertype_mask = 0xFFFF;
		key.dip = 0;
		key.dip_mask = 0;
		ip6_ptr = (uint8_t *)all_ipv6_node;
		memcpy(key.dip6, ip6_ptr, 16);
		memset(key.dip6_mask, 0xFF, 16);
		key.ingress_port = hostif->dev_port;
		key.ingress_port_mask = 0x1FF;
		data.egress_port = g_cpu_pcie_port;
		if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
			SETUP_PANIC("entry_add_with_copy_to_cpu_nos fail\n");
		}
		#endif

		memset(&key, 0, sizeof(protocolPacketKey));
		key.priority = LOWEST_PRI;
		key.ipv4_isvalid = 0;
		key.ipv4_isvalid_mask = 0x1;	
		key.ipv6_isvalid = 1;
		key.ipv6_isvalid_mask = 0x1;
		key.vxlan_isvalid = 0;
		key.vxlan_isvalid_mask = 0;
		key.ethertype = ETHERTYPE_IPV6;
		key.ethertype_mask = 0xFFFF;
		key.dip = 0;
		key.dip_mask = 0;
		ip6_ptr = (uint8_t *)&hostif->ip6_linklocal.s6_addr;
		memcpy(key.dip6, ip6_ptr, 16);
		memset(key.dip6_mask, 0xFF, 16);
		key.ingress_port = hostif->dev_port;
		key.ingress_port_mask = 0x1FF;
		data.egress_port = g_cpu_pcie_port;
		if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
			SETUP_PANIC("entry_add_with_copy_to_cpu_nos fail\n");
		}

		memset(&key, 0, sizeof(protocolPacketKey));
		key.priority = LOWEST_PRI;
		key.ipv4_isvalid = 0;
		key.ipv4_isvalid_mask = 0x1;	
		key.ipv6_isvalid = 1;
		key.ipv6_isvalid_mask = 0x1;
		key.vxlan_isvalid = 0;
		key.vxlan_isvalid_mask = 0;
		key.ethertype = ETHERTYPE_IPV6;
		key.ethertype_mask = 0xFFFF;
		key.dip = 0;
		key.dip_mask = 0;
		ip6_ptr = (uint8_t *)&hostif->ip6_linklocal_mc.s6_addr;
		memcpy(key.dip6, ip6_ptr, 16);
		memset(key.dip6_mask, 0xFF, 16);
		key.ingress_port = hostif->dev_port;
		key.ingress_port_mask = 0x1FF;
		data.egress_port = g_cpu_pcie_port;
		if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
			SETUP_PANIC("entry_add_with_copy_to_cpu_nos fail\n");
		}
	}
}

static void hostif_ipv6_copy_to_cpu_nos_init() {
	protocolPacketKey key;
	copy_tocpuData data;
	switch_hostif_t *hostif;
	int index;
	
	memset(&key, 0, sizeof(protocolPacketKey));
	for (index = 0; index < g_hostif_info_array.hostif_num; index++) {
		hostif = &g_hostif_info_array.hostifs[index].hostif;
		key.priority = LOWEST_PRI;
		key.ipv4_isvalid = 1;
		key.ipv4_isvalid_mask = 0x1;	
		key.ipv6_isvalid = 0;
		key.ipv6_isvalid_mask = 0x1;
		key.vxlan_isvalid = 0;
		key.vxlan_isvalid_mask = 0;
		key.ethertype = ETHERTYPE_IPV4;
		key.ethertype_mask = 0xFFFF;
		key.dip = hostif->v4addr.addr.ip4;
		key.dip_mask = 0xFFFFFFFF;
		key.ingress_port = hostif->dev_port;
		key.ingress_port_mask = 0x1FF;
		data.egress_port = g_cpu_pcie_port;
		if (entry_add_with_copy_to_cpu_nos(&key, &data) != 0) {
			SETUP_PANIC("entry_add_with_copy_to_cpu_nos fail\n");
		}
	}
}

static int ip_addr_add_lo(char *ip_str) {
	char command[60];
	sprintf(command, "ip addr add %s/32 dev lo", ip_str);
	system(command);
}

static int ip_addr_add_dummy(char *ip_str, const char *dummyint) {
	char command[80];
	sprintf(command, "ip link add %s type dummy", dummyint);
	system(command);
	sprintf(command, "ifconfig %s up",dummyint);
	system(command);
	sprintf(command, "ip addr add %s/31 dev %s", ip_str, dummyint);
	system(command);
}

static void mgt_ip_and_vip_add() {
	ip_addr_add_lo(switch_cfg.mgt_ip_addr);	
	ip_addr_add_lo(switch_cfg.vip_addr);

	if (switch_cfg.backup_vip != 0) {
		ip_addr_add_lo(switch_cfg.backup_vip_addr);
	}
}
	
void process_protocol_packet_table_init() {
	mgt_ip_and_vip_add();
	protocol_packet_table_setup();
	receive_from_cpu_lldp_init();
	receive_from_cpu_nos_init();
	arp_copy_to_cpu_nos_init();
	mgt_ip_copy_to_cpu_nos_init();
	vip_copy_to_cpu_nos_init();
	backup_vip_copy_to_cpu_nos_init();
	hostif_copy_to_cpu_nos_init();	
}
