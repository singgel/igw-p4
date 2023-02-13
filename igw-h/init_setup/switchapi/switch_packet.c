/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include "switch_packet.h"
#include "utils.h"

/* cpuif_netdev ID allocated by bf_knet */
bf_knet_cpuif_t knet_cpuif_id;

/* cpuif_knetdev name assigned by bf_knet */
static char cpuif_knetdev_name[IFNAMSIZ]; 

static inline uint64_t bit_mask(uint64_t x) {
  	uint64_t temp = 1;
  	return (x >= sizeof(uint64_t) * CHAR_BIT) ? (uint64_t)-1 : (temp << x) - 1;
}

/*
* For packets sent to hostif_knetdev, the filter can also programmed to 
* strip off and add headers to the packet. The user specifies the offset 
* and size of the data (in bytes) to be stripped and also the offset and 
* size of data to be inserted, along with the data to be inserted. 
* Multiple such insertions or removals can be performed. 
* We refer to such actions as packet mutations
*/
int switch_knet_rx_filter_create(
    const uint16_t device,
    const switch_pktdriver_rx_filter_priority_t priority,
    const uint64_t flags,
    const switch_pktdriver_rx_filter_key_t *rx_key,
    const switch_pktdriver_rx_filter_action_t *rx_action,
    uint64_t *filter_id) {
  	bf_knet_rx_filter_t rx_filter;
  	switch_cpu_header_t *filter_packet_hdr = NULL;
  	switch_cpu_header_t *mask_packet_hdr = NULL;
  	bf_status_t status = BF_SUCCESS;
  	size_t cpu_hdr_offset = 0;

  	memset(&rx_filter, 0, sizeof(bf_knet_rx_filter_t));	
	/* Filter priority (int). Lower value higher priority */
  	rx_filter.spec.priority = priority;
  	/* We index into an array hence -1 */
  	cpu_hdr_offset = sizeof(switch_ethernet_header_t) + sizeof(switch_fabric_header_t) - 2;
  	// Check filter size vs packet size
  	filter_packet_hdr = (switch_cpu_header_t *)(rx_filter.spec.filter + cpu_hdr_offset);
  	mask_packet_hdr = (switch_cpu_header_t *)(rx_filter.spec.mask + cpu_hdr_offset);

  	if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT) {
    	filter_packet_hdr->ingress_port = htons(rx_key->dev_port);
    	mask_packet_hdr->ingress_port = bit_mask(8 * sizeof(filter_packet_hdr->ingress_port));
  	}
	
  	if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_BD) {
    	filter_packet_hdr->ingress_bd = htons(rx_key->bd);
    	mask_packet_hdr->ingress_bd = bit_mask(8 * sizeof(filter_packet_hdr->ingress_bd));
  	}
	
  	if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_REASON_CODE) {
    	filter_packet_hdr->reason_code = htons(rx_key->reason_code);
    	mask_packet_hdr->reason_code = bit_mask(8 * sizeof(filter_packet_hdr->reason_code));
  	}
	
  	if (flags & SWITCH_PKTDRIVER_RX_FILTER_ATTR_IFINDEX) {
    	filter_packet_hdr->ingress_ifindex = htons(rx_key->ifindex);
    	mask_packet_hdr->ingress_ifindex = bit_mask(8 * sizeof(filter_packet_hdr->ingress_ifindex));
  	}
	/* Number of bytes (from start) to use for filtering */
  	rx_filter.spec.filter_size = sizeof(switch_ethernet_header_t) + sizeof(switch_cpu_header_t) 
						+ sizeof(switch_fabric_header_t);

	rx_filter.action.dest_proto = 0;
  	if (rx_action->knet_hostif_handle) {
    	rx_filter.action.dest_type = BF_KNET_FILTER_DESTINATION_HOSTIF;
		/* Destination netdev when dest type is BF_FILTER_DESTINATION_HOSTIF */
    	rx_filter.action.knet_hostif_id = rx_action->knet_hostif_handle;
		/* Size of dynamically allocated packet mutations array*/
		rx_filter.action.count = 1;

		/*The mutation array defines a set of strip and insert operations 
	 	   to be performed on the packe*/
    	rx_filter.action.pkt_mutation = SWITCH_CALLOC(device, sizeof(bf_knet_packet_mutation_t), 1);
    	if (rx_filter.action.pkt_mutation == NULL) {
      		printf("knet hostif rx filter create failed\n");
      		return -1;
    	}
		/* Insert or strip action, one of the types BF_KNET_RX_MUT_XXX  */
    	rx_filter.action.pkt_mutation[0].mutation_type = BF_KNET_RX_MUT_STRIP;
		/* offset for stripping/inserting data (uint8_t) */
    	rx_filter.action.pkt_mutation[0].offset = offsetof(switch_ethernet_header_t, ether_type);
		/* data strip/insert length (uint8_t) */
		rx_filter.action.pkt_mutation[0].len = sizeof(switch_packet_header_t);
  	} else {
    	rx_filter.action.dest_type = BF_KNET_FILTER_DESTINATION_CPUIF;
    	rx_filter.action.count = 0;
  	}

  	status = bf_knet_rx_filter_add(knet_cpuif_id, &rx_filter);
  	if (status != BF_SUCCESS) {
    	printf("bf_knet_rx_filter_add failed, status = %d\n", status);
    	return -1;
  	}

  	*filter_id = rx_filter.spec.filter_id;
  	if (rx_filter.action.count > 0) 
		SWITCH_FREE(device, rx_filter.action.pkt_mutation);

	return 0;
}

