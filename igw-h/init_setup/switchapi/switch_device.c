/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include "switch_port.h"
#include "switch_packet.h"
#include "switch_device.h"
#include "switch_hostif.h"
#include "switch_config.h"
#include "utils.h"

switch_device_context_t *device_ctx;
static switch_hostif_info_t mirror_hostif;

int g_cpu_pcie_port = 320;
	
int switch_device_recirc_port(uint32_t port) {
  uint32_t i = 0;

  for (i = 0; i < device_ctx->max_recirc_ports; i++) {
    if (port == device_ctx->recirc_port_list[i]) {
      return 1;
    }
  }
  return 0;
}

static int switch_device_init(bf_dev_id_t device) {	
    uint32_t port = 0;
    switch_api_port_info_t api_port_info;

	/***********************************************************
	  In box 32X: 33 connect to cpu, 33/0，33/2 two 10g interface
	  In box 65X: 65 connet to panel, so can not set fec
	  here, we skip 33/65 port create
	**********************************************************/
    memset(&api_port_info, 0, sizeof(switch_api_port_info_t));
    for (port = 0; port < (device_ctx->max_ports - 4); port += 4) {
		api_port_info.port = port;
		if (switch_cfg.port_speed == 40) {
			api_port_info.port_speed = SWITCH_PORT_SPEED_40G;
		} else {
			api_port_info.port_speed = SWITCH_PORT_SPEED_100G;
		}
		api_port_info.initial_admin_state = true;
		api_port_info.rx_mtu = SWITCH_PORT_RX_MTU_DEFAULT;
      	api_port_info.tx_mtu = SWITCH_PORT_TX_MTU_DEFAULT;
		api_port_info.fec_mode = SWITCH_PORT_FEC_MODE_NONE;
		if (switch_api_port_add(device, &api_port_info, 0) < 0) {
			SETUP_PANIC("switch_api_port_add fail!\n");
			return -1;
		}
	}

	#if 0  //目前用的设备还是旧款，不支持使用10g kr来传输,所以链路可能会出现不稳
	//33/0 33/2--->connect to cpu
	if (switch_cfg.hardware_model == Wedge_100BF_32X) {
		for (port = device_ctx->max_ports - 4; port < device_ctx->max_ports; port += 2) {
			api_port_info.port = port;
			api_port_info.port_speed = SWITCH_PORT_SPEED_10G;
			api_port_info.initial_admin_state = true;
			api_port_info.rx_mtu = SWITCH_PORT_RX_MTU_DEFAULT;
      		api_port_info.tx_mtu = SWITCH_PORT_TX_MTU_DEFAULT;
			api_port_info.fec_mode = SWITCH_PORT_FEC_MODE_NONE;
			if (switch_api_port_add(device, &api_port_info, 1) < 0) {
				SETUP_PANIC("switch_api_port_add fail!\n");
				return -1;
			}
		}
	}
	#endif
}

static int switch_mirror_hostif_create() {
	uint64_t flags;
	int ret;
	uint8_t fake_mac[6] = {0x00,0x0c,0x29,0x71,0x17,0x8e};

	strncpy(mirror_hostif.hostif.intf_name, "hbgw-dump", IFNAMSIZ);
	memcpy(mirror_hostif.hostif.mac, fake_mac, 6);
	mirror_hostif.hostif.admin_state = 1;
		
	flags = SWITCH_PKT_HOSTIF_ATTR_MAC_ADDRESS;
	ret = mirror_hostif_create(DEVICE_ID, &mirror_hostif, flags);
	if (ret < 0 ) {
		SETUP_PANIC("switch_mirror_hostif_create failed!\n");
		return -1;
	}
	
	return 0;
}

int switch_device_create(){		
	uint16_t index = 0;
	
	if (switch_config_init() < 0) {
		SETUP_PANIC("switch_config_init fail!\n");
		return -1;
	}
		
	device_ctx = (switch_device_context_t *)malloc(sizeof(switch_device_context_t));
	if (!device_ctx){
		SETUP_PANIC("malloc switch_device_context_t fail!\n");
		return -1;
	}
	memset(device_ctx, 0, sizeof(switch_device_context_t));
	device_ctx->max_pipes = SWITCH_MAX_PIPES;	
	device_ctx->device_id = DEVICE_ID;
  	for (index = 0; index < SWITCH_MAX_PORTS; index++) {
    	device_ctx->fp_list[index] = SWITCH_PORT_INVALID;
    	device_ctx->dp_list[index] = SWITCH_PORT_INVALID;
  	}
	switch_max_pipes_get(DEVICE_ID, &device_ctx->max_pipes);
	switch_max_ports_get(DEVICE_ID, &device_ctx->max_ports);
	assert(device_ctx->max_ports != 0);
	printf("init_setup max_ports :%d\n", device_ctx->max_ports);
	printf("init_setup max_pipes :%d\n", device_ctx->max_pipes);

	if (device_ctx->max_pipes == TWO_PIPELINE) {
		g_cpu_pcie_port = 192;
	} 
	
	switch_port_list_get(DEVICE_ID, device_ctx->max_ports, 
		device_ctx->fp_list, device_ctx->dp_list);
	
	switch_device_init(DEVICE_ID);
	
	parser_switch_config_to_hostif();

	assert(switch_pktdriver_mode_is_kernel());
	switch_pktdriver_knet_device_add(DEVICE_ID);
	
	create_hostif(DEVICE_ID);
	if (switch_cfg.hardware_model == Wedge_100BF_65X) {
		switch_mirror_hostif_create();
	} else { 
		// Wedge_100BF_32X also use hostif in older device.
 		switch_mirror_hostif_create();
	}
	
	hostif_hash_table_init();
	port_link_change_notify_register();
	
	return 0;
}

