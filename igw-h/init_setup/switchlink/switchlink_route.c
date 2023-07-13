/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
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
#include "switchlink_route.h"
#include "switch_hostif.h"
#include "switch_config.h"
#include "switch_device.h"
#include "switch_ecmp_group2.h"
#include "bf_ecmp_group_selector.h"

bool pipe0_memberstatus_val[PER_PIPE_PORT_NUMS];
bool pipe2_memberstatus_val[PER_PIPE_PORT_NUMS];

static  int ioctl_fd;

static struct jd_hlist_head *gateway_entry_hash_table = NULL;

static int get_member_index_from_pipe0_port(uint16_t devport) {
	int i;
	for (i = 0; i < PER_PIPE_PORT_NUMS; i++) {
		if (pipe0_devport_val[i] == devport) {
			return i;
		}
	}
	return i;
}

static int get_member_index_from_pipe2_port(uint16_t devport) {
	int i;
	for (i = 0; i < PER_PIPE_PORT_NUMS; i++) {
		if (pipe2_devport_val[i] == devport) {
			return i;
		}
	}
	return i;
}


static int get_member_index_from_port(uint8_t pipe, uint16_t dev_port) {
	if (pipe == 0) {
		return get_member_index_from_pipe0_port(dev_port);
	} else if (pipe == 2) {
		return get_member_index_from_pipe2_port(dev_port);
	}
	return PER_PIPE_PORT_NUMS;
}

static void hostif_array_to_gateways(void)
{
	uint16_t i;
	uint32_t key;
	switch_hostif_t *hostif = NULL;
	gateway_entry_t *gw = NULL;

	if (g_hostif_info_array.hostif_num > MAX_GATEWAY) {
        SETUP_PANIC("too many hostif!\n");
		return ;
	}

	if (switch_cfg.hardware_model != Wedge_100BF_65X) {
		SETUP_PANIC("only support Wedge_100BF_65X!\n");
		return ;
	}
	
	for (i = 0; i < g_hostif_info_array.hostif_num; i++) {
		hostif = &g_hostif_info_array.hostifs[i].hostif;
		
		gw = (gateway_entry_t *)malloc(sizeof(gateway_entry_t));
		if (!gw) {
			SETUP_PANIC("malloc fail!\n");
		}
		memset(gw, 0, sizeof(gateway_entry_t));
		gw->gw_ip = hostif->gw_ip;
		gw->fp_port = hostif->fp_port;		
		gw->dev_port = hostif->dev_port;
		gw->pipe = DEV_PORT_TO_PIPE(gw->dev_port);
		if ((gw->pipe != 0) && (gw->pipe != 2)) {
			SETUP_PANIC("BUG!\n");
		}
		gw->member_index = get_member_index_from_port(gw->pipe, gw->dev_port);
		assert(gw->member_index < PER_PIPE_PORT_NUMS);
		key = gw->gw_ip & GW_ENTRY_HASH_TBL_MASK;
		jd_hlist_add_head(&gw->hlist, &gateway_entry_hash_table[key]);
	}	
}

static gateway_entry_t * gw_entry_get(uint32_t gw_ip){
	uint32_t key;
	gateway_entry_t *entry;
    struct jd_hlist_node *pos;
	key = gw_ip & GW_ENTRY_HASH_TBL_MASK;
    jd_hlist_for_each_entry(entry, pos, &gateway_entry_hash_table[key], hlist) {
        if (entry->gw_ip == gw_ip)
            return entry;
    }
	
	return NULL;
}

static short get_intf_up(int fd, char *ifname)
{
    int ret;
    struct ifreq ifr;

	if (!ifname)
		return 0;
	
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
    ret = ioctl(fd, SIOCGIFFLAGS, &ifr);
    if (ret < 0) {
        return 0;
    }
	
	if (ifr.ifr_flags & IFF_UP) {
		return 1;
	}

    return 0;
}

static int process_ecmp(struct nlattr *attr, gateway_list_t *gw_list) {
	struct rtnexthop *rnh = (struct rtnexthop *)nla_data(attr);
	int attrlen = nla_len(attr);
	char ifname[IF_NAMESIZE + 1];
	switchlink_ip_addr_t gateway;
	
	gw_list->nums = 0;	
	while (RTNH_OK(rnh, attrlen)) {
	  struct rtattr *rta = RTNH_DATA(rnh);
	  if (rta->rta_type == RTA_GATEWAY) {
		gateway.ip = ntohl(*((uint32_t *)RTA_DATA(rta)));
		if (get_intf_up(ioctl_fd, if_indextoname(rnh->rtnh_ifindex, ifname)) == 1) {
			gw_list->gw_ip[gw_list->nums] = gateway.ip;
			gw_list->nums++;
			if (gw_list->nums >= MAX_GATEWAY) {
				break;
			}	
		} 
	  }
	  rnh = RTNH_NEXT(rnh);
	}
	
	if (gw_list->nums == 0) {
	  return 0;
	}

	return 1;
}

static void init_members_state(void)
{	
	int i;
	for (i = 0; i < PER_PIPE_PORT_NUMS; i++) {
		pipe0_memberstatus_val[i] = false;
		pipe2_memberstatus_val[i] = false;
	}
}

