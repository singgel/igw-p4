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

#include "utils.h"
#include "switch_hostif.h"
#include "switch_device.h"
#include "switch_config.h"

#include "switch_igw_ip_type_table.h"

#include "switch_ecmp_group2.h"
#include "bf_ecmp_group_action_profile.h"
#include "bf_ecmp_group_selector.h"
#include "bf_ecmp_group_02_v2.h"

static void ecmp_group02_init_2p() {
	int i;
	ecmpGroupKey egkey;
	ecmpGroupActionProfileKey key;
	ecmpGroupActionPfofileData data;
	ecmpGroupSelectorKey selector_key;
	uint32_t memberid_val[16];
	uint16_t max_group_size;
	uint16_t memberid_num;
	bool memberstatus_val[16];
	uint16_t memberstatus_num;
	uint16_t selector_group_id;

	for (i = 1; i < 17; i++) {
		key.memberid = i;
		data.egress_port = (i - 1) * 4;
		memberid_val[i - 1] = i;
		memberstatus_val[i - 1] = true;
		assert(ecmp_group_action_profile_entry_add(&key,&data) == 0);
	}

	selector_key.selector_groupid = 1;
	max_group_size = 16;
	memberid_num = 16;
	memberstatus_num= 16;
	assert(ecmp_group_selector_entry_add(&selector_key,max_group_size,
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num) == 0);

	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_1;
	egkey.have_shared_bd = 0;
	egkey.have_shared_bd_mask = 0;
	selector_group_id = selector_key.selector_groupid;
	assert(p02_ecmp_group_entry_add(&egkey,selector_group_id) == 0);

	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_3;
	egkey.have_shared_bd = 0;
	egkey.have_shared_bd_mask = 0;
	selector_group_id = selector_key.selector_groupid;
	assert(p02_ecmp_group_entry_add(&egkey,selector_group_id) == 0);
}

static void ecmp_group02_init_4p() {
	int i;
	ecmpGroupActionProfileKey key;
	ecmpGroupActionPfofileData data;
	ecmpGroupSelectorKey selector_key;
	ecmpGroupKey egkey;
	uint32_t memberid_val[16];
	uint16_t max_group_size;
	uint16_t memberid_num;
	bool memberstatus_val[16];
	uint16_t memberstatus_num;
	uint16_t selector_group_id;

    //PIPE 1 port
	for (i = 1; i < 17; i++) {
		key.memberid = i;
		data.egress_port = 128 + (i - 1) * 4;
		memberid_val[i - 1] = i;
		memberstatus_val[i - 1] = true;
		assert(ecmp_group_action_profile_entry_add(&key,&data) == 0);
	}
	
	selector_key.selector_groupid = PIPE1_SELECTORCID;
	max_group_size = 16;
	memberid_num = 16;
	memberstatus_num= 16;
	assert(ecmp_group_selector_entry_add(&selector_key,max_group_size,
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num) == 0);

    //PIPE 3 port
	for (i = 17; i < 33; i++) {
		key.memberid = i;
		data.egress_port = 384 + (i - 17) * 4;
		memberid_val[i - 17] = i;
		memberstatus_val[i - 17] = true;
		assert(ecmp_group_action_profile_entry_add(&key,&data) == 0);
	}

	selector_key.selector_groupid = PIPE3_SELECTORCID;
	max_group_size = 16;
	memberid_num = 16;
	memberstatus_num= 16;
	assert(ecmp_group_selector_entry_add(&selector_key,max_group_size,
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num) == 0);

	//PIPE 0 port
	for (i = 33; i < 49; i++) {
		key.memberid = i;
		data.egress_port = 0 + (i - 33) * 4;
		memberid_val[i - 33] = i;
		memberstatus_val[i - 33] = false;
		assert(ecmp_group_action_profile_entry_add(&key,&data) == 0);
	}
	
	selector_key.selector_groupid = PIPE0_SELECTORCID;
	max_group_size = 16;
	memberid_num = 16;
	memberstatus_num= 16;
	assert(ecmp_group_selector_entry_add(&selector_key,max_group_size,
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num) == 0);

	//PIPE 2 port
	for (i = 49; i < 65; i++) {
		key.memberid = i;
		data.egress_port = 256 + (i - 49) * 4;
		memberid_val[i - 49] = i;
		memberstatus_val[i - 49] = false;
		assert(ecmp_group_action_profile_entry_add(&key,&data) == 0);
	}

	selector_key.selector_groupid = PIPE2_SELECTORCID;
	max_group_size = 16;
	memberid_num = 16;
	memberstatus_num= 16;
	assert(ecmp_group_selector_entry_add(&selector_key,max_group_size,
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num) == 0);

	/***************************************************************/
	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_1;
	egkey.have_shared_bd = 0;
	egkey.have_shared_bd_mask = 0x1;
	assert(p02_ecmp_group_entry_add(&egkey, PIPE1_SELECTORCID) == 0);
	
	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_1;
	egkey.have_shared_bd = 1;
	egkey.have_shared_bd_mask = 0x1;
	assert(p02_ecmp_group_entry_add(&egkey, PIPE3_SELECTORCID) == 0);

	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_3;
	egkey.have_shared_bd = 0;
	egkey.have_shared_bd_mask = 0;
	assert(p02_ecmp_group_entry_add(&egkey, PIPE3_SELECTORCID) == 0);

	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_0;
	egkey.have_shared_bd = 0;
	egkey.have_shared_bd_mask = 0x1;
	assert(p02_ecmp_group_entry_add(&egkey, PIPE0_SELECTORCID) == 0);
	
	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_0;
	egkey.have_shared_bd = 1;
	egkey.have_shared_bd_mask = 0x1;
	assert(p02_ecmp_group_entry_add(&egkey, PIPE2_SELECTORCID) == 0);

	egkey.priority = 0;
	egkey.egr_pipeline = EGR_PIPELINE_2;
	egkey.have_shared_bd = 0;
	egkey.have_shared_bd_mask = 0;
	assert(p02_ecmp_group_entry_add(&egkey, PIPE2_SELECTORCID) == 0);
}

void ecmp_group02_init() {
	ecmp_group_action_profile_table_setup();
	ecmp_group_selector_table_setup();
	p02_ecmp_group_table_setup();

	if (switch_cfg.hardware_model == Wedge_100BF_32X) {
		ecmp_group02_init_2p();
	} else {
		ecmp_group02_init_4p();
	}
}


