/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include "utils.h"
#include "bf_outer_mac_rewrite_table.h"


// Key field ids, table data field ids, action ids, Table hdl required for
// interacting with the table
const bf_rt_table_hdl *macRewriteTable = NULL;
static bf_rt_table_key_hdl *bfrtTableKey;
static bf_rt_table_data_hdl *bfrtTableData;
// Key field ids
static bf_rt_id_t macRewrite_egress_port_field_id = 0;
// Action Ids
static bf_rt_id_t macRewrite_action_id = 0;
// Data field Ids for route action
static bf_rt_id_t macRewrite_action_src_mac_field_id = 0;
static bf_rt_id_t macRewrite_action_dst_mac_field_id = 0;

// This function does the initial set up of getting key field-ids, action-ids
// and data field ids associated with the macRewrite table. This is done once
// during init time.
void mac_rewrite_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
  	// Get table object from name
  	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, "P02_Egress.tunnel_mac_rewrite.tunnel_mac_rewrite", &macRewriteTable);
  	assert(bf_status == BF_SUCCESS);

  	// Get action Ids for tunnel_mac_rewrite
  	bf_status = bf_rt_action_name_to_id(macRewriteTable, "P02_Egress.tunnel_mac_rewrite.rewrite_tunnel_mac", &macRewrite_action_id);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_key_field_id_get(macRewriteTable, "eg_intr_md.egress_port", &macRewrite_egress_port_field_id);
  	assert(bf_status == BF_SUCCESS);

  	/***********************************************************************
   	* DATA FIELD ID GET FOR "tunnel_mac_rewrite" ACTION
   	**********************************************************************/
  	bf_status = bf_rt_data_field_id_with_action_get(
      macRewriteTable,
      "smac",
      macRewrite_action_id,
      &macRewrite_action_src_mac_field_id);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_data_field_id_with_action_get(
      macRewriteTable,
      "dmac",
      macRewrite_action_id,
      &macRewrite_action_dst_mac_field_id);
  	assert(bf_status == BF_SUCCESS);

  	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(macRewriteTable, &bfrtTableKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(macRewriteTable, &bfrtTableData);
  	assert(bf_status == BF_SUCCESS);
}

// This function clears up any allocated memory during mac_rewrite_table_teardown()
void mac_rewrite_table_teardown() {
  	bf_status_t bf_status;
  	// Deallocate key and data
  	bf_status = bf_rt_table_key_deallocate(bfrtTableKey);
  	bf_status = bf_rt_table_data_deallocate(bfrtTableData);
}

static int macRewrite_key_setup(const macRewriteKey *macRewrite_key,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
  	bf_status = bf_rt_key_field_set_value(table_key, macRewrite_egress_port_field_id, macRewrite_key->egress_port);
  	if (bf_status != BF_SUCCESS)
		return -1;

	return 0;
}

static int macRewrite_data_setup(const macWrite_actionData *data,
                                  bf_rt_table_data_hdl *table_data) {
  	// Set value into the data object
  	bf_status_t bf_status = bf_rt_data_field_set_value(
      	table_data, macRewrite_action_src_mac_field_id, data->srcMac);
  	if (bf_status != BF_SUCCESS)
		return -1;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, macRewrite_action_dst_mac_field_id, data->dstMac);
  	if (bf_status != BF_SUCCESS)
		return -1;

  	return 0;
}

static int macRewrite_entry_find(const macRewriteKey *macRewrite_key) {
	bf_status_t status = BF_SUCCESS;
	bf_rt_entry_read_flag_e flag = ENTRY_READ_FROM_HW;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(macRewriteTable, &bfrtTableKey);
  	bf_rt_table_data_reset(macRewriteTable, &bfrtTableData);

  	if (macRewrite_key_setup(macRewrite_key, bfrtTableKey) < 0)
		return -1;
	
#ifdef BFRT_GENERIC_FLAGS
	uint64_t flags = 0;
  	BF_RT_FLAG_CLEAR(flags, BF_RT_FROM_HW);
	status = bf_rt_table_entry_get(macRewriteTable, jd_bf_p->session, 
  									&jd_bf_p->dev_tgt, flags, bfrtTableKey, bfrtTableData);
#else  	// Entry get from hardware with the flag set to read from hardware
  	status = bf_rt_table_entry_get(macRewriteTable, jd_bf_p->session, 
  									&jd_bf_p->dev_tgt, bfrtTableKey, bfrtTableData, flag);
#endif
  	if (status != BF_SUCCESS)
		return -1;
	
  	return 0;
}

