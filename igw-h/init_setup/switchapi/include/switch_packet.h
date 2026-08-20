/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __SWITCH_PACKET_H__
#define __SWITCH_PACKET_H__

#include <tofino/bf_pal/dev_intf.h>
#include <knet_mgr/bf_knet_if.h>
#include <sys/ioctl.h>
#include <linux/if_ether.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <limits.h>

#define SWITCH_CALLOC(d, x, c) bf_sys_calloc(x, c)
#define SWITCH_FREE(d, x) bf_sys_free(x)

#define PACKED __attribute__((__packed__))

#define ETH_LEN 6

#define ADMIN_STATE_UP 		1
#define ADMIN_STATE_DOWN 	0

#define SWITCH_FABRIC_HEADER_ETHTYPE 0x9000
#define SWITCH_PKTDRIVER_TX_EGRESS_QUEUE_DEFAULT 7
#define SWITCH_INVALID_HW_PORT 0x1FF
#define SWITCH_IPV4_COMPUTE_MASK(_len) (0xFFFFFFFF << (32 - _len))

/** cpu header */
typedef struct PACKED switch_cpu_header_s {
  	uint8_t reserved : 1;  				/** reserved */
  	uint8_t capture_tstamp_on_tx : 1;	/** capture departure time */
  	uint8_t tx_bypass : 1;				/** tx bypass */
  	uint8_t egress_queue : 5;	 		/** egress queue id */
  	uint16_t ingress_port;	 			/** ingress port */
  	uint16_t ingress_ifindex;			/** ingress ifindex */
  	uint16_t ingress_bd;				/** ingress bridge domain */
  	uint16_t reason_code; 				/** rx - reason code, tx - tx bypass flags*/
} switch_cpu_header_t;

/** ethernet header */
typedef struct PACKED switch_ethernet_header_s {
  	uint8_t dst_mac[ETH_LEN];	/** destination mac */
  	uint8_t src_mac[ETH_LEN];	/** source mac */
  	uint16_t ether_type;		/** ethernet type */
} switch_ethernet_header_t;

/** fabric header */
typedef struct PACKED switch_fabric_header_s {
 	uint16_t ether_type;		/** fabric header ethertype */
  	uint8_t pad1 : 1;			/** padding */
  	uint8_t packet_version : 2;	/** packet version */
  	uint8_t header_version : 2;	/** header version */
  	uint8_t packet_type : 3;	/** header type - cpu/unicast/multicast */
 	uint8_t fabric_color : 3;	/** packet color */
  	uint8_t fabric_qos : 5;		/** qos value */
  	uint8_t dst_device;			/** device id */
 	uint16_t dst_port_or_group;	/** dev port or mgid */
} switch_fabric_header_t;

typedef struct PACKED switch_packet_header_s {
  	switch_fabric_header_t fabric_header;	/** fabric header */
  	switch_cpu_header_t cpu_header;			/** cpu header */
} switch_packet_header_t;

typedef enum switch_pktdriver_rx_filter_priority_s {
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_MIN = 0x0,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PORT = 0x1,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_INTERFACE = 0x2,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_VLAN = 0x3,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_LN = 0x4,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_RIF = 0x5,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO1 = 0x10,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO2 = 0x11,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO3 = 0x12,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_PRIO4 = 0x13,
  SWITCH_PKTDRIVER_RX_FILTER_PRIORITY_MAX = 0x14
} switch_pktdriver_rx_filter_priority_t;

typedef struct switch_pktdriver_rx_filter_key_s {
  	uint16_t dev_port;			/** port number */
  	uint32_t ifindex;			/** ifindex */
  	uint16_t bd;		 		/** bridge domain */
  	uint32_t reason_code;		/** reason code */
  	uint32_t reason_code_mask;	/** reason code mask */
} switch_pktdriver_rx_filter_key_t;

typedef struct switch_pktdriver_rx_filter_action_s {
  /** knet hostif handle - used with kernel packet processing */
  bf_knet_hostif_t knet_hostif_handle;
} switch_pktdriver_rx_filter_action_t;

typedef enum switch_pktdriver_tx_filter_priority_s {
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_MIN = 0x0,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_HOSTIF = 0x1,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO1 = 0x4,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO2 = 0x5,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO3 = 0x6,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_PRIO4 = 0x7,
  SWITCH_PKTDRIVER_TX_FILTER_PRIORITY_MAX = 0x8
} switch_pktdriver_tx_filter_priority_t;

