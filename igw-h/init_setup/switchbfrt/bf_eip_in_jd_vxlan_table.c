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
		"P13_Egress.eip_in_redirect.rewrite_az_in_jd_vxlan", 
		&rewrite_az_in_jd_vxlan_action_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_action_name_to_id(eipInJdVxlanTable, 
		"P13_Egress.eip_in_redirect.rewrite_eip_in_jd_vxlan", 
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

static int eipInJdVxlan_key_setup(const eipInJdVxlanKey *key,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
	bf_status = bf_rt_key_field_set_value(table_key, 
			priority_field_id, 
			key->priority);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			vxlan_is_valid_field_id, 
			key->vxlan_isvalid,
			key->vxlan_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			vxlan_tof_field_id, 
			key->vxlan_tof,
			key->vxlan_tof_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			between_cluster_field_id, 
			key->between_cluster,
			key->between_cluster_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			within_cluster_field_id, 
			key->within_cluster,
			key->within_cluster_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
}

static int data_setup_for_rewrite_az_in(uint32_t shared_bw_vip,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	action_shared_bw_vip_field_id, 
      	shared_bw_vip);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

static int data_setup_for_rewrite_eip_in(uint32_t srcip,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	action_srcip_field_id, 
      	srcip);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

int entry_add_with_rewrite_az_in_jd_vxlan(const eipInJdVxlanKey *key,
                                         uint32_t shared_bw_vip) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(eipInJdVxlanTable, &bfKey);
  	bf_rt_table_action_data_reset(eipInJdVxlanTable, rewrite_az_in_jd_vxlan_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (eipInJdVxlan_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_rewrite_az_in(shared_bw_vip, bfData) < 0)
		return -1;

  	if (1) {
    	status = bf_rt_table_entry_add(eipInJdVxlanTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt,
    			#ifdef BFRT_GENERIC_FLAGS
							       0,
				#endif
    				bfKey, 
    				bfData);
  	} 
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	
	return -1;
}

int entry_add_with_rewrite_eip_in_jd_vxlan(const eipInJdVxlanKey *key,
                                         uint32_t srcip) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(eipInJdVxlanTable, &bfKey);
  	bf_rt_table_action_data_reset(eipInJdVxlanTable, rewrite_eip_in_jd_vxlan_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (eipInJdVxlan_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_rewrite_eip_in(srcip, bfData) < 0)
		return -1;

  	if (1) {
    	status = bf_rt_table_entry_add(eipInJdVxlanTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt,
    			#ifdef BFRT_GENERIC_FLAGS
							       0,
				#endif
    				bfKey, 
    				bfData);
  	} 
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	
	return -1;
}

