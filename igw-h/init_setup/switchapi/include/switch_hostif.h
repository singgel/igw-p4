/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __SWITCH_HOSTIF_H__
#define __SWITCH_HOSTIF_H__

#include "switch_packet.h"
#include "init_setup_list.h"

#define HOSTIF_HASH_TBL_SIZE		(1 << 6)
#define HOSTIF_HASH_TBL_MASK		(HOSTIF_HASH_TBL_SIZE - 1)

#define IPV6_LEN 16

#define MIRROR_DEV_PORT 0x1FF

/** IP address family wrapper for v4 and v6 types. None is ignored */
typedef enum _switch_ip_addr_family_t {
  SWITCH_IP_ADDR_FAMILY_NONE,
  SWITCH_IP_ADDR_FAMILY_IPV4,
  SWITCH_IP_ADDR_FAMILY_IPV6
} switch_ip_addr_family_t;

typedef uint32_t switch_ip4_t;
typedef uint8_t switch_ip6_t[IPV6_LEN];

/** IP address wrapper for v4 and v6 addresses */
typedef struct _switch_ip_address_t {
  	switch_ip_addr_family_t addr_family;
 	union {
    	switch_ip4_t ip4;
    	switch_ip6_t ip6;
  	};
} switch_ip_address_t;

typedef struct _switch_ip_prefix_t {
  	uint16_t len;
  	switch_ip_address_t addr;
} switch_ip_prefix_t;

typedef struct hostif_entry_s {
	struct jd_hlist_node hlist;
  	char intf_name[IFNAMSIZ];		
	uint32_t dev_port;
	uint32_t fp_port;	
} hostif_entry_t;

/** host interface */
typedef struct switch_hostif_s {
  	char intf_name[IFNAMSIZ];		/** netdev interface name */
  	unsigned char mac[ETH_LEN];		/** hostif mac address */
  	switch_ip_prefix_t v4addr;		/** hostif v4 ip address */
	struct in6_addr ip6;
	struct in6_addr ip6_mc;
	uint8_t ip6_prefix_len;
  	char operstatus;	 			/** oper status */
  	uint8_t admin_state;			/** admin state */
	uint32_t fake_fp_port;		    /**user view*/		
	uint32_t fp_port;	
	uint32_t dev_port;
	uint32_t gw_ip;
	uint8_t ipv6_enable;
} switch_hostif_t;

typedef struct switch_hostif_info_s {
    switch_hostif_t hostif;
    uint64_t knet_hostif_handle;
	uint64_t rx_filter_handle;
	uint64_t tx_filter_handle;
} switch_hostif_info_t;

#define HOSTIF_ARRAY_SIZE  128

typedef struct switch_hosif_info_array_s {
	int hostif_num;
	switch_hostif_info_t hostifs[HOSTIF_ARRAY_SIZE];
} switch_hosif_info_array;

extern switch_hosif_info_array g_hostif_info_array;
extern int switch_api_hostif_create(uint16_t device, 
	switch_hostif_info_t *hostif, uint64_t flags);
extern int mirror_hostif_create(uint16_t device, 
	switch_hostif_info_t *hostif_info, uint64_t flags);
extern void parser_switch_config_to_hostif(void);
extern int switch_hostif_set_interface_oper_state(const char *intf_name, 
			bool state);
extern int set_hostif_admin_state(const char *intf_name,
                      int state);
extern void create_hostif(uint16_t device);
extern hostif_entry_t * hostif_entry_get(uint32_t dev_port);
extern void hostif_hash_table_init();

#endif
