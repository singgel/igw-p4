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
#include "bf_eip_in_action_profile.h"
#include "bf_eip_in_selector.h"
#include "bf_eip_in_select_redirect_ip.h"
#include "bf_eip_out_action_profile.h"
#include "bf_eip_out_selector.h"
#include "bf_eip_out_select_redirect_ip.h"

static uint16_t max_group_size;
static uint32_t memberid_val[4];
static uint16_t memberid_num;
static uint16_t memberstatus_num;

static void eip_in_ecmp_dl_table_init() {
	int i;
	eipInActionProfileKey key1;
	eipInActionPfofileData data;	
	eipInSelectorKey key2;
	bool memberstatus_val[4];

	eip_in_action_profile_table_setup();
	key1.memberid = 1;
	data.dl_ip = ip_atoi("10.226.190.104");
	assert(eip_in_action_profile_entry_add(&key1, &data) == 0);
	key1.memberid = 2;
	data.dl_ip = ip_atoi("10.226.190.105");
	assert(eip_in_action_profile_entry_add(&key1, &data) == 0);
	key1.memberid = 3;
	data.dl_ip = ip_atoi("10.226.190.106");
	assert(eip_in_action_profile_entry_add(&key1, &data) == 0);
	key1.memberid = 4;
	data.dl_ip = ip_atoi("10.226.190.107");
	assert(eip_in_action_profile_entry_add(&key1, &data) == 0);

	eip_in_selector_table_setup();

	max_group_size = 4;
	memberstatus_num = 4;
	memberid_num = 4;
	
	for (i = 0; i < 4; i++) {
		memberid_val[i] = i + 1;
		memberstatus_val[i] = true;
	}
	
	key2.selector_groupid = EIP_IN_SELECTOR_GROUP_ID;
	assert(eip_in_selector_entry_add(&key2, max_group_size, 
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num, true) == 0);

	eip_in_select_redirect_ip_table_setup();
	assert(eip_in_select_redirect_ip_entry_add(1, EIP_IN_SELECTOR_GROUP_ID) == 0);
}

static void eip_out_ecmp_dl_table_init() {
	int i;
	eipOutActionProfileKey key1;
	eipOutActionPfofileData data;	
	eipOutSelectorKey key2;
	bool memberstatus_val[4];

	eip_out_action_profile_table_setup();
	key1.memberid = 1;
	data.dl_ip = ip_atoi("10.226.190.104");
	assert(eip_out_action_profile_entry_add(&key1, &data) == 0);
	key1.memberid = 2;
	data.dl_ip = ip_atoi("10.226.190.105");
	assert(eip_out_action_profile_entry_add(&key1, &data) == 0);
	key1.memberid = 3;
	data.dl_ip = ip_atoi("10.226.190.106");
	assert(eip_out_action_profile_entry_add(&key1, &data) == 0);
	key1.memberid = 4;
	data.dl_ip = ip_atoi("10.226.190.107");
	assert(eip_out_action_profile_entry_add(&key1, &data) == 0);

	eip_out_selector_table_setup();
	max_group_size = 4;
	memberstatus_num = 4;
	memberid_num = 4;
	
	for (i = 0; i < 4; i++) {
		memberid_val[i] = i + 1;
		memberstatus_val[i] = true;
	}
	
	key2.selector_groupid = EIP_OUT_SELECTOR_GROUP_ID;
	assert(eip_out_selector_entry_add(&key2, max_group_size, 
		memberid_val, memberid_num,
		memberstatus_val, memberstatus_num, true) == 0);
	
	eip_out_select_redirect_ip_table_setup();
	assert(eip_out_select_redirect_ip_entry_add(1, EIP_OUT_SELECTOR_GROUP_ID) == 0);
}

void eip_ecmp_dl_table_init() {
	eip_in_ecmp_dl_table_init();
	eip_out_ecmp_dl_table_init();
}

