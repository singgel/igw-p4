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

#include "bf_eip_out_selector.h"

// Key field ids, table data field ids, action ids, Table hdl required for
// interacting with the table
const bf_rt_table_hdl *eip_out_selector_Table = NULL;
static bf_rt_table_key_hdl *bfrtTableKey;
static bf_rt_table_data_hdl *bfrtTableData;
// Key field ids
static bf_rt_id_t selector_key_field_id = 0;
// Action Ids
// Data field Ids 
static bf_rt_id_t memberid_field_id = 0;
static bf_rt_id_t memberstatus_field_id = 0;
static bf_rt_id_t max_group_size_field_id = 0;

// This function does the initial set up of getting key field-ids, action-ids
// and data field ids associated with the macRewrite table. This is done once
// during init time.
void eip_out_selector_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
  	// Get table object from name
  	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, "P02_Ingress.eip_out_redirect.dl_ip_group_selector", 
  				&eip_out_selector_Table);
  	assert(bf_status == BF_SUCCESS);

  	// Get key Ids 
  	bf_status = bf_rt_key_field_id_get(eip_out_selector_Table, "$SELECTOR_GROUP_ID", 
  				&selector_key_field_id);
  	assert(bf_status == BF_SUCCESS);

  	/***********************************************************************
   	* DATA FIELD ID GET FOR "ecmp_dl_ip" ACTION
   	**********************************************************************/
  	bf_status = bf_rt_data_field_id_get(
      eip_out_selector_Table,
      "$ACTION_MEMBER_ID",
      &memberid_field_id);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_data_field_id_get(
      eip_out_selector_Table,
      "$ACTION_MEMBER_STATUS",
      &memberstatus_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_data_field_id_get(
      eip_out_selector_Table,
      "$MAX_GROUP_SIZE",
      &max_group_size_field_id);
  	assert(bf_status == BF_SUCCESS);

  	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(eip_out_selector_Table, &bfrtTableKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(eip_out_selector_Table, &bfrtTableData);
  	assert(bf_status == BF_SUCCESS);
}

static int eip_out_selector_key_setup(const eipOutSelectorKey *key,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
  	bf_status = bf_rt_key_field_set_value(table_key, selector_key_field_id, key->selector_groupid);
  	if (bf_status != BF_SUCCESS)
		return -1;

	return 0;
}

static int eip_out_selector_data_setup(bf_rt_table_data_hdl *table_data,
						uint16_t max_group_size,
						uint32_t *memberid_val,uint16_t memberid_num,
						bool *memberstatus_val,uint16_t memberstatus_num) {
  	// Set value into the data object
  	bf_status_t bf_status = bf_rt_data_field_set_value(
      	table_data, max_group_size_field_id, max_group_size);
  	if (bf_status != BF_SUCCESS)
		return -1;

  	bf_status = bf_rt_data_field_set_value_array(
      	table_data, memberid_field_id, memberid_val,memberid_num);
  	if (bf_status != BF_SUCCESS)
		return -1;
	
  	bf_status = bf_rt_data_field_set_value_bool_array(
      	table_data, memberstatus_field_id, memberstatus_val, memberstatus_num);
  	if (bf_status != BF_SUCCESS)
		return -1;

  	return 0;
}

int eip_out_selector_entry_add(const eipOutSelectorKey *key,
                                        uint16_t max_group_size,
                                        uint32_t *memberid_val,uint16_t memberid_num,
										bool *memberstatus_val,uint16_t memberstatus_num,
										const bool add) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(eip_out_selector_Table, &bfrtTableKey);
  	bf_rt_table_data_reset(eip_out_selector_Table, &bfrtTableData);

  	// Fill in the Key and Data object
  	if (eip_out_selector_key_setup(key, bfrtTableKey) < 0)
		return -1;
  	if(eip_out_selector_data_setup(bfrtTableData,max_group_size,memberid_val,
			memberid_num,memberstatus_val,memberstatus_num) < 0)
		return -1;

  	// Call table entry add API, if the request is for an add, else call modify
  	status = BF_SUCCESS;
  	if (add) {
    	status = bf_rt_table_entry_add(eip_out_selector_Table, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt, 
			#ifdef BFRT_GENERIC_FLAGS
							       0,
			#endif
					bfrtTableKey, 
					bfrtTableData);
  	} else {
    	status = bf_rt_table_entry_mod(eip_out_selector_Table, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt,
    		#ifdef BFRT_GENERIC_FLAGS
							       0,
			#endif
    				bfrtTableKey, 
    				bfrtTableData);
  	}
	
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	
	return -1;
}