typedef struct switch_pktdriver_tx_filter_key_s {
  	int hostif_fd;	/** netdev fd */
  	/** knet hostif handle - used with kernel packet processing */
  	bf_knet_hostif_t knet_hostif_handle;
} switch_pktdriver_tx_filter_key_t;

typedef struct switch_pktdriver_tx_filter_action_s {
 	uint16_t bypass_flags;	 	/** tx bypass flags */
  	uint16_t bd;				/** bd if tx bypass is false */
  	uint16_t dev_port;			/** dev port if tx bypass is true */
  	uint16_t port_lag_index; 	/** port_lag_index if tx bypass is true */
} switch_pktdriver_tx_filter_action_t;

typedef enum switch_fabric_header_type_s {
  SWITCH_FABRIC_HEADER_TYPE_NONE = 0,
  SWITCH_FABRIC_HEADER_TYPE_UNICAST = 1,
  SWITCH_FABRIC_HEADER_TYPE_MULTICAST = 2,
  SWITCH_FABRIC_HEADER_TYPE_MIRROR = 3,
  SWITCH_FABRIC_HEADER_TYPE_CONTROL = 4,
  SWITCH_FABRIC_HEADER_TYPE_CPU = 5
} switch_fabric_header_type_t;

typedef enum switch_pktdriver_rx_filter_attr_s {
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_DEV_PORT = (1 << 0),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_IFINDEX = (1 << 1),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_BD = (1 << 2),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_REASON_CODE = (1 << 3),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_ETHER_TYPE = (1 << 4),
  SWITCH_PKTDRIVER_RX_FILTER_ATTR_GLOBAL = (1 << 5)
} switch_pktdriver_rx_filter_attr_t;

typedef enum switch_pkt_hostif_attr_s {
  SWITCH_PKT_HOSTIF_ATTR_MAC_ADDRESS = (1 << 0),
  SWITCH_PKT_HOSTIF_ATTR_IPV4_ADDRESS = (1 << 1),
  SWITCH_PKT_HOSTIF_ATTR_IPV6_ADDRESS = (1 << 2),
  SWITCH_PKT_HOSTIF_ATTR_INTERFACE_NAME = (1 << 3),
  SWITCH_PKT_HOSTIF_ATTR_ADMIN_STATE = (1 << 4),
  SWITCH_PKT_HOSTIF_ATTR_OPER_STATUS = (1 << 5),
  SWITCH_PKT_HOSTIF_ATTR_VLAN_ACTION = (1 << 6),
} switch_pkt_hostif_attr_t;

typedef enum switch_tx_bypass_flags_s {
  SWITCH_BYPASS_NONE = 0x0,
  SWITCH_BYPASS_L2 = (1 << 0),
  SWITCH_BYPASS_L3 = (1 << 1),
  SWITCH_BYPASS_ACL = (1 << 2),
  SWITCH_BYPASS_QOS = (1 << 3),
  SWITCH_BYPASS_METER = (1 << 4),
  SWITCH_BYPASS_SYSTEM_ACL = (1 << 5),
  SWITCH_BYPASS_ALL = 0xFFFF
} switch_tx_bypass_flags_t;

extern bf_knet_cpuif_t knet_cpuif_id;
extern int switch_pkt_cpuif_add(const uint16_t device);
extern int switch_pkt_cpuif_del(const uint16_t device);
extern int switch_pkt_get_cpuif_cnt(uint16_t *cpuif_count);
extern  int switch_knet_rx_filter_create(
    const uint16_t device,
    const switch_pktdriver_rx_filter_priority_t priority,
    const uint64_t flags,
    const switch_pktdriver_rx_filter_key_t *rx_key,
    const switch_pktdriver_rx_filter_action_t *rx_action,
    uint64_t *filter_id);
extern int switch_pktdriver_rx_filter_delete(
    const uint16_t device,
    const uint64_t filter_id);
extern  int switch_knet_tx_filter_create(
    const uint16_t device,
    const switch_pktdriver_tx_filter_priority_t priority,
    const switch_pktdriver_tx_filter_key_t *tx_key,
    const switch_pktdriver_tx_filter_action_t *tx_action,
    uint64_t *tx_filter_handle);
extern int switch_pktdriver_tx_filter_delete(
    const uint16_t device, uint64_t knet_hostif_id);
extern bool switch_pktdriver_mode_is_kernel(void);
extern int switch_pktdriver_knet_device_add(bf_dev_id_t device);


#endif
