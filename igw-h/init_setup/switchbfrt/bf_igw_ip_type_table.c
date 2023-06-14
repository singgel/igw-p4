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

#include "bf_igw_ip_type_table.h"

const bf_rt_table_hdl *igwIpTypeTable = NULL;
static bf_rt_table_key_hdl *bfKey;
static bf_rt_table_data_hdl *bfData;

// Action Ids
static bf_rt_id_t ip_from_internet_in_hit_action_id = 0;
static bf_rt_id_t ip_from_internet_in_dl_hit_action_id = 0;
static bf_rt_id_t ip_from_internet_out_hit_action_id = 0;
static bf_rt_id_t ip_from_internet_out_dl_hit_action_id = 0;
static bf_rt_id_t need_drop_action_id = 0;

static bf_rt_id_t priority_field_id = 0;
static bf_rt_id_t ipv4_is_valid_field_id = 0;
static bf_rt_id_t ipv4_dstaddr_field_id = 0;
static bf_rt_id_t vxlan_is_valid_field_id = 0;
static bf_rt_id_t inner_ipv4_is_valid_field_id = 0;
static bf_rt_id_t inner_ipv6_is_valid_field_id = 0;
static bf_rt_id_t vxlan_type_field_id = 0;
static bf_rt_id_t vxlan_tof_field_id = 0;
static bf_rt_id_t ipv6_is_valid_field_id = 0;
static bf_rt_id_t ipv6_dstaddr_field_id = 0;

void igw_ip_type_table_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Ingress.igw_ip_type.igw_ip_type", 
		&igwIpTypeTable);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
  	// Get action Ids
	/*****************************************************/
	bf_status = bf_rt_action_name_to_id(igwIpTypeTable, 
		"P02_Ingress.igw_ip_type.ip_from_internet_in_hit", 
		&ip_from_internet_in_hit_action_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_action_name_to_id(igwIpTypeTable, 
		"P02_Ingress.igw_ip_type.ip_from_internet_in_dl_hit", 
		&ip_from_internet_in_dl_hit_action_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_action_name_to_id(igwIpTypeTable, 
		"P02_Ingress.igw_ip_type.ip_from_internet_out_hit", 
		&ip_from_internet_out_hit_action_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_action_name_to_id(igwIpTypeTable, 
		"P02_Ingress.igw_ip_type.ip_from_internet_out_dl_hit", 
		&ip_from_internet_out_dl_hit_action_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_action_name_to_id(igwIpTypeTable, 
		"P02_Ingress.igw_ip_type.need_drop", 
		&need_drop_action_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
  	// Get field-ids for key field
	/*****************************************************/
	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
		"$MATCH_PRIORITY", 
		&priority_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
		"hdr.ipv4.$valid", 
		&ipv4_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
		"hdr.ipv4.dstAddr", 
		&ipv4_dstaddr_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
		"hdr.vxlan.$valid", 
		&vxlan_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
			"hdr.inner_ipv4.$valid", 
			&inner_ipv4_is_valid_field_id);
	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
			"hdr.inner_ipv6.$valid", 
			&inner_ipv6_is_valid_field_id);
	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
			"meta.tunnel.vxlan_type", 
			&vxlan_type_field_id);
	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
			"hdr.vxlan.tof", 
			&vxlan_tof_field_id);
	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
		"hdr.ipv6.$valid", 
		&ipv6_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(igwIpTypeTable, 
		"hdr.ipv6.dstAddr", 
		&ipv6_dstaddr_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(igwIpTypeTable, &bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(igwIpTypeTable, &bfData);
  	assert(bf_status == BF_SUCCESS);	
}

void igw_ip_type_table_teardown() {
  	bf_status_t bf_status;
  
  	// Deallocate key and data
  	bf_status = bf_rt_table_key_deallocate(bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_deallocate(bfData);
  	assert(bf_status == BF_SUCCESS);
}

