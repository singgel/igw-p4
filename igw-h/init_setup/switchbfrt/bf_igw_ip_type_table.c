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
static bf_rt_id_t inner_ipv4_dstaddr_field_id = 0;
static bf_rt_id_t inner_ipv6_is_valid_field_id = 0;
static bf_rt_id_t vxlan_type_field_id = 0;
static bf_rt_id_t vxlan_tof_field_id = 0;
static bf_rt_id_t ipv6_is_valid_field_id = 0;
static bf_rt_id_t ipv6_dstaddr_field_id = 0;

// Data field Ids for copy_to_cpu_nos action
static bf_rt_id_t internet_in_hit_egrpipe_field_id = 0;
static bf_rt_id_t internet_in_dl_hit_egrpipe_field_id = 0;
static bf_rt_id_t internet_out_hit_egrpipe_field_id = 0;
static bf_rt_id_t internet_out_dl_hit_egrpipe_field_id = 0;

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
		"hdr.inner_ipv4.dstAddr", 
		&inner_ipv4_dstaddr_field_id);
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

	/******************************************************
   	* DATA FIELD ID GET FOR "xxx" ACTION
   	*******************************************************/
   	
  	bf_status = bf_rt_data_field_id_with_action_get(
      igwIpTypeTable,
      "egr_pipeline",
      ip_from_internet_in_hit_action_id,
      &internet_in_hit_egrpipe_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_data_field_id_with_action_get(
      igwIpTypeTable,
      "egr_pipeline",
      ip_from_internet_in_dl_hit_action_id,
      &internet_in_dl_hit_egrpipe_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_data_field_id_with_action_get(
      igwIpTypeTable,
      "egr_pipeline",
      ip_from_internet_out_hit_action_id,
      &internet_out_hit_egrpipe_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_data_field_id_with_action_get(
      igwIpTypeTable,
      "egr_pipeline",
      ip_from_internet_out_dl_hit_action_id,
      &internet_out_dl_hit_egrpipe_field_id);
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

static int igwIpType_key_setup(const igwIpTypeKey *key,
                       bf_rt_table_key_hdl *table_key) {
	bf_status_t bf_status;
	
	bf_status = bf_rt_key_field_set_value(table_key, 
			priority_field_id, 
			key->priority);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			ipv4_is_valid_field_id, 
			key->ipv4_isvalid,
			key->ipv4_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			ipv4_dstaddr_field_id, 
			key->ipv4_dstaddr,
			key->ipv4_dstaddr_mask);
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
			inner_ipv4_is_valid_field_id, 
			key->inner_ipv4_isvalid,
			key->inner_ipv4_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			inner_ipv4_dstaddr_field_id, 
			key->inner_ipv4_dstaddr,
			key->inner_ipv4_dstaddr_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			inner_ipv6_is_valid_field_id, 
			key->inner_ipv6_isvalid,
			key->inner_ipv6_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			vxlan_type_field_id, 
			key->vxlan_type,
			key->vxlan_type_mask);
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
			ipv6_is_valid_field_id, 
			key->ipv6_isvalid,
			key->ipv6_isvalid_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_key_field_set_value_and_mask_ptr(table_key, 
			ipv6_dstaddr_field_id, 
			key->ipv6_dstaddr,
			key->ipv6_dstaddr_mask,
			16);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	return 0;
}

static int data_setup_for_internet_in_hit(uint16_t egr_pipeline,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	internet_in_hit_egrpipe_field_id, 
      	egr_pipeline);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

static int data_setup_for_internet_in_dl_hit(uint16_t egr_pipeline,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	internet_in_dl_hit_egrpipe_field_id, 
      	egr_pipeline);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

static int data_setup_for_internet_out_hit(uint16_t egr_pipeline,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	internet_out_hit_egrpipe_field_id, 
      	egr_pipeline);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

static int data_setup_for_internet_out_dl_hit(uint16_t egr_pipeline,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	internet_out_dl_hit_egrpipe_field_id, 
      	egr_pipeline);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

int entry_add_with_ip_from_internet_in_hit(const igwIpTypeKey *key, uint16_t egr_pipeline) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(igwIpTypeTable, &bfKey);
  	bf_rt_table_action_data_reset(igwIpTypeTable, ip_from_internet_in_hit_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (igwIpType_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_internet_in_hit(egr_pipeline, bfData) < 0)
		return -1;

    status = bf_rt_table_entry_add(igwIpTypeTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				bfKey, 
    			bfData);
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	return -1;
}

int entry_add_with_ip_from_internet_in_dl_hit(const igwIpTypeKey *key, uint16_t egr_pipeline) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(igwIpTypeTable, &bfKey);
  	bf_rt_table_action_data_reset(igwIpTypeTable, ip_from_internet_in_dl_hit_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (igwIpType_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_internet_in_dl_hit(egr_pipeline, bfData) < 0)
		return -1;

    status = bf_rt_table_entry_add(igwIpTypeTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				bfKey, 
    			bfData);
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	return -1;
}

int entry_add_with_ip_from_internet_out_hit(const igwIpTypeKey *key, uint16_t egr_pipeline) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(igwIpTypeTable, &bfKey);
  	bf_rt_table_action_data_reset(igwIpTypeTable, ip_from_internet_out_hit_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (igwIpType_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_internet_out_hit(egr_pipeline, bfData) < 0)
		return -1;

    status = bf_rt_table_entry_add(igwIpTypeTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				bfKey, 
    			bfData);
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	return -1;
}

int entry_add_with_ip_from_internet_out_dl_hit(const igwIpTypeKey *key, uint16_t egr_pipeline) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(igwIpTypeTable, &bfKey);
  	bf_rt_table_action_data_reset(igwIpTypeTable, ip_from_internet_out_dl_hit_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (igwIpType_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_internet_out_dl_hit(egr_pipeline, bfData) < 0)
		return -1;

    status = bf_rt_table_entry_add(igwIpTypeTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				bfKey, 
    			bfData);
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	return -1;
}

int entry_add_with_need_drop(const igwIpTypeKey *key) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(igwIpTypeTable, &bfKey);
  	bf_rt_table_action_data_reset(igwIpTypeTable, need_drop_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (igwIpType_key_setup(key, bfKey) < 0)
		return -1;

    status = bf_rt_table_entry_add(igwIpTypeTable, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				bfKey, 
    			bfData);
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	return -1;
}

