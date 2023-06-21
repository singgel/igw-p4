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

#include "bf_eip_in_select_redirect_ip.h"

const bf_rt_table_hdl *eip_in_select_redirect_ip_Table = NULL;
static bf_rt_table_key_hdl *bfrtTableKey;
static bf_rt_table_data_hdl *bfrtTableData;
// Key field ids
static bf_rt_id_t vxlan_key_field_id = 0;
// Action Ids
// Data field Ids 
static bf_rt_id_t selector_groupid_field_id = 0;


// This function does the initial set up of getting key field-ids, action-ids
// and data field ids associated with the macRewrite table. This is done once
// during init time.
void eip_in_select_redirect_ip_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
  	// Get table object from name
  	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, "P13_Egress.eip_in_redirect.select_redirect_ip", 
  				&eip_in_select_redirect_ip_Table);
  	assert(bf_status == BF_SUCCESS);

  	// Get key Ids 
  	bf_status = bf_rt_key_field_id_get(eip_in_select_redirect_ip_Table, "hdr.vxlan.$valid", 
  			&vxlan_key_field_id);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_data_field_id_get(
      eip_in_select_redirect_ip_Table,
      "$SELECTOR_GROUP_ID",
      &selector_groupid_field_id);
  	assert(bf_status == BF_SUCCESS);

  	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(eip_in_select_redirect_ip_Table, &bfrtTableKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(eip_in_select_redirect_ip_Table, &bfrtTableData);
  	assert(bf_status == BF_SUCCESS);
}

static int eip_in_seletct_dl_ip_key_setup(uint16_t vxlan_valid,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
  	bf_status = bf_rt_key_field_set_value(table_key, vxlan_key_field_id, vxlan_valid);
  	if (bf_status != BF_SUCCESS)
		return -1;

	return 0;
}

static int eip_in_seletct_dl_ip_data_setup(bf_rt_table_data_hdl *table_data,
						uint16_t selector_group_id) {
  	// Set value into the data object
  	bf_status_t bf_status = bf_rt_data_field_set_value(
      	table_data, selector_groupid_field_id, selector_group_id);
  	if (bf_status != BF_SUCCESS)
		return -1;
  	return 0;
}

int eip_in_select_redirect_ip_entry_add(uint16_t vxlan_valid,
                                      uint16_t selector_group_id) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(eip_in_select_redirect_ip_Table, &bfrtTableKey);
  	bf_rt_table_data_reset(eip_in_select_redirect_ip_Table, &bfrtTableData);

  	// Fill in the Key and Data object
  	if (eip_in_seletct_dl_ip_key_setup(vxlan_valid, bfrtTableKey) < 0)
		return -1;
  	if(eip_in_seletct_dl_ip_data_setup(bfrtTableData, selector_group_id) < 0)
		return -1;

  	// Call table entry add API, if the request is for an add, else call modify
    status = bf_rt_table_entry_add(eip_in_select_redirect_ip_Table, 
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