int switch_pktdriver_rx_filter_delete(
    const uint16_t device,
    const uint64_t filter_id) {
 	bf_status_t status = BF_SUCCESS;

  	status = bf_knet_rx_filter_delete(knet_cpuif_id, filter_id);
  	if (status != BF_SUCCESS) {
    	printf("bf_knet_rx_filter_delete failed, status = %d\n", status);
    	return -1;
  	}
  	return 0;
}

/*
 * In the TX direction bf_knet supports actions for inserting data at a 
 * user-specified byte offset for hostif_knetdevs.
 * There can be only one action per hostif_knetdev
*/
int switch_knet_tx_filter_create(
    const uint16_t device,
    const switch_pktdriver_tx_filter_priority_t priority,
    const switch_pktdriver_tx_filter_key_t *tx_key,
    const switch_pktdriver_tx_filter_action_t *tx_action,
    uint64_t *tx_filter_handle) {
  	bf_knet_tx_action_t knet_tx_action;
  	switch_packet_header_t *packet_hdr = NULL;
  	bf_status_t status = BF_SUCCESS;

  	memset(&knet_tx_action, 0, sizeof(bf_knet_tx_action_t));
  	knet_tx_action.count = 1;
  	knet_tx_action.pkt_mutation = 
		SWITCH_CALLOC(device, sizeof(bf_knet_packet_mutation_t), 1);
  	if (knet_tx_action.pkt_mutation == NULL) {
    	printf("bf_knet_packet_mutation_t malloc failed\n");
    	return -1;
  	}
	
  	knet_tx_action.pkt_mutation[0].mutation_type = BF_KNET_RX_MUT_INSERT;
  	knet_tx_action.pkt_mutation[0].offset = offsetof(switch_ethernet_header_t, ether_type);
  	knet_tx_action.pkt_mutation[0].len = sizeof(switch_packet_header_t);
    if (knet_tx_action.pkt_mutation[0].offset + 
		knet_tx_action.pkt_mutation[0].len < BF_KNET_DATA_BYTES_MAX) {
    	packet_hdr = (switch_packet_header_t *)(knet_tx_action.pkt_mutation[0].data);
  	} else {
    	printf("knet ation mutaion is greater than knet max muation\n");
    	return -1;
  	}

	/* data to insert if action type is BF_KNET_RX_MUT_INSERT
        (array of uint8_t) */
  	memset(knet_tx_action.pkt_mutation[0].data, 0, BF_KNET_DATA_BYTES_MAX);
  	if (tx_action->bypass_flags == SWITCH_BYPASS_ALL) {
		packet_hdr->fabric_header.dst_port_or_group =
 					htons(tx_action->dev_port);
    	packet_hdr->cpu_header.tx_bypass = true;
  	} else {
		packet_hdr->fabric_header.dst_port_or_group = SWITCH_INVALID_HW_PORT;
    	packet_hdr->cpu_header.ingress_bd = htons(tx_action->bd);
	}
	
  	packet_hdr->cpu_header.reason_code = htons(tx_action->bypass_flags);
  	packet_hdr->fabric_header.packet_type = SWITCH_FABRIC_HEADER_TYPE_CPU;
  	packet_hdr->fabric_header.ether_type = htons(SWITCH_FABRIC_HEADER_ETHTYPE);
  	packet_hdr->cpu_header.egress_queue = SWITCH_PKTDRIVER_TX_EGRESS_QUEUE_DEFAULT;

  	status = bf_knet_tx_action_add(knet_cpuif_id, tx_key->knet_hostif_handle, &knet_tx_action);
  	return (status == BF_SUCCESS) ? 0 : -1;
}

