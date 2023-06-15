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

#include "bf_eip_in_jd_vxlan_table.h"

const bf_rt_table_hdl *eipInJdVxlanTable = NULL;

static bf_rt_table_key_hdl *bfKey;
static bf_rt_table_data_hdl *bfData;

// Action Ids
static bf_rt_id_t rewrite_az_in_jd_vxlan_action_id = 0;
static bf_rt_id_t rewrite_eip_in_jd_vxlan_action_id = 0;

// Key field ids
static bf_rt_id_t priority_field_id = 0;
static bf_rt_id_t vxlan_is_valid_field_id = 0;
static bf_rt_id_t vxlan_tof_field_id = 0;
static bf_rt_id_t between_cluster_field_id = 0;
static bf_rt_id_t within_cluster_field_id = 0;

static bf_rt_id_t action_shared_bw_vip_field_id = 0;
static bf_rt_id_t action_srcip_field_id = 0;

void eip_in_jd_vxlan_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P13_Egress.eip_in_redirect.modify_jd_vxlan", 
		&eipInJdVxlanTable);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
  	// Get action Ids
	/*****************************************************/
	bf_status = bf_rt_action_name_to_id(eipInJdVxlanTable, 
		"P13_Egress.eip_in_redirect.modify_jd_vxlan.rewrite_az_in_jd_vxlan", 
		&rewrite_az_in_jd_vxlan_action_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_action_name_to_id(eipInJdVxlanTable, 
		"P13_Egress.eip_in_redirect.modify_jd_vxlan.rewrite_eip_in_jd_vxlan", 
		&rewrite_eip_in_jd_vxlan_action_id);
  	assert(bf_status == BF_SUCCESS);	

	/******************************************************
  	// Get field-ids for key field
	/*****************************************************/
	bf_status = bf_rt_key_field_id_get(eipInJdVxlanTable, 
		"$MATCH_PRIORITY", 
		&priority_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(eipInJdVxlanTable, 
		"hdr.vxlan.$valid", 
		&vxlan_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(eipInJdVxlanTable, 
		"hdr.vxlan.tof", 
		&vxlan_tof_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(eipInJdVxlanTable, 
		"meta.ratelimit.between_cluster", 
		&between_cluster_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(eipInJdVxlanTable, 
		"meta.ratelimit.within_cluster", 
		&within_cluster_field_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
   	* DATA FIELD ID GET FOR "rewrite_az_in_jd_vxlan" ACTION
   	*******************************************************/

  	bf_status = bf_rt_data_field_id_with_action_get(
      eipInJdVxlanTable,
      "shared_bw_vip",
      rewrite_az_in_jd_vxlan_action_id,
      &action_shared_bw_vip_field_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
   	* DATA FIELD ID GET FOR "rewrite_eip_in_jd_vxlan" ACTION
   	*******************************************************/

  	bf_status = bf_rt_data_field_id_with_action_get(
      eipInJdVxlanTable,
      "srcip",
      rewrite_eip_in_jd_vxlan_action_id,
      &action_srcip_field_id);
  	assert(bf_status == BF_SUCCESS);

	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(eipInJdVxlanTable, &bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(eipInJdVxlanTable, &bfData);
  	assert(bf_status == BF_SUCCESS);
}


