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
#include "switch_eip_in_jd_vxlan_table.h"
#include "bf_eip_in_jd_vxlan_table.h"
#include "switch_config.h"


void eip_in_jd_vxlan_table_init() {
	eip_in_jd_vxlan_table_setup();
	
}

