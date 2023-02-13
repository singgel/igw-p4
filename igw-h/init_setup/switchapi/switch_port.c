/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include "utils.h"
#include "switch_port.h"
#include "switch_device_int.h"
#include "switch_hostif.h"

static void set_loopback_mode(bf_dev_id_t device, bf_dev_port_t dev_port){
	bf_status_t bf_status;
	uint32_t log_pipe;
	int set_loopback = 0;

	log_pipe = DEV_PORT_TO_PIPE(dev_port);
	switch(device_ctx->max_pipes) {
		case TWO_PIPELINE:
			if (log_pipe == 0) 
				set_loopback = 1;
			break;
		case FOUR_PIPELINE:
			if (log_pipe == 1 || log_pipe == 3) 
				set_loopback = 1;
			break;
		default:
			return ;
	}

	if (set_loopback) {
		bf_status_t status;
		status = bf_pal_port_mtu_set(device, dev_port, 
			(SWITCH_PORT_TX_MTU_DEFAULT + MTU_ADJUST), 
			(SWITCH_PORT_RX_MTU_DEFAULT+ MTU_ADJUST));
  		if (status != BF_SUCCESS) {
	  		SETUP_PANIC("bf_pal_port_mtu_set faild status=%d for device %u\n", status, device);
	  		return ;
  		}
  
		/*loop back port must be fec_none*/
		if (switch_pd_port_fec_set(device, dev_port, SWITCH_PORT_FEC_MODE_NONE) < 0) {
			SETUP_PANIC("switch_pd_port_fec_set error!\n");
			return ;
		}
		
		/*BF_LPBK_MAC_NEAR, MAC Tx to Rx*/
		bf_status = bf_pal_port_loopback_mode_set(device, dev_port, BF_LPBK_MAC_NEAR);
		if (bf_status != BF_SUCCESS) {
			SETUP_PANIC("bf_pal_port_loopback_mode_set error!\n");
			return ;
		}
	}
}
	
int switch_max_ports_get(bf_dev_id_t device, uint32_t *max_ports) {
  bf_status_t bf_status;

  /* Get the max number of ports on the target*/
  bf_status = bf_pal_max_ports_get(device, max_ports);
  if (bf_status != BF_SUCCESS) {
	  SETUP_PANIC("switch_max_ports_get error!\n");
	  return -1;
  }

  return 0;
}

int switch_max_pipes_get(bf_dev_id_t device, uint32_t *max_pipes) {
  bf_status_t bf_status;

  bf_status = bf_pal_num_pipes_get(device, max_pipes);
  if (bf_status != BF_SUCCESS) {
	  SETUP_PANIC("switch_max_pipes_get error!\n");
	  return -1;
  }

  return 0;
}

int switch_port_list_get(bf_dev_id_t device, uint32_t max_ports, 
				uint32_t *fp_list, uint32_t *dev_port_list) {
  bf_status_t status;
  uint32_t index = 0;
  for (index = 0; index < max_ports; index++) {
    fp_list[index] = index;
    status = bf_pal_fp_idx_to_dev_port_map(device, fp_list[index], &dev_port_list[index]);
	if (status != BF_SUCCESS) {
    	SETUP_PANIC("forward-port to dev port map faild\n");
       	return -1;
    }
  }
  return 0;
}

int switch_dev_port_get(bf_dev_id_t device, 
				uint32_t fp_port, uint32_t *dev_port) {
  bf_status_t status;
  status = bf_pal_fp_idx_to_dev_port_map(device, fp_port, dev_port);
  if (status != BF_SUCCESS) {
    	SETUP_PANIC("forward-port to dev port map faild\n");
       	return -1;
  }
  return 0;
}

