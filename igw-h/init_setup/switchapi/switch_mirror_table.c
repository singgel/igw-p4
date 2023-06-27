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
#include "switch_packet_table.h"
#include "switch_config.h"

#include "switch_mirror_table.h"
#include "bf_mirror_cfg.h"


static void mirror_cfg_init(){
	normalData data;
	const char directions[20] = "BOTH";
	mirror_cfg_table_setup();

	data.direction = directions;
	data.max_pkt_len = 1600;
	data.session_enable = true;
	data.ucast_egress_port_valid = true;
	if (switch_cfg.hardware_model == Wedge_100BF_32X) {
		data.ucast_egress_port = CPU_MIRROR_PORT_2P;
	} else {
		data.ucast_egress_port = CPU_MIRROR_PORT_4P;
	}

	assert(entry_add_with_normal(CPU_MIRROR_SESSION_ID, &data) == 0);
}

void mirror_table_init() {
	mirror_cfg_init();	
}

