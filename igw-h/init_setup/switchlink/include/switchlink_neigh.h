/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __SWITCHLINK_NEIGH_H__
#define __SWITCHLINK_NEIGH_H__

#include <stdint.h>
#include "init_setup_list.h"

typedef struct mac_rewrite_entry_s {
	struct jd_hlist_node hlist;
  	char intf_name[IFNAMSIZ];
	uint32_t ifindex;
  	unsigned char smac[6];			
  	unsigned char dmac[6];		
  	uint32_t hostif_ip;		
  	uint32_t gw_ip;			
	uint32_t dev_port;
	uint8_t  valid; 
} mac_rewrite_entry_t;

#define MAC_ENTRY_HASH_TBL_SIZE		(1 << 6)
#define MAC_ENTRY_HASH_TBL_MASK		(MAC_ENTRY_HASH_TBL_SIZE - 1)

extern void process_neigh_msg(struct nlmsghdr *nlmsg, int type);
extern void neigh_system_init(void);

#endif /* __SWITCHLINK_INT_H__ */

