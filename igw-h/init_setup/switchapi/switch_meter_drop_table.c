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
#include "switch_packet_table.h"
#include "switch_meter_drop_table.h"
#include "bf_eip_in_meter_drop.h"
#include "switch_config.h"

static void eip_in_meter_drop_init() {
	eip_in_meter_drop_table_setup();
	assert(entry_add_with_eip_in_drop_packet(RED) == 0);
}

static void eip_out_meter_drop_init() {
	// in egress acl need do it
}

void meter_drop_table_init() {
	eip_in_meter_drop_init();
	eip_out_meter_drop_init();
}