int switch_pd_recirc_port_list_get(
    bf_dev_id_t device,
    uint32_t *max_recirc_ports,
    uint32_t *recirc_port_list) {
  bf_status_t status;
  uint32_t index = 0;
  uint32_t num_ports = 0;
  uint32_t start_recirc_index = 0, end_recirc_index = 0;
  uint32_t dev_port = 0;


  for (index = 0; index < SWITCH_MAX_RECIRC_PORTS; index++) {
    recirc_port_list[index] = SWITCH_CPU_PORT_PCIE_DEFAULT + index;
  }
  
  status = bf_pal_recirc_port_range_get(device, &start_recirc_index, &end_recirc_index);
  if (status != BF_SUCCESS) {
    SETUP_PANIC("Failed to get recirc port ranges on device %d", device);
    return status;
  }

  for (index = start_recirc_index; index <= end_recirc_index; index += 4) {
    recirc_port_list[num_ports] = index;
    num_ports++;
  }

  // One Recirc port per pipe.
  *max_recirc_ports = num_ports;

  return status;
}

static int switch_device_dev_port_get(bf_dev_id_t device,
                                           uint32_t port, uint32_t *dev_port) {
  if (port == SWITCH_CPU_PORT_ETH_DEFAULT) {
    *dev_port = device_ctx->eth_cpu_dev_port;
  } else if (port == SWITCH_CPU_PORT_PCIE_DEFAULT) {
    *dev_port = device_ctx->pcie_cpu_dev_port;
  } else if (switch_device_recirc_port(port)) {
	return -1;
  } else {
    assert(device_ctx->fp_list[port] == port);
    *dev_port = device_ctx->dp_list[port];
  }

  return 0;
}

static bf_port_speed_t switch_port_speed_to_pd_port_speed(
    switch_port_speed_t port_speed) {
  switch (port_speed) {
    case SWITCH_PORT_SPEED_NONE:
      return BF_SPEED_NONE;
    case SWITCH_PORT_SPEED_10G:
      return BF_SPEED_10G;
    case SWITCH_PORT_SPEED_25G:
      return BF_SPEED_25G;
    case SWITCH_PORT_SPEED_40G:
      return BF_SPEED_40G;
    case SWITCH_PORT_SPEED_50G:
      return BF_SPEED_50G;
    case SWITCH_PORT_SPEED_100G:
      return BF_SPEED_100G;
    default:
      return BF_SPEED_NONE;
  }
}

static inline bf_fec_type_t switch_fec_mode_to_bf_fec_type(
    switch_port_fec_mode_t fec_type) {
  switch (fec_type) {
    case SWITCH_PORT_FEC_MODE_NONE:
      return BF_FEC_TYP_NONE;
    case SWITCH_PORT_FEC_MODE_FC:
      return BF_FEC_TYP_FIRECODE;
    case SWITCH_PORT_FEC_MODE_RS:
      return BF_FEC_TYP_REED_SOLOMON;
    default:
      return BF_FEC_TYP_NONE;
  }
}

int switch_pd_port_add(bf_dev_id_t device,
                                   uint32_t dev_port,
                                   switch_port_speed_t port_speed) {
  bf_status_t status;
  bf_port_speed_t pd_port_speed = BF_SPEED_NONE;
  pd_port_speed = switch_port_speed_to_pd_port_speed(port_speed);
  status = bf_pal_port_add(device, dev_port, pd_port_speed, 0x0);
  if (status != BF_SUCCESS) {
	  SETUP_PANIC("bf_pal_port_add faild status=%d for device %u\n", status, device);
	  return -1;
  }

  return 0;
}

int switch_pd_port_mtu_set(bf_dev_id_t device,
                                       uint32_t dev_port,
                                       uint32_t tx_mtu,
                                       uint32_t rx_mtu) {
  bf_status_t status;
  status = bf_pal_port_mtu_set(device, dev_port, tx_mtu, rx_mtu);
  if (status != BF_SUCCESS) {
	  SETUP_PANIC("bf_pal_port_mtu_set faild status=%d for device %u\n", status, device);
	  return -1;
  }
  return 0;
}

