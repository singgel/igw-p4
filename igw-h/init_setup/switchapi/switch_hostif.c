/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include <netinet/in.h>
#include <linux/if.h>
#include <libnl3/netlink/netlink.h>
#include <libnl3/netlink/route/link.h>
#include <libnl3/netlink/route/addr.h>
#include <errno.h>

#include <assert.h>
#include "utils.h"
#include "switch_packet.h"
#include "switch_hostif.h"
#include "switch_port.h"
#include "switch_config.h"

switch_hosif_info_array g_hostif_info_array;

static struct jd_hlist_head *hostif_hash_table = NULL;

struct in6_ifreq {
    struct in6_addr ifr6_addr;
    unsigned int ifr6_prefixlen;
    unsigned int ifr6_ifindex;
};

/*
* After this operation succeeds the user should be able to see 
* an interface with intf_name in the list of interfaces 
* as shown by ifconfig -a.
*/
int switch_pkt_hostif_create(uint16_t device,
                         const switch_hostif_t *hostif,
                         const uint64_t flags, uint64_t *knet_hostif_handle) {    
  	bf_knet_hostif_knetdev_t hostif_knetdev;
  	bf_status_t status = BF_SUCCESS;
 	struct ifreq ifr;
  	int rc = 0;
  	int sock_fd = 0;
  	const char *intf_name = hostif->intf_name;

  	strncpy(hostif_knetdev.name, intf_name, IFNAMSIZ);
  	status = bf_knet_hostif_kndev_add(knet_cpuif_id, &hostif_knetdev);
  	if (status != BF_SUCCESS) {
    	printf("hostif create failed, status = %d\n", status);
    	return -1;
  	}

  	*knet_hostif_handle = hostif_knetdev.knet_hostif_id;

  	sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  	if (sock_fd < 0) {
    	printf("hostif netdev socket create failed\n");
		status = bf_knet_hostif_kndev_delete(knet_cpuif_id, hostif_knetdev.knet_hostif_id);
		assert(status == BF_SUCCESS);
    	return -1;
  	}

  	if (flags & SWITCH_PKT_HOSTIF_ATTR_IPV4_ADDRESS) {
    	struct sockaddr_in sin;
    	memset(&sin, 0x0, sizeof(struct sockaddr));
    	memset(&ifr, 0x0, sizeof(ifr));
    	strncpy(ifr.ifr_name, intf_name, IFNAMSIZ);
    	sin.sin_family = AF_INET;
    	sin.sin_addr.s_addr = ntohl(hostif->v4addr.addr.ip4);
    	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    	rc = ioctl(sock_fd, SIOCSIFADDR, &ifr);
    	if (rc < 0) {
      		printf("ioctl create ipv4addr failed\n");
      		goto cleanup;
    	}
    	sin.sin_addr.s_addr = ntohl(SWITCH_IPV4_COMPUTE_MASK(hostif->v4addr.len));
    	memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
    	rc = ioctl(sock_fd, SIOCSIFNETMASK, &ifr);
    	if (rc < 0) {
      		printf("ioctl create ipv4mask failed\n");
      		goto cleanup;
    	}
  	}

  	if (hostif->ipv6_enable) {
		int sockfd6; 
		struct in6_ifreq ifreq6;
		 
		sockfd6 = socket(AF_INET6, SOCK_DGRAM, 0); 
		if (sockfd6 < 0) {
      		goto cleanup;
		}
		
    	memset(&ifr, 0x0, sizeof(ifr));
    	strncpy(ifr.ifr_name, intf_name, IFNAMSIZ);
		if (ioctl(sockfd6, SIOCGIFINDEX, &ifr) < 0){
      		printf("ioctl SIOCGIFINDEX failed\n");
			close(sockfd6); 
      		goto cleanup;
		}
		
		memcpy((void *)&ifreq6.ifr6_addr,(void *)&hostif->ip6,sizeof(struct in6_addr));
		ifreq6.ifr6_prefixlen = hostif->ip6_prefix_len; 
		ifreq6.ifr6_ifindex = ifr.ifr_ifindex;
		if(ioctl(sockfd6, SIOCSIFADDR, &ifreq6) < 0) {
			printf("ioctl SIOCSIFADDR ipv6 addr failed\n");
			close(sockfd6); 
			goto cleanup;
		}
		close(sockfd6); 
	}
	
  	if (flags & SWITCH_PKT_HOSTIF_ATTR_MAC_ADDRESS) {
    	memset(&ifr, 0x0, sizeof(ifr));
    	memcpy(ifr.ifr_hwaddr.sa_data, hostif->mac, ETH_LEN);
    	strncpy(ifr.ifr_name, intf_name, IFNAMSIZ);
    	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    	rc = ioctl(sock_fd, SIOCSIFHWADDR, &ifr);
    	if (rc < 0) {
      		printf("ioctl create hwaddr failed\n");
      		goto cleanup;
    	}
  	}

  	memset(&ifr, 0x0, sizeof(ifr));
  	strncpy(ifr.ifr_name, intf_name, IFNAMSIZ);
  	ifr.ifr_flags &= ~IFF_UP;
  	if (hostif->admin_state) {
    	ifr.ifr_flags |= IFF_UP;		
		ifr.ifr_flags |= IFF_MULTICAST;
  	} else {
    	ifr.ifr_flags &= ~IFF_UP;
  	}

  	rc = ioctl(sock_fd, SIOCSIFFLAGS, &ifr);
  	if (rc < 0) {
    	printf("ioctl create admin_state failed\n");
		goto cleanup;
  	}
	
  	close(sock_fd);
  	return 0;
	
cleanup:
	status = bf_knet_hostif_kndev_delete(knet_cpuif_id, hostif_knetdev.knet_hostif_id);
	assert(status == BF_SUCCESS);	
  	close(sock_fd);
	return -1;
}