int switch_pktdriver_tx_filter_delete(const uint16_t device, 
											uint64_t knet_hostif_id) {
  	bf_status_t status = BF_SUCCESS;
  	status = bf_knet_tx_action_delete(knet_cpuif_id, knet_hostif_id);
  	if (status != BF_SUCCESS) {
    	printf("bf_knet_tx_action_delete failed\n");
    	return -1;
  	}
  	return 0;
}

static int set_interface_admin_state(const char *intf_name,
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

/*
* After this operation succeeds the user should be able to see 
* an interface with cpuif_knetdev_name in the list of interfaces 
* as shown by ifconfig -a.
*/
int switch_pkt_cpuif_add(const uint16_t device) {
  	bf_status_t status = BF_SUCCESS;
	char cpuif_netdev_name[IFNAMSIZ] = "";
	
  	status = bf_pal_cpuif_netdev_name_get(device, cpuif_netdev_name, IFNAMSIZ);
  	if (status != BF_SUCCESS) {
    	printf("cpuif_netdev get failed, status = %d", status);
    	return -1;
  	}

  	status = bf_knet_cpuif_ndev_add(cpuif_netdev_name, cpuif_knetdev_name, &knet_cpuif_id);
  	if (status != BF_SUCCESS) {
    	printf("bf_knet_cpuif_ndev_add failed, status = %d\n", status);
    	return -1;
  	} 

	assert(set_interface_admin_state(cpuif_netdev_name, ADMIN_STATE_UP) == 0);	
  	return 0;
}

int switch_pkt_cpuif_del(const uint16_t device) {
  	bf_status_t status = BF_SUCCESS;
  	status = bf_knet_cpuif_ndev_delete(knet_cpuif_id);
  	if (status != BF_SUCCESS) {
    	printf("bf_knet_cpuif_ndev_delete failed, status = %d\n", status);
    	return -1;
  	}
  	return 0;
}

int switch_pkt_get_cpuif_cnt(uint16_t *cpuif_count) { 
  	bf_status_t status;
	status = bf_knet_get_cpuif_cnt(cpuif_count);
  	return (status == BF_SUCCESS) ? 0 : -1;
}

bool switch_pktdriver_mode_is_kernel(void) {
  return bf_knet_module_is_inited();
}

int switch_pktdriver_knet_device_add(bf_dev_id_t device) {
	if (switch_pkt_cpuif_add(device) < 0) {
		SETUP_PANIC("switch_pkt_cpuif_add fail!\n");
		return -1;
	}
   return 0;
}

