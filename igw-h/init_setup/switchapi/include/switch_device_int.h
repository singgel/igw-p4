/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __SWITCH_DEVICE_INT_H__
#define __SWITCH_DEVICE_INT_H__

#include <bf_pm/bf_pm_intf.h>
#include <tofino/bf_pal/bf_pal_port_intf.h>
#include <tofino/bf_pal/dev_intf.h>
#include <knet_mgr/bf_knet_if.h>
#include <bf_switchd/bf_switchd.h>
#include <traffic_mgr/traffic_mgr_counters.h>

#define DEVICE_ID 0

#define SWITCH_MAX_PIPES 4
#define TWO_PIPELINE 	2
#define FOUR_PIPELINE 	4

#define SWITCH_MAX_PORTS 512
#define SWITCH_MAX_RECIRC_PORTS 4

/** device context information */
typedef struct switch_device_context_s {
  //switch_api_device_info_t device_info; 	/** application device info */
  	uint32_t cpu_port;	 					/** cpu port number */
  	uint32_t max_pipes;						/** maximum pipes */
	uint32_t max_ports;						/** maximum ports */
  	uint32_t fp_list[SWITCH_MAX_PORTS];   	/** front port list */
  	uint32_t dp_list[SWITCH_MAX_PORTS];   	/** dev port list */
  	uint32_t eth_cpu_dev_port;				/** cpu ethernet dev port */
  	uint32_t pcie_cpu_dev_port;				/** cpu pcie dev port */
	uint32_t eth_cpu_port;				
	uint32_t pcie_cpu_port;
	bf_dev_id_t device_id;					/** device id */
	uint32_t max_recirc_ports;
	uint32_t recirc_port_list[SWITCH_MAX_RECIRC_PORTS];
} switch_device_context_t;

typedef enum switch_device_hardware_modle_s {
  SWITCH_HARDWARE_MODEL_NONE = 0,
  Wedge_100BF_65X = 1,
  Wedge_100BF_32X = 2,
} switch_device_hardware_modle_t;

typedef enum switch_device_fec_modle_s {
  FEC_NONE = 1,
  FEC_RS = 2,
} switch_device_fec_modle_t;

extern switch_device_context_t *device_ctx;
extern int switch_device_recirc_port(uint32_t port);


#endif