int switch_pkt_hostif_delete(uint64_t knet_hostif_handle) {
	bf_status_t status = BF_SUCCESS;
	
  	status = bf_knet_hostif_kndev_delete(knet_cpuif_id, knet_hostif_handle);
  	if (status != BF_SUCCESS) {
    	printf("hostif delete failed, status = %d\n", status);
      	return -1;
  	}
  	return 0;
}

int switch_api_hostif_create(uint16_t device, 
	switch_hostif_info_t *hostif_info, uint64_t flags) {
	int ret;
	switch_pktdriver_rx_filter_key_t rx_key;
	switch_pktdriver_rx_filter_action_t rx_action;
	switch_pktdriver_tx_filter_key_t tx_key;
	switch_pktdriver_tx_filter_action_t tx_action;

	hostif_info->hostif.admin_state = true;
	if(switch_pkt_hostif_create(device, &hostif_info->hostif, 
			flags, &hostif_info->knet_hostif_handle) < 0) {
    	printf("switch_pkt_hostif_create failed\n");
		return -1;
	}

	memset(&rx_key, 0x0, sizeof(rx_key));
  	memset(&rx_action, 0x0, sizeof(rx_action));
    switch_dev_port_get(device, hostif_info->hostif.fp_port,
		&hostif_info->hostif.dev_port);
	rx_key.dev_port = hostif_info->hostif.dev_port;
	rx_action.knet_hostif_handle = hostif_info->knet_hostif_handle;
	flags = SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT;
	ret = switch_knet_rx_filter_create(device,
			SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PORT,
			flags,
			&rx_key,
			&rx_action,
			&hostif_info->rx_filter_handle);
	if (ret < 0) {
		printf("switch_knet_rx_filter_create failed\n");
		return -1;
	}
	
	memset(&tx_key, 0x0, sizeof(tx_key));
  	memset(&tx_action, 0x0, sizeof(tx_action));
	tx_key.knet_hostif_handle = hostif_info->knet_hostif_handle;
	tx_action.bypass_flags = SWITCH_BYPASS_ALL;
	tx_action.dev_port = hostif_info->hostif.dev_port;
	ret = switch_knet_tx_filter_create(device,
			0,
			&tx_key,
			&tx_action,
			&hostif_info->tx_filter_handle);
	if (ret < 0) {
		printf("switch_knet_tx_filter_create failed\n");
		return -1;
	}
	
	return 0;
}

