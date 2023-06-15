/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __SWITCH_CONFIG_H__
#define __SWITCH_CONFIG_H__

#include <jansson.h>
#include <stdint.h>

#define IPV6_ENBALE  1
#define IPV6_DISABLE 0

#define CONST_MAX_PORT_NAME_LEN 15
#define CONST_IPV4_ADDR_LEN 16
#define CONST_IPV6_ADDR_LEN 128
#define CONST_MAC_ADDR_LEN 32

typedef struct switch_cfg_hostif_s {
	char intf_name[CONST_MAX_PORT_NAME_LEN + 1];
	char ip_addr[CONST_IPV4_ADDR_LEN + 1];		
	char gw_ip_addr[CONST_IPV4_ADDR_LEN + 1];	
	uint16_t net_mask_len;
	char ip6_addr[CONST_IPV6_ADDR_LEN + 1];
	uint8_t ip6_prefix_len;
	uint8_t ipv6_enable;
	char mac_str[CONST_MAC_ADDR_LEN + 1];	
	uint8_t mac[6];
	uint16_t fp_port;
} switch_cfg_hostif_t;

#define HOSTIFS_SIZE  128
typedef struct switch_config_s {
	switch_cfg_hostif_t hostifs[HOSTIFS_SIZE];
	int hostif_num;
	char mgt_ip_addr[CONST_IPV4_ADDR_LEN + 1];
	char vip_addr[CONST_IPV4_ADDR_LEN + 1];	
	char backup_vip_addr[CONST_IPV4_ADDR_LEN + 1];	
	uint32_t mgt_ip;
	uint32_t vip;		
	uint32_t shared_bw_vip;	
	uint32_t backup_vip;
	uint16_t  port_speed;	
	uint16_t  hardware_model;
} switch_config_t;

extern 	switch_config_t switch_cfg;
extern int switch_config_init(void);


#endif