void process_route_msg(struct nlmsghdr *nlmsg, int type) 
{ 
	int hdrlen, attrlen;
  	struct nlattr *attr;
  	struct rtmsg *rmsg;
	uint8_t af = AF_UNSPEC;
	uint8_t  dst_valid = 0, gateway_valid = 0, ecmp_valid = 0;
	switchlink_ip_addr_t dst_addr;
	switchlink_ip_addr_t gateway_addr;
	gateway_list_t gw_list;
	uint32_t gw_ip = 0;
	gateway_entry_t *gw_entry;	
	struct in_addr	tmpaddr;
	int i, ret, p0_need_change , p2_need_change;
	ecmpGroupSelectorKey selector_key;

	p0_need_change = 0;
	p2_need_change = 0;
	rmsg = nlmsg_data(nlmsg);
  	hdrlen = sizeof(struct rtmsg);

	if (rmsg->rtm_family > AF_MAX) {
		return ;
  	} else {
    	af = rmsg->rtm_family;
		if ((af != AF_INET)) {
			return;
		}
  	}
	
	memset(&dst_addr, 0, sizeof(switchlink_ip_addr_t));
  	memset(&gateway_addr, 0, sizeof(switchlink_ip_addr_t));
 	gw_list.nums = 0;
 	
	attrlen = nlmsg_attrlen(nlmsg, hdrlen);
	attr = nlmsg_attrdata(nlmsg, hdrlen);
	while (nla_ok(attr, attrlen)) {
	  int attr_type = nla_type(attr);
	  switch (attr_type) {
		case RTA_DST:
		  dst_addr.ip = ntohl(nla_get_u32(attr));
		  if (dst_addr.ip)
		  	return ;
		  break;
		case RTA_GATEWAY:
		  gateway_valid = 1;
		  memset(&gateway_addr, 0, sizeof(switchlink_ip_addr_t));
		  gateway_addr.ip = ntohl(nla_get_u32(attr));
		  gateway_addr.prefix_len = 32;
		  break;
		case RTA_MULTIPATH:
		  ecmp_valid = process_ecmp(attr, &gw_list);
		  break;	  
		default:
		  break;
	  }
	  attr = nla_next(attr, &attrlen);
	}

	if (rmsg->rtm_dst_len == 0) { // default route
    	dst_valid = 1;
    	memset(&dst_addr, 0, sizeof(switchlink_ip_addr_t));
    	dst_addr.prefix_len = 0;
  	}

	if (!dst_valid) {
		return ;
	}

	if (!gateway_valid && !ecmp_valid) {
		return ;
	}

	init_members_state();

	if (!ecmp_valid && (gateway_valid == 1)) {
		gw_list.gw_ip[0] = gateway_addr.ip;
		gw_list.nums = 1;
	}

	SETUP_LOG("IGW: default route nexthop changed!\n");
	for (i == 0; i < gw_list.nums; i++) {
		gw_ip = gw_list.gw_ip[i];
		gw_entry = gw_entry_get(gw_ip);
		if (gw_entry) {
			if (gw_entry->pipe == 0) {
				p0_need_change = 1;
				pipe0_memberstatus_val[gw_entry->member_index] = true;
			} else {
				p2_need_change = 1;
				pipe2_memberstatus_val[gw_entry->member_index] = true;
			}
			tmpaddr.s_addr = htonl(gw_entry->gw_ip);
			SETUP_LOG("IGW: oif: Ethernet%d devport:%d gateway_ip: %s pipe:%d\n", 
						(gw_entry->fp_port/4) + 1,
						 gw_entry->dev_port,inet_ntoa(tmpaddr),
						 gw_entry->pipe);
		}
	}

	if (!p0_need_change && !p2_need_change)
		return ;

	if (p0_need_change) {
		selector_key.selector_groupid = PIPE0_SELECTORCID;
		ret = ecmp_group_selector_entry_add(&selector_key, PER_PIPE_PORT_NUMS,
			pipe0_memberid_val, PER_PIPE_PORT_NUMS,
			pipe0_memberstatus_val, PER_PIPE_PORT_NUMS, false);
		if (ret < 0) {
			SETUP_LOG("IGW_ERROR: igw set pipe0 ecmp state error!\n");
		}
	}

	if (p2_need_change) {
		selector_key.selector_groupid = PIPE2_SELECTORCID;
		ret = ecmp_group_selector_entry_add(&selector_key, PER_PIPE_PORT_NUMS,
			pipe2_memberid_val, PER_PIPE_PORT_NUMS,
			pipe2_memberstatus_val, PER_PIPE_PORT_NUMS, false);
		if (ret < 0) {
			SETUP_LOG("IGW_ERROR: igw set pipe2 ecmp state error!\n");
		}
	}
}

void route_system_init() {
	uint32_t i, size;

	ioctl_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (ioctl_fd < 0) {
        SETUP_PANIC("Cannot open ioctl_fd socket\n");
		return ;
	}
	
	size = GW_ENTRY_HASH_TBL_SIZE * sizeof(struct jd_hlist_head);
	gateway_entry_hash_table = (struct jd_hlist_head *)malloc(size);
	if (gateway_entry_hash_table == NULL) {
        SETUP_PANIC("init gateway_entry_hash_table error\n");
        return ;
    }
	
	for (i = 0; i < GW_ENTRY_HASH_TBL_SIZE; i++) {
        JD_INIT_HLIST_HEAD(&gateway_entry_hash_table[i]);
    }
	
	hostif_array_to_gateways();
}