void parser_switch_config_to_hostif(void)
{
	uint16_t i;
	switch_hostif_t *hostif = NULL;
	switch_cfg_hostif_t *cfg_hostif = NULL;

	assert(switch_cfg.hostif_num <= HOSTIF_ARRAY_SIZE);
	memset(&g_hostif_info_array, 0,sizeof(g_hostif_info_array));
	g_hostif_info_array.hostif_num = switch_cfg.hostif_num;
	
	for (i = 0; i < switch_cfg.hostif_num; i++) {
		hostif = &g_hostif_info_array.hostifs[i].hostif;
		cfg_hostif = &switch_cfg.hostifs[i];
		strncpy(hostif->intf_name, cfg_hostif->intf_name, IFNAMSIZ);
		memcpy(hostif->mac, cfg_hostif->mac, 6);
		hostif->admin_state = 1;
		hostif->fake_fp_port = cfg_hostif->fp_port;	
		hostif->fp_port = (hostif->fake_fp_port - 1) * 4;
		hostif->v4addr.len = cfg_hostif->net_mask_len;
		hostif->v4addr.addr.addr_family = SWITCH_IP_ADDR_FAMILY_IPV4;
		hostif->v4addr.addr.ip4 = ip_atoi(cfg_hostif->ip_addr);
		hostif->gw_ip = ip_atoi(cfg_hostif->gw_ip_addr);
		hostif->ipv6_enable = cfg_hostif->ipv6_enable;

		//ipv6 config
		if (hostif->ipv6_enable) {
			assert(ip6_atoi(cfg_hostif->ip6_addr, &hostif->ip6) == 0);
			hostif->ip6_prefix_len = cfg_hostif->ip6_prefix_len;
			memset(&hostif->ip6_mc, 0,sizeof(struct in6_addr));
			ipv6_addr_solict_mult_set(&hostif->ip6, &hostif->ip6_mc);
		}
	}	
}

int mirror_hostif_create(uint16_t device, 
	switch_hostif_info_t *hostif_info, uint64_t flags) {
	int ret;
	switch_pktdriver_rx_filter_key_t rx_key;
	switch_pktdriver_rx_filter_action_t rx_action;
	switch_pktdriver_tx_filter_key_t tx_key;
	switch_pktdriver_tx_filter_action_t tx_action;

	hostif_info->hostif.admin_state = true;
	if(switch_pkt_hostif_create(device, &hostif_info->hostif, 
			flags, &hostif_info->knet_hostif_handle) < 0) {
    	printf("switch_pkt_hostif_create failed\n");
		return -1;
	}

	memset(&rx_key, 0x0, sizeof(rx_key));
  	memset(&rx_action, 0x0, sizeof(rx_action));
    
	rx_key.dev_port = MIRROR_DEV_PORT;
	rx_action.knet_hostif_handle = hostif_info->knet_hostif_handle;
	flags = SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT;
	ret = switch_knet_rx_filter_create(device,
			SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PORT,
			flags,
			&rx_key,
			&rx_action,
			&hostif_info->rx_filter_handle);
	if (ret < 0) {
		printf("switch_knet_rx_filter_create failed\n");
		return -1;
	}
	
	memset(&tx_key, 0x0, sizeof(tx_key));
  	memset(&tx_action, 0x0, sizeof(tx_action));
	tx_key.knet_hostif_handle = hostif_info->knet_hostif_handle;
	tx_action.bypass_flags = SWITCH_BYPASS_ALL;
	tx_action.dev_port = MIRROR_DEV_PORT;
	ret = switch_knet_tx_filter_create(device,
			0,
			&tx_key,
			&tx_action,
			&hostif_info->tx_filter_handle);
	if (ret < 0) {
		printf("switch_knet_tx_filter_create failed\n");
		return -1;
	}
	
	return 0;
}

int switch_hostif_set_interface_oper_state(
    const char *intf_name, bool state) {
  	int err;
  	struct nl_sock *sock;
  	struct rtnl_link *p_rtnl_link, *change;
  	unsigned char opstate = 0;
  	bool sock_created = false;

  	sock = nl_socket_alloc();
  	if (!sock) {
     	return -1;
  	}
	
  	// connect to socket
   	if ((err = nl_connect(sock, NETLINK_ROUTE))) {
     	nl_socket_free(sock);
     	return -1;
   	}
	
   	sock_created = true;
  	nl_socket_disable_seq_check(sock);
   	if (rtnl_link_get_kernel(sock, 0, intf_name, &p_rtnl_link) < 0)
    	printf("Cannot get link by name %s\n", intf_name);

  	change = rtnl_link_alloc();
  	opstate = rtnl_link_get_operstate(p_rtnl_link);
  	rtnl_link_set_operstate(change, opstate);
  	if (state == 0) {
    	rtnl_link_set_carrier(change, 0);
    	rtnl_link_set_operstate(change, IF_OPER_LOWERLAYERDOWN);
  	} else {
    	rtnl_link_set_carrier(change, 1);
    	rtnl_link_set_operstate(change, IF_OPER_UP);
  	}

  	err = rtnl_link_change(sock, p_rtnl_link, change, 0);
  	rtnl_link_put(p_rtnl_link);
  	rtnl_link_put(change);
  	if (sock_created) {
    	nl_close(sock);
    	nl_socket_free(sock);
  	}
	
  	return 0;
}

