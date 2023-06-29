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

#include "switch_ecmp_group2.h"
#include "bf_ecmp_group_action_profile.h"
#include "bf_ecmp_group_selector.h"
#include "bf_ecmp_group_02_v2.h"

static void ecmp_group02_init_2p() {
	int i;
	ecmpGroupActionProfileKey key;
	ecmpGroupActionPfofileData data;
	ecmpGroupSelectorKey selector_key;
	uint32_t memberid_val[16];
	uint16_t max_group_size;
	uint16_t memberid_num;
	bool memberstatus_val[16];
	uint16_t memberstatus_num;
	uint16_t egr_pipeline;
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

	egr_pipeline = 1;
	selector_group_id = selector_key.selector_groupid;
	assert(p02_ecmp_group_entry_add(egr_pipeline,selector_group_id) == 0);
}

static void ecmp_group02_init_4p() {
	int i;
	ecmpGroupActionProfileKey key;
	ecmpGroupActionPfofileData data;
	ecmpGroupSelectorKey selector_key;
	uint32_t memberid_val[16];
	uint16_t max_group_size;
	uint16_t memberid_num;
	bool memberstatus_val[16];
	uint16_t memberstatus_num;
	uint16_t egr_pipeline;
	uint16_t selector_group_id;

	for (i = 1; i < 17; i++) {
		key.memberid = i;
		data.egress_port = 128 + (i - 1) * 4;
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
	
	egr_pipeline = 1;
	selector_group_id = selector_key.selector_groupid;
	assert(p02_ecmp_group_entry_add(egr_pipeline,selector_group_id) == 0);

	for (i = 17; i < 33; i++) {
		key.memberid = i;
		data.egress_port = 384 + (i - 17) * 4;
		memberid_val[i - 17] = i;
		memberstatus_val[i - 17] = true;
		assert(ecmp_group_action_profile_entry_add(&key,&data) == 0);
	}

	selector_key.selector_groupid = 2;
	max_group_size = 16;
	memberid_num = 16;
	memberstatus_num= 16;
	assert(ecmp_group_selector_entry_add(&selector_key,max_group_size,
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num) == 0);
	egr_pipeline = 3;
	selector_group_id = selector_key.selector_groupid;
	assert(p02_ecmp_group_entry_add(egr_pipeline,selector_group_id) == 0);
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


