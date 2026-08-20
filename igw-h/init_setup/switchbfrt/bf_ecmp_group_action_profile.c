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

#include "bf_ecmp_group_action_profile.h"

// Key field ids, table data field ids, action ids, Table hdl required for
// interacting with the table
const bf_rt_table_hdl *ecmp_group_action_profile_Table = NULL;
static bf_rt_table_key_hdl *bfrtTableKey;
static bf_rt_table_data_hdl *bfrtTableData;
// Key field ids
static bf_rt_id_t action_memberid_field_id = 0;
// Action Ids
static bf_rt_id_t ecmp_group_output_action_id = 0;
// Data field Ids 
static bf_rt_id_t egress_port_field_id = 0;

// This function does the initial set up of getting key field-ids, action-ids
// and data field ids associated with the macRewrite table. This is done once
// during init time.
void ecmp_group_action_profile_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
  	// Get table object from name
  	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, "P02_Ingress.ecmp_group.pipe02_group_action_profile", 
  					&ecmp_group_action_profile_Table);
  	assert(bf_status == BF_SUCCESS);

  	// Get action Ids 
  	bf_status = bf_rt_action_name_to_id(ecmp_group_action_profile_Table, "P02_Ingress.ecmp_group.ecmp_group_entry_lag_v2", 
  					&ecmp_group_output_action_id);
  	assert(bf_status == BF_SUCCESS);

  	// Get key Ids 
  	bf_status = bf_rt_key_field_id_get(ecmp_group_action_profile_Table, "$ACTION_MEMBER_ID", 
  					&action_memberid_field_id);
  	assert(bf_status == BF_SUCCESS);

  	/***********************************************************************
   	* DATA FIELD ID GET FOR "ecmp_group_entry_lag_v2" ACTION
   	**********************************************************************/
  	bf_status = bf_rt_data_field_id_with_action_get(
      ecmp_group_action_profile_Table,
      "egress_port",
      ecmp_group_output_action_id,
      &egress_port_field_id);
  	assert(bf_status == BF_SUCCESS);

  	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(ecmp_group_action_profile_Table, &bfrtTableKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(ecmp_group_action_profile_Table, &bfrtTableData);
  	assert(bf_status == BF_SUCCESS);
}

static int ecmp_group_action_profile_key_setup(ecmpGroupActionProfileKey *key,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
  	bf_status = bf_rt_key_field_set_value(table_key, action_memberid_field_id, key->memberid);
  	if (bf_status != BF_SUCCESS)
		return -1;

	return 0;
}

static int ecmp_group_action_profile_data_setup(ecmpGroupActionPfofileData *data,
                                  bf_rt_table_data_hdl *table_data) {
  	// Set value into the data object
  	bf_status_t bf_status = bf_rt_data_field_set_value(
      	table_data, egress_port_field_id, data->egress_port);
  	if (bf_status != BF_SUCCESS)
		return -1;

  	return 0;
}

int ecmp_group_action_profile_entry_add(ecmpGroupActionProfileKey *key,
                                        ecmpGroupActionPfofileData *data) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(ecmp_group_action_profile_Table, &bfrtTableKey);
  	bf_rt_table_action_data_reset(ecmp_group_action_profile_Table, ecmp_group_output_action_id, &bfrtTableData);

  	// Fill in the Key and Data object
  	if (ecmp_group_action_profile_key_setup(key, bfrtTableKey) < 0)
		return -1;
  	if(ecmp_group_action_profile_data_setup(data, bfrtTableData) < 0)
		return -1;

  	// Call table entry add API, if the request is for an add, else call modify
    status = bf_rt_table_entry_add(ecmp_group_action_profile_Table, 
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