int set_hostif_admin_state(const char *intf_name,
                      int state) {
  int rc = 0;
  struct ifreq ifr;
  int sock_fd = 0;

  sock_fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (sock_fd < 0) {
    printf ("socket create failed\n");
    return -1;
  }

  memset(&ifr, 0x0, sizeof(ifr));
  strncpy(ifr.ifr_name, intf_name, IFNAMSIZ);
  if (state) {
    ifr.ifr_flags |= IFF_UP;	
    ifr.ifr_flags |= IFF_MULTICAST;
  } else {
    ifr.ifr_flags &= ~IFF_UP;
  }

  rc = ioctl(sock_fd, SIOCSIFFLAGS, &ifr);
  if (rc < 0) {
    printf ("ioctl update admin status failed\n");	
	close(sock_fd);
    return -1;
  }

  close(sock_fd);
  return 0;
}

void create_hostif(uint16_t device){
	uint16_t index;
	uint64_t flags;	
	int ret;
	
	for (index = 0; index < g_hostif_info_array.hostif_num; index++) {
		switch_hostif_info_t *hostif_info = &g_hostif_info_array.hostifs[index];
		flags = SWITCH_PKT_HOSTIF_ATTR_IPV4_ADDRESS | SWITCH_PKT_HOSTIF_ATTR_MAC_ADDRESS;
		ret = switch_api_hostif_create(device, hostif_info, flags);
		if (ret < 0 ) {
			SETUP_PANIC("switch_api_hostif_create failed! index =%d\n", index);
			return ;
		}
		
		ret = switch_hostif_set_interface_oper_state(hostif_info->hostif.intf_name, true);
		if (ret < 0 ) {
			SETUP_PANIC("switch_hostif_set_interface_oper_state failed! index =%d\n", index);
			return ;
		}
	}
}

hostif_entry_t * hostif_entry_get(uint32_t dev_port) {
	uint32_t key;
	hostif_entry_t *entry;
    struct jd_hlist_node *pos;
	key = dev_port & HOSTIF_HASH_TBL_MASK;
    jd_hlist_for_each_entry(entry, pos, &hostif_hash_table[key], hlist) {
        if (entry->dev_port == dev_port)
            return entry;
    }
	return NULL;
}

static void hostif_array_to_hash_table(void)
{
	uint16_t i;
	uint32_t key;
	switch_hostif_t *hostif = NULL;
	hostif_entry_t *hostif_entry = NULL;

	for (i = 0; i < g_hostif_info_array.hostif_num; i++) {
		hostif = &g_hostif_info_array.hostifs[i].hostif;
		hostif_entry = malloc(sizeof(hostif_entry_t));
		if (!hostif_entry) 
			SETUP_PANIC("malloc hostif_entry_t fail \n");
		memset(hostif_entry, 0, sizeof(hostif_entry_t));
		
		strncpy(hostif_entry->intf_name, hostif->intf_name, IFNAMSIZ);
		hostif_entry->dev_port = hostif->dev_port;		
		hostif_entry->fp_port = hostif->fp_port;
		key = hostif_entry->dev_port & HOSTIF_HASH_TBL_MASK;
		jd_hlist_add_head(&hostif_entry->hlist, &hostif_hash_table[key]);
	}	
}

void hostif_hash_table_init() {
	uint32_t i, size;

	size = HOSTIF_HASH_TBL_SIZE * sizeof(struct jd_hlist_head);
	hostif_hash_table = (struct jd_hlist_head *)malloc(size);
	if (hostif_hash_table == NULL) {
        SETUP_PANIC("init hostif_hash_table error\n");
        return ;
    }
	
	for (i = 0; i < HOSTIF_HASH_TBL_SIZE; i++) {
        JD_INIT_HLIST_HEAD(&hostif_hash_table[i]);
    }

	hostif_array_to_hash_table();
}
