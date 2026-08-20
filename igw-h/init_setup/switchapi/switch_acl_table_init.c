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

#include "utils.h"
#include "switch_hostif.h"
#include "switch_device.h"

#include "switch_acl_table_init.h"
#include "bf_egress_acl_table.h"

static void egress_acl_init() {
	egressAclKey key;
	egress_acl_table_setup();

	memset(&key, 0, sizeof(egressAclKey));
	key.priority = ACL_SYSTEM_PRI;
	key.ipv4_isvalid = 1;
	key.ipv4_isvalid_mask = 0x1;
	key.meter_packet_color = 0;
	key.meter_packet_color_mask = 0;
	key.need_drop = 1;
	key.need_drop_mask = 0x1;
	assert(add_with_system_acl_drop_packet(&key) == 0);

	memset(&key, 0, sizeof(egressAclKey));
	key.priority = ACL_HIGHEST_PRI;
	key.ipv4_isvalid = 0;
	key.ipv4_isvalid_mask = 0;
	key.meter_packet_color = ACL_RED;
	key.meter_packet_color_mask = 0x3;
	key.need_drop = 0;
	key.need_drop_mask = 0;
	assert(add_with_system_acl_drop_packet(&key) == 0);
}

void acl_table_init() {
	egress_acl_init();
}
