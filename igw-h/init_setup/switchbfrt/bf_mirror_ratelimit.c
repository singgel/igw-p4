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

#include "bf_mirror_ratelimit.h"

const bf_rt_table_hdl *mirrorRlTable = NULL;

static bf_rt_table_key_hdl *bfKey;
static bf_rt_table_data_hdl *bfData;

// Action Ids
static bf_rt_id_t ratelimit_action_id = 0;

// Key field ids
static bf_rt_id_t mirror_flag_field_id = 0;

// Data field Ids for copy_to_cpu action
static bf_rt_id_t cir_field_id = 0;
static bf_rt_id_t pir_field_id = 0;
static bf_rt_id_t cbs_field_id = 0;
static bf_rt_id_t pbs_field_id = 0;

void mirror_rl_table_setup() {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Egress.mirror.mirror_rl", 
		&mirrorRlTable);
  	assert(bf_status == BF_SUCCESS);
	
  	/******************************************************
  	// Get action Ids
	/*****************************************************/
	bf_status = bf_rt_action_name_to_id(mirrorRlTable, 
		"P02_Egress.mirror.ratelimit", 
		&ratelimit_action_id);
  	assert(bf_status == BF_SUCCESS);

	
	/******************************************************
  	// Get field-ids for key field
	/*****************************************************/
	bf_status = bf_rt_key_field_id_get(mirrorRlTable, 
		"meta.mirror.flag", 
		&mirror_flag_field_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
   	* DATA FIELD ID GET FOR "copy_to_cpu" ACTION
   	*******************************************************/
  	bf_status = bf_rt_data_field_id_with_action_get(
      mirrorRlTable,
      "$METER_SPEC_CIR_PPS",
      ratelimit_action_id,
      &cir_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_data_field_id_with_action_get(
      mirrorRlTable,
      "$METER_SPEC_PIR_PPS",
      ratelimit_action_id,
      &pir_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_data_field_id_with_action_get(
      mirrorRlTable,
      "$METER_SPEC_CBS_PKTS",
      ratelimit_action_id,
      &cbs_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_data_field_id_with_action_get(
      mirrorRlTable,
      "$METER_SPEC_PBS_PKTS",
      ratelimit_action_id,
      &pbs_field_id);
  	assert(bf_status == BF_SUCCESS);

	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(mirrorRlTable, &bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(mirrorRlTable, &bfData);
  	assert(bf_status == BF_SUCCESS);
}

static int mirror_rl_key_setup(uint8_t mirror_flag,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
	bf_status = bf_rt_key_field_set_value(table_key, 
			mirror_flag_field_id, mirror_flag);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
	return 0;
}

static int data_setup_for_mirror_rl(mirror_rl_Data *data,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, cir_field_id, data->cir_pps);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_data_field_set_value(
      	table_data, pir_field_id, data->pir_pps);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_data_field_set_value(
      	table_data, cbs_field_id, data->cbs_pkts);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}

	bf_status = bf_rt_data_field_set_value(
      	table_data, pbs_field_id, data->pbs_pkts);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
	
  	return 0;
}

int entry_add_with_mirror_rl(uint8_t flag, mirror_rl_Data *data) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(mirrorRlTable, &bfKey);
  	bf_rt_table_action_data_reset(mirrorRlTable, ratelimit_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (mirror_rl_key_setup(flag, bfKey) < 0)
		return -1;
  	if(data_setup_for_mirror_rl(data, bfData) < 0)
		return -1;

  	if (1) {
    	status = bf_rt_table_entry_add(mirrorRlTable, 
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


