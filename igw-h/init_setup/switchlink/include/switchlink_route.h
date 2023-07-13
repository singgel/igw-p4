/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __SWITCHLINK_ROUTE_H__
#define __SWITCHLINK_ROUTE_H__

#include <stdint.h>
#include "init_setup_list.h"

#define MAX_GATEWAY 64

typedef struct gateway_entry_s {
	struct jd_hlist_node hlist;	
	uint8_t  member_index;
	uint8_t  pipe;
	uint32_t fp_port;	
  	uint32_t gw_ip;			
	uint32_t dev_port;
} gateway_entry_t;

typedef struct gateway_list_s {	
  	uint32_t gw_ip[MAX_GATEWAY];		
  	uint32_t nums;			
} gateway_list_t;

#define GW_ENTRY_HASH_TBL_SIZE		(1 << 6)
#define GW_ENTRY_HASH_TBL_MASK		(GW_ENTRY_HASH_TBL_SIZE - 1)

extern void process_route_msg(struct nlmsghdr *nlmsg, int type);
extern void route_system_init(void);

#endif /* __SWITCHLINK_INT_H__ */