int switch_pd_port_fec_set(bf_dev_id_t device,
                                       uint32_t dev_port,
                                       switch_port_fec_mode_t switch_fec_mode) {
  bf_status_t status;
  bf_fec_type_t bf_fec_type = BF_FEC_TYP_NONE;

  bf_fec_type = switch_fec_mode_to_bf_fec_type(switch_fec_mode);
  status = bf_pal_port_fec_set(device, dev_port, bf_fec_type);
  if (status != BF_SUCCESS) {
	  SETUP_PANIC("bf_pal_port_fec_set faild status=%d for device %u\n", status, device);
	  return -1;
  }

  return 0;
}

int switch_pd_port_enable(bf_dev_id_t device,
                                      uint32_t dev_port) {
  bf_status_t status;
  status = bf_pal_port_enable(device, dev_port);
  if (status != BF_SUCCESS) {
	  SETUP_PANIC("bf_pal_port_enable faild status=%d for device %u\n", status, device);
	  return -1;
  }
  return status;
}

int switch_api_port_add(bf_dev_id_t device,
                        switch_api_port_info_t *api_port_info,
                        int skip_lpbk) {
	switch_port_speed_t port_speed;
	uint32_t port;
	uint32_t dev_port;
	uint32_t tx_mtu;
	uint32_t rx_mtu;
	switch_port_fec_mode_t fec_mode = SWITCH_PORT_FEC_MODE_NONE;

	port = api_port_info->port;
  	port_speed = api_port_info->port_speed;
  
  	switch_device_dev_port_get(device, port, &dev_port);
    if (api_port_info->rx_mtu) {
    	rx_mtu = api_port_info->rx_mtu;
  	}

  	if (api_port_info->tx_mtu) {
    	tx_mtu = api_port_info->tx_mtu;
  	}

    switch_pd_port_add(device, dev_port, port_speed);
	switch_pd_port_mtu_set(device, dev_port, tx_mtu, rx_mtu);
	
    fec_mode = api_port_info->fec_mode;
	
    if (fec_mode != SWITCH_PORT_FEC_MODE_NONE) {
		switch_pd_port_fec_set(device, dev_port, fec_mode);
	}

	if (api_port_info->initial_admin_state) {
      switch_pd_port_enable(device, dev_port);
    }

	if (!skip_lpbk) {	
		set_loopback_mode(device, dev_port);
	}
	
	return 0;
}

static int switch_port_state_change(int device,
                          int dev_port, bool up, void *cookie) {
    hostif_entry_t *hostif_entry = NULL;
	int ret;
	switch_port_oper_status_t oper_status = SWITCH_PORT_OPER_STATUS_NONE;
	oper_status = up ? SWITCH_PORT_OPER_STATUS_UP : SWITCH_PORT_OPER_STATUS_DOWN;

	if (oper_status == SWITCH_PORT_OPER_STATUS_DOWN) {
		SETUP_LOG("tofino devport: %d DOWN\n", dev_port);
		hostif_entry = hostif_entry_get(dev_port);		
		if (hostif_entry) {
			ret = set_hostif_admin_state(hostif_entry->intf_name, 0);
			if (ret < 0) {
				SETUP_LOG("BGW_ERROR: hostif: %s DOWN fail!\n", hostif_entry->intf_name);
			} else {
			 	SETUP_LOG("hostif: %s DOWN\n", hostif_entry->intf_name);
			}
		} 
	} else {
		SETUP_LOG("tofino devport: %d UP\n", dev_port);
		hostif_entry = hostif_entry_get(dev_port);	
		if (hostif_entry) {
			ret = set_hostif_admin_state(hostif_entry->intf_name, 1);
			if (ret < 0) {
				SETUP_LOG("BGW_ERROR: hostif: %s UP fail!\n", hostif_entry->intf_name);
			} else {
			 	SETUP_LOG("hostif: %s UP\n", hostif_entry->intf_name);
			}
		} 
	}
}

void port_link_change_notify_register() {
	bf_status_t status;
	status = bf_pal_port_status_notif_reg(switch_port_state_change, NULL);
	if (status != BF_SUCCESS) {
		SETUP_PANIC("bf_pal_port_status_notif_reg fail\n");
		return ;
	}
}
