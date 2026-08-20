/**********************************************************************
* 
* Copyright (c) 2023 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
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

#include "bf_egress_acl_table.h"


const bf_rt_table_hdl *egressAclTable = NULL;

static bf_rt_table_key_hdl *bfKey;
static bf_rt_table_data_hdl *bfData;

// Action Ids
static bf_rt_id_t system_acl_drop_action_id = 0;
// Key field ids
static bf_rt_id_t priority_field_id = 0;
static bf_rt_id_t meter_packet_color_field_id = 0;
static bf_rt_id_t ipv4_is_valid_field_id = 0;
static bf_rt_id_t need_drop_field_id = 0;

void egress_acl_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Egress.egress_system_acl.egress_system_acl", 
		&egressAclTable);
  	assert(bf_status == BF_SUCCESS);
	
  	/******************************************************
  	// Get action Ids
	/*****************************************************/
	bf_status = bf_rt_action_name_to_id(egressAclTable, 
		"P02_Egress.egress_system_acl.system_acl_drop_packet", 
		&system_acl_drop_action_id);
  	assert(bf_status == BF_SUCCESS);	

	/******************************************************
  	// Get field-ids for key field
	/*****************************************************/
	bf_status = bf_rt_key_field_id_get(egressAclTable, 
		"$MATCH_PRIORITY", 
		&priority_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(egressAclTable, 
		"hdr.bg_md.meter_packet_color", 
		&meter_packet_color_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(egressAclTable, 
		"hdr.ipv4.$valid", 
		&ipv4_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(egressAclTable, 
		"hdr.bg_md.need_drop", 
		&need_drop_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(egressAclTable, &bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(egressAclTable, &bfData);
  	assert(bf_status == BF_SUCCESS);
}

static int egressAcl_key_setup(egressAclKey *key,
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
			meter_packet_color_field_id, 
			key->meter_packet_color,
			key->meter_packet_color_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
  	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			need_drop_field_id, 
			key->need_drop,
			key->need_drop_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	return 0;
}

int add_with_system_acl_drop_packet(egressAclKey *key) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(egressAclTable, &bfKey);
  	bf_rt_table_action_data_reset(egressAclTable, system_acl_drop_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (egressAcl_key_setup(key, bfKey) < 0)
		return -1;

  	if (1) {
    	status = bf_rt_table_entry_add(egressAclTable, 
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

