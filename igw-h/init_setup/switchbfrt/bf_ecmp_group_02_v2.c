/**********************************************************************
* 
* Copyright (c) 2023 jd.com, Inc. All Rights Reserved
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

#include "bf_ecmp_group_02_v2.h"

// Key field ids, table data field ids, action ids, Table hdl required for
// interacting with the table
const bf_rt_table_hdl *p02_ecmp_group_Table = NULL;
static bf_rt_table_key_hdl *bfrtTableKey;
static bf_rt_table_data_hdl *bfrtTableData;
// Key field ids
static bf_rt_id_t ecmp_group_key_field_id = 0;
static bf_rt_id_t priority_field_id = 0;
static bf_rt_id_t have_shared_bd_key_field_id = 0;

// Action Ids
// Data field Ids 
static bf_rt_id_t selector_groupid_field_id = 0;

// This function does the initial set up of getting key field-ids, action-ids
// and data field ids associated with the macRewrite table. This is done once
// during init time.
void p02_ecmp_group_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
  	// Get table object from name
  	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, "P02_Ingress.ecmp_group.ecmp_group_02_v2", 
  					&p02_ecmp_group_Table);
  	assert(bf_status == BF_SUCCESS);

  	// Get key Ids 
  	bf_status = bf_rt_key_field_id_get(p02_ecmp_group_Table, 
		"$MATCH_PRIORITY", 
		&priority_field_id);
  	assert(bf_status == BF_SUCCESS);
	
  	bf_status = bf_rt_key_field_id_get(p02_ecmp_group_Table, "meta.l3.egr_pipeline", 
						&ecmp_group_key_field_id);
  	assert(bf_status == BF_SUCCESS);
	
  	bf_status = bf_rt_key_field_id_get(p02_ecmp_group_Table, "meta.ratelimit.have_shared_bd", 
						&have_shared_bd_key_field_id);
  	assert(bf_status == BF_SUCCESS);

  	/***********************************************************************
   	* DATA FIELD ID GET FOR "ecmp_group_output" ACTION
   	**********************************************************************/
  	bf_status = bf_rt_data_field_id_get(
      p02_ecmp_group_Table,
      "$SELECTOR_GROUP_ID",
      &selector_groupid_field_id);
  	assert(bf_status == BF_SUCCESS);

  	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(p02_ecmp_group_Table, &bfrtTableKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(p02_ecmp_group_Table, &bfrtTableData);
  	assert(bf_status == BF_SUCCESS);
}

static int p02_ecmp_group_key_setup(ecmpGroupKey *key,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;

	bf_status = bf_rt_key_field_set_value(table_key, 
			priority_field_id, 
			key->priority);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
  	bf_status = bf_rt_key_field_set_value(table_key, 
		ecmp_group_key_field_id, 
		key->egr_pipeline);
  	if (bf_status != BF_SUCCESS)
		return -1;
	
  	bf_status = bf_rt_key_field_set_value_and_mask(table_key, 
			have_shared_bd_key_field_id, 
			key->have_shared_bd,
			key->have_shared_bd_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}

	return 0;
}

static int p02_ecmp_group_data_setup(bf_rt_table_data_hdl *table_data,
						uint16_t selector_group_id) {
  	// Set value into the data object
  	bf_status_t bf_status = bf_rt_data_field_set_value(
      	table_data, selector_groupid_field_id, selector_group_id);
  	if (bf_status != BF_SUCCESS)
		return -1;
  	return 0;
}

int p02_ecmp_group_entry_add(ecmpGroupKey *key,
                                      uint16_t selector_group_id) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(p02_ecmp_group_Table, &bfrtTableKey);
  	bf_rt_table_data_reset(p02_ecmp_group_Table, &bfrtTableData);

  	// Fill in the Key and Data object
  	if (p02_ecmp_group_key_setup(key, bfrtTableKey) < 0)
		return -1;
  	if(p02_ecmp_group_data_setup(bfrtTableData, selector_group_id) < 0)
		return -1;

  	// Call table entry add API, if the request is for an add, else call modify
    status = bf_rt_table_entry_add(p02_ecmp_group_Table, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt, 
			#ifdef BFRT_GENERIC_FLAGS
							       0,
			#endif
					bfrtTableKey, 
					bfrtTableData);
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	
	return -1;
}

