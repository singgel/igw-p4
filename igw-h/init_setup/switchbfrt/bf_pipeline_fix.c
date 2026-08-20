/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
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

#include "bf_pipeline_fix.h"

const bf_rt_table_hdl *pipelineFixTable = NULL;

static bf_rt_table_key_hdl *bfKey;
static bf_rt_table_data_hdl *bfData;

// Key field ids
static bf_rt_id_t priority_field_id = 0;
static bf_rt_id_t inner_ipv4_is_valid_field_id = 0;
static bf_rt_id_t inner_ipv4_srcaddr_field_id = 0;

static bf_rt_id_t egr_pipeline_data_field_id = 0;

// Action Ids
static bf_rt_id_t setpipeline_action_id = 0;

void pipeline_fix_table_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Ingress.pipelinefix.pipeline_fix", 
		&pipelineFixTable);
  	assert(bf_status == BF_SUCCESS);
	
  	/******************************************************
  	// Get action Ids
	/*****************************************************/
	bf_status = bf_rt_action_name_to_id(pipelineFixTable, 
		"P02_Ingress.pipelinefix.set_pipeline", 
		&setpipeline_action_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
  	// Get field-ids for key field
	/*****************************************************/
	bf_status = bf_rt_key_field_id_get(pipelineFixTable, 
		"$MATCH_PRIORITY", 
		&priority_field_id);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_key_field_id_get(pipelineFixTable, 
		"hdr.inner_ipv4.$valid", 
		&inner_ipv4_is_valid_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	bf_status = bf_rt_key_field_id_get(pipelineFixTable, 
		"hdr.inner_ipv4.srcAddr", 
		&inner_ipv4_srcaddr_field_id);
  	assert(bf_status == BF_SUCCESS);
	
 	bf_status = bf_rt_data_field_id_with_action_get(
      pipelineFixTable,
      "egr_pipeline",
      setpipeline_action_id,
      &egr_pipeline_data_field_id);
  	assert(bf_status == BF_SUCCESS);
	
	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(pipelineFixTable, &bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(pipelineFixTable, &bfData);
  	assert(bf_status == BF_SUCCESS);
}

static int setpipeline_key_setup(setPipelineKey *key,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
	bf_status = bf_rt_key_field_set_value(table_key, 
			priority_field_id, 
			key->priority);
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
			inner_ipv4_srcaddr_field_id, 
			key->inner_ipv4_srcaddr,
			key->inner_ipv4_srcaddr_mask);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}
	
	return 0;
}

static int data_setup_for_setpipeline(uint16_t egr_pipeline,
                                  bf_rt_table_data_hdl *table_data) {
	bf_status_t bf_status;

  	bf_status = bf_rt_data_field_set_value(
      	table_data, 
      	egr_pipeline_data_field_id, 
      	egr_pipeline);
  	if(bf_status != BF_SUCCESS) {
		return -1;
  	}
  	return 0;
}

int entry_add_with_setpipeline(setPipelineKey *key,
                                         uint16_t egr_pipeline) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(pipelineFixTable, &bfKey);
  	bf_rt_table_action_data_reset(pipelineFixTable, setpipeline_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (setpipeline_key_setup(key, bfKey) < 0)
		return -1;
  	if(data_setup_for_setpipeline(egr_pipeline, bfData) < 0)
		return -1;

  	if (1) {
    	status = bf_rt_table_entry_add(pipelineFixTable, 
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

