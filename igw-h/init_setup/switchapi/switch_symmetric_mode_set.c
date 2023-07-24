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

static void internet_in_symmetric_mode_set() {
	/*************************P13 egress***************************/
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

	/*************************P13 ingress***************************/
	//fip_dnat
	table_symmetric_mode_set(
		"P13_Ingress.dnat.fip_dnat");

	//eip_in_drop_stats
	table_symmetric_mode_set(
		"P13_Ingress.eip_in_drop_stats.meter_drop_show");
	table_symmetric_mode_set(
		"P13_Ingress.eip_in_drop_stats.ipv6_meter_drop_show");

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

static void internet_out_symmetric_mode_set() {
	/*************************P02 egress***************************/
	//eip_out_ingress_pkt_stats
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_ingress_pkt_stats.eip_out_ingress_pkt_stats");
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_ingress_pkt_stats.eip6_out_ingress_pkt_stats");

	//eip_out_meter
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_meter.bw_ratelimit");
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_meter.ipv6_bw_ratelimit");

	//eip_out_drop_stats
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_drop_stats.meter_drop_show");
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_drop_stats.ipv6_meter_drop_show");

	//eip_out_egress_pkt_stats
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_egress_pkt_stats.eip_out_egress_pkt_stats");
	table_symmetric_mode_set(
		"P02_Egress.process_gw_egress.eip_out_egress_pkt_stats.eip6_out_egress_pkt_stats");	
}

void symmetric_mode_set_init(){
	internet_in_symmetric_mode_set();
	internet_out_symmetric_mode_set();
}
