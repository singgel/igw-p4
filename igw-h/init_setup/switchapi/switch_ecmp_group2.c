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

static void ecmp_group02_init_2p() {
	
}

static void ecmp_group02_init_4p() {
	
}

void ecmp_group02_init() {
	if (switch_cfg.hardware_model == Wedge_100BF_32X) {
		ecmp_group02_init_2p();
	} else {
		ecmp_group02_init_4p();
	}
}


