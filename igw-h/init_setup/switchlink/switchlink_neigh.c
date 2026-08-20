/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <netlink/netlink.h>
#include <netlink/msg.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "utils.h"
#include "switchlink.h"
#include "switchlink_neigh.h"
#include "switch_hostif.h"
#include "bf_outer_mac_rewrite_table.h"

static struct jd_hlist_head *mac_entry_hash_table = NULL;

static mac_rewrite_entry_t * mac_entry_get(uint32_t ifindex){
	uint32_t key;
	mac_rewrite_entry_t *entry;
    struct jd_hlist_node *pos;
	key = ifindex & MAC_ENTRY_HASH_TBL_MASK;
    jd_hlist_for_each_entry(entry, pos, &mac_entry_hash_table[key], hlist) {
        if (entry->ifindex == ifindex)
            return entry;
    }
	
	return NULL;
}

static void hostif_array_to_mac_entry_table(void)
{
	uint16_t i;
	uint32_t key;
	switch_hostif_t *hostif = NULL;
	mac_rewrite_entry_t *mac_entry = NULL;

	for (i = 0; i < g_hostif_info_array.hostif_num; i++) {
		hostif = &g_hostif_info_array.hostifs[i].hostif;
		mac_entry = malloc(sizeof(mac_rewrite_entry_t));
		if (!mac_entry) 
			SETUP_PANIC("malloc mac_rewrite_entry_t fail \n");
		memset(mac_entry, 0, sizeof(mac_rewrite_entry_t));
		
		strncpy(mac_entry->intf_name, hostif->intf_name, IFNAMSIZ);
		mac_entry->ifindex = if_nametoindex(mac_entry->intf_name);
		if (mac_entry->ifindex == 0) 
			SETUP_PANIC("if_nametoindex fail for nic:%s \n",mac_entry->intf_name);
		mac_entry->hostif_ip = hostif->v4addr.addr.ip4;
		mac_entry->gw_ip = hostif->gw_ip;
		memcpy(mac_entry->smac, hostif->mac, 6);		
		mac_entry->dev_port = hostif->dev_port;
		mac_entry->valid = 0;
		key = mac_entry->ifindex & MAC_ENTRY_HASH_TBL_MASK;
		jd_hlist_add_head(&mac_entry->hlist, &mac_entry_hash_table[key]);
	}	
}

void process_neigh_msg(struct nlmsghdr *nlmsg, int type) 
{
	int len = nlmsg->nlmsg_len;
	struct ndmsg *neighbor = NLMSG_DATA(nlmsg);
	struct rtattr *tb[NDA_MAX + 1];	
	int mac_addr_valid = 0;
  	int ipaddr_valid = 0;
  	int need_write_flag = 0;
    struct in_addr v4addr;
	uint8_t dmac_addr[6];
	mac_rewrite_entry_t *mac_entry;
	int ret, diff_mac;

	if (type != RTM_NEWNEIGH)
		return ;
	len -= NLMSG_LENGTH(sizeof(*neighbor));
	if (len < 0) 
		return ;
	if (neighbor->ndm_family != AF_INET) 
		return ;
	
	mac_entry = mac_entry_get(neighbor->ndm_ifindex);
	if (!mac_entry) {
		return ;
	}

	parse_rtattr_flags(tb, NDA_MAX, RTM_RTA(neighbor), len, 0);
	if (tb[NDA_DST]) {
		ipaddr_valid = 1;		
		memcpy(&v4addr, RTA_DATA(tb[NDA_DST]), RTA_PAYLOAD(tb[NDA_DST]));
		//printf ("ipaddr:%s\n", inet_ntoa(v4addr));
		if (mac_entry->gw_ip != ntohl(v4addr.s_addr)) {
			return ;
		}
	}

	if (tb[NDA_LLADDR]) {
        memcpy(&dmac_addr, RTA_DATA(tb[NDA_LLADDR]), 6);
		if (!is_valid_ether_addr(dmac_addr))
			return ;	
		mac_addr_valid = 1;	
		//printf("mac:" MAC_FMT "\n", MAC_ARG(dmac_addr));
	}

	if (!ipaddr_valid || !mac_addr_valid) 
			return ;

	if (!mac_entry->valid) {
		need_write_flag = 1;
	} else {
		diff_mac = memcmp(mac_entry->dmac, dmac_addr, 6);
		if (diff_mac) {
			need_write_flag = 1;
		}
	}

	if (need_write_flag == 0) {		
		return ;
	}

	ret = mac_rewrite_table_entry_add(mac_entry->dev_port,
		mac_entry->smac, dmac_addr);
	if (ret == 0) {
		memcpy(mac_entry->dmac, dmac_addr, 6);			
		mac_entry->valid = 1;
		SETUP_LOG("write tofino arp table for port: %s\n",mac_entry->intf_name);
	} 
}

void neigh_system_init(){
	uint32_t i, size;

	mac_rewrite_table_setup();

	size = MAC_ENTRY_HASH_TBL_SIZE * sizeof(struct jd_hlist_head);
	mac_entry_hash_table = (struct jd_hlist_head *)malloc(size);
	if (mac_entry_hash_table == NULL) {
        SETUP_PANIC("init mac_entry_hash_table error\n");
        return ;
    }
	
	for (i = 0; i < MAC_ENTRY_HASH_TBL_SIZE; i++) {
        JD_INIT_HLIST_HEAD(&mac_entry_hash_table[i]);
    }

	hostif_array_to_mac_entry_table();
}
