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
#include "switch_config.h"
#include "switch_symmetric_mode_set.h"
#include "bf_symmetric_mode_set.h"


static void table_symmetric_mode_set(const char *table_name) {
	const bf_rt_table_hdl *tbl_dhl = NULL;
	bf_rt_table_attributes_hdl *tbl_attr_dhl;
	symmetric_table_setup(table_name, &tbl_dhl, &tbl_attr_dhl);
	symmetric_mode_set(false, tbl_dhl, tbl_attr_dhl);
}

void symmetric_mode_set_init(){
	//eip_in_redirect
	table_symmetric_mode_set(
		"P13_Egress.eip_in_redirect.eip_in_redirect");
	table_symmetric_mode_set(
		"P13_Egress.eip_in_redirect.eip6_in_redirect");
	
	//eip_in_ingress_pkt_stats
	table_symmetric_mode_set(
		"P13_Egress.eip_in_ingress_pkt_stats.eip_in_ingress_pkt_stats");
	table_symmetric_mode_set(
		"P13_Egress.eip_in_ingress_pkt_stats.eip6_in_ingress_pkt_stats");

	//eip_in_meter
	table_symmetric_mode_set(
		"P13_Egress.eip_in_meter.bw_ratelimit");
	table_symmetric_mode_set(
		"P13_Egress.eip_in_meter.ipv6_bw_ratelimit");

	//fip_dnat
	table_symmetric_mode_set(
		"P13_Ingress.dnat.fip_dnat");

	//vm_loc_mapping
	table_symmetric_mode_set(
		"P13_Ingress.vm_location_mapping.vm_loc_mapping");
	table_symmetric_mode_set(
		"P13_Ingress.vm_location_mapping.ipv6_vm_loc_mapping");

	//eip_in_egress_pstats
	table_symmetric_mode_set(
		"P13_Ingress.eip_in_egress_pstats.eip_in_egress_pkt_stats");
	table_symmetric_mode_set(
		"P13_Ingress.eip_in_egress_pstats.eip6_in_egress_pkt_stats");
}
