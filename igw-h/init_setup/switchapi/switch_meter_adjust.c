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
#include "switch_meter_adjust.h"
#include "bf_meter_adjust.h"

static void eip_in_meter_adjust_init() {
	eip_in_meter_adjust_setup();
	eip_in_meter_byte_count_adjust_set(EIP_IN_METER_ADJUST_BYTE_COUNT);
}

static void eip6_in_meter_adjust_init() {
	eip6_in_meter_adjust_setup();
	eip6_in_meter_byte_count_adjust_set(EIP6_IN_METER_ADJUST_BYTE_COUNT);
}

static void eip_in_share_meter_adjust_init() {
	eip_in_share_meter_adjust_setup();
	eip_in_share_meter_byte_count_adjust_set(EIP_IN_SHARE_METER_ADJUST_BYTE_COUNT);
}

static void eip_out_meter_adjust_init() {
	eip_out_meter_adjust_setup();
	eip_out_meter_byte_count_adjust_set(EIP_OUT_METER_ADJUST_BYTE_COUNT);
}

static void eip6_out_meter_adjust_init() {
	eip6_out_meter_adjust_setup();
	eip6_out_meter_byte_count_adjust_set(EIP6_OUT_METER_ADJUST_BYTE_COUNT);
}

static void eip_out_share_meter_adjust_init() {
	eip_out_share_meter_adjust_setup();
	eip_out_share_meter_byte_count_adjust_set(EIP_OUT_SHARE_METER_ADJUST_BYTE_COUNT);
}

void meter_adjust_init() {
	eip_in_meter_adjust_init();
	//eip6_in_meter_adjust_init();
	eip_in_share_meter_adjust_init();

	eip_out_meter_adjust_init();
	//eip6_out_meter_adjust_init();
	eip_out_share_meter_adjust_init();
}