// This function adds or modifies an entry in the ipRoute table with "route"
// action. The workflow is similar for either table entry add or modify
static int macRewrite_entry_add(const macRewriteKey *macRewrite_key,
                                         const macWrite_actionData *data,
                                         const bool add) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(macRewriteTable, &bfrtTableKey);
  	bf_rt_table_action_data_reset(macRewriteTable, macRewrite_action_id, &bfrtTableData);

  	// Fill in the Key and Data object
  	if (macRewrite_key_setup(macRewrite_key, bfrtTableKey) < 0)
		return -1;
  	if(macRewrite_data_setup(data, bfrtTableData) < 0)
		return -1;

  	// Call table entry add API, if the request is for an add, else call modify
  	status = BF_SUCCESS;
  	if (add) {
    	status = bf_rt_table_entry_add(macRewriteTable, 
					jd_bf_p->session, 
    				&jd_bf_p->dev_tgt, 
			#ifdef BFRT_GENERIC_FLAGS
							       0,
			#endif
					bfrtTableKey, 
					bfrtTableData);
  	} else {
    	status = bf_rt_table_entry_mod(macRewriteTable, 
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

// This function deletes an entry specified by the macRewrite_key
static int macRewrite_entry_delete(const macRewriteKey *macRewrite_key) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	// Reset key before use
  	bf_rt_table_key_reset(macRewriteTable, &bfrtTableKey);
  	if (macRewrite_key_setup(macRewrite_key, bfrtTableKey) < 0)
		return -1;
	
  	status = bf_rt_table_entry_del(macRewriteTable, 
		jd_bf_p->session, 
		&jd_bf_p->dev_tgt, 
#ifdef BFRT_GENERIC_FLAGS
					0,
#endif
		bfrtTableKey);
  	if (status == BF_SUCCESS) {
  		bf_rt_session_complete_operations(jd_bf_p->session);
		return 0;
  	}
	return -1;
}
	
int mac_rewrite_table_entry_add(uint16_t egress_port, char *smac, char *dmac) {
	macRewriteKey macRewrite_key;
	macWrite_actionData macRewrite_data;
	bool flag = true;
	char rmac[6];

	if (!smac || !dmac)
		return -1;
	macRewrite_key.egress_port = egress_port;
	
	reverse_mac(rmac, smac);
	memcpy(&macRewrite_data.srcMac, rmac, 6);	
	reverse_mac(rmac, dmac);
	memcpy(&macRewrite_data.dstMac, rmac, 6);
	
	if (macRewrite_entry_find(&macRewrite_key) == 0) {
		flag = false;
	} 
	
	return macRewrite_entry_add(&macRewrite_key, &macRewrite_data, flag);
}
	
void perform_driver_func() {
 	// Do table level set up
 	mac_rewrite_table_setup();

	char smac[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    char dmac[6] = {0x66,0x55,0x44,0x33,0x22,0x11};
    char dmac2[6] = {0xff,0xee,0xdd,0xcc,0xbb,0xaa};

    assert(mac_rewrite_table_entry_add(142, smac, dmac) == 0);
    assert(mac_rewrite_table_entry_add(142, smac, dmac2) == 0);
    assert(mac_rewrite_table_entry_add(146, smac, dmac) == 0);
    assert(mac_rewrite_table_entry_add(146, smac, dmac2) == 0);
    assert(mac_rewrite_table_entry_add(150, smac, dmac) == 0);

  	return;
}
