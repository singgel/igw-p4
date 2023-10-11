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

#include "bf_mirror_drop.h"

const bf_rt_table_hdl *mirrorDropTable = NULL;

static bf_rt_table_key_hdl *bfKey;
static bf_rt_table_data_hdl *bfData;

// Action Ids
static bf_rt_id_t drop_packet_action_id = 0;

// Key field ids
static bf_rt_id_t packet_color_field_id = 0;

void mirror_drop_table_setup() 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		"P02_Egress.mirror.mirror_drop", 
		&mirrorDropTable);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
  	// Get action Ids
	/*****************************************************/
	bf_status = bf_rt_action_name_to_id(mirrorDropTable, 
		"P02_Egress.mirror.drop_packet", 
		&drop_packet_action_id);
  	assert(bf_status == BF_SUCCESS);

	/******************************************************
  	// Get field-ids for key field
	/*****************************************************/
	bf_status = bf_rt_key_field_id_get(mirrorDropTable, 
		"meta.meter_packet_color", 
		&packet_color_field_id);
  	assert(bf_status == BF_SUCCESS);

	// Allocate key and data once, and use reset across different uses
  	bf_status = bf_rt_table_key_allocate(mirrorDropTable, &bfKey);
  	assert(bf_status == BF_SUCCESS);
  	bf_status = bf_rt_table_data_allocate(mirrorDropTable, &bfData);
  	assert(bf_status == BF_SUCCESS);
}

static int mirror_drop_key_setup(uint8_t color,
                       bf_rt_table_key_hdl *table_key) {
  	bf_status_t bf_status;
	bf_status = bf_rt_key_field_set_value(table_key, 
			packet_color_field_id, color);
  	if (bf_status != BF_SUCCESS) {
		return -1;
  	}	
	return 0;
}

int entry_add_with_mirror_drop(uint8_t color) {
	bf_status_t status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

  	bf_rt_table_key_reset(mirrorDropTable, &bfKey);
  	bf_rt_table_action_data_reset(mirrorDropTable, drop_packet_action_id, &bfData);

  	// Fill in the Key and Data object
  	if (mirror_drop_key_setup(color, bfKey) < 0)
		return -1;

  	if (1) {
    	status = bf_rt_table_entry_add(mirrorDropTable, 
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
