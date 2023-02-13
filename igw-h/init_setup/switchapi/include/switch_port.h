/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _SWITCH_PORT_INIT_H__
#define _SWITCH_PORT_INIT_H__

#include <bf_pm/bf_pm_intf.h>
#include <tofino/bf_pal/bf_pal_port_intf.h>
#include <tofino/bf_pal/dev_intf.h>
#include <knet_mgr/bf_knet_if.h>

#define SWITCH_PORT_INVALID -1

#define SWITCH_CPU_PORT_ETH_DEFAULT 501
#define SWITCH_CPU_PORT_PCIE_DEFAULT 502

#define SWITCH_PORT_RX_MTU_DEFAULT 2000
#define SWITCH_PORT_TX_MTU_DEFAULT 2000
#define MTU_ADJUST 40

#define SWITCH_PORT_INTERNAL(_port) (_port == SWITCH_CPU_PORT_PCIE_DEFAULT)

typedef enum switch_port_oper_status_s {
  SWITCH_PORT_OPER_STATUS_NONE = 0,
  SWITCH_PORT_OPER_STATUS_UNKNOWN = 1,
  SWITCH_PORT_OPER_STATUS_UP = 2,
  SWITCH_PORT_OPER_STATUS_DOWN = 3,
  SWITCH_PORT_OPER_STATUS_MAX
} switch_port_oper_status_t;

typedef enum switch_port_speed_s {
  SWITCH_PORT_SPEED_NONE = 0,
  SWITCH_PORT_SPEED_10G = 1,
  SWITCH_PORT_SPEED_25G = 2,
  SWITCH_PORT_SPEED_40G = 3,
  SWITCH_PORT_SPEED_50G = 4,
  SWITCH_PORT_SPEED_100G = 5
} switch_port_speed_t;

typedef enum switch_port_fec_mode_s {
  SWITCH_PORT_FEC_MODE_NONE = 0,
  SWITCH_PORT_FEC_MODE_FC = 1,
  SWITCH_PORT_FEC_MODE_RS = 2
} switch_port_fec_mode_t;

typedef struct switch_api_port_info_s {
  uint32_t port;
  switch_port_speed_t port_speed;
  bool initial_admin_state;
  uint32_t tx_mtu;
  uint32_t rx_mtu;
  switch_port_fec_mode_t fec_mode;  
} switch_api_port_info_t;

extern int switch_dev_port_get(bf_dev_id_t device, 
				uint32_t fp_port, uint32_t *dev_port);
extern int switch_max_ports_get(bf_dev_id_t device, uint32_t *max_ports);
extern int switch_max_pipes_get(bf_dev_id_t device, uint32_t *max_pipes);
extern int switch_port_list_get(bf_dev_id_t device, uint32_t max_ports, 
				uint32_t *fp_list, uint32_t *dev_port_list);
extern int switch_api_port_add(bf_dev_id_t device,
                        switch_api_port_info_t *api_port_info,
                        int skip_lpbk);
extern int switch_pd_recirc_port_list_get(
    bf_dev_id_t device, uint32_t *max_recirc_ports, uint32_t *recirc_port_list);
extern int switch_pd_port_fec_set(bf_dev_id_t device,
          uint32_t dev_port, switch_port_fec_mode_t switch_fec_mode);
extern void port_link_change_notify_register();
#endif

