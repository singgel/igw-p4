/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _BGW_DEFINES_
#define _BGW_DEFINES_

#define IPP_META    hdr,meta,ig_intr_md,ig_intr_from_prsr,ig_intr_md_for_dprsr,ig_tm_md
#define EPP_META    hdr,meta,eg_intr_md,eg_prsr_md,eg_dprsr_md,eg_output_md

// ----------------------------------------------------------------------------
// 1. Common protocols/types
//-----------------------------------------------------------------------------
#define ETHERTYPE_IPV4 0x0800
#define ETHERTYPE_ARP  0x0806
#define ETHERTYPE_VLAN 0x8100
#define ETHERTYPE_IPV6 0x86dd
#define ETHERTYPE_MPLS 0x8847
#define ETHERTYPE_PTP  0x88F7
#define ETHERTYPE_FCOE 0x8906
#define ETHERTYPE_ROCE 0x8915
#define ETHERTYPE_BFN  0x9000
#define ETHERTYPE_QINQ 0x8100

#define IP_PROTOCOLS_ICMP   1
#define IP_PROTOCOLS_IGMP   2
#define IP_PROTOCOLS_IPV4   4
#define IP_PROTOCOLS_TCP    6
#define IP_PROTOCOLS_UDP    17
#define IP_PROTOCOLS_IPV6   41
#define IP_PROTOCOLS_SRV6   43
#define IP_PROTOCOLS_GRE    47
#define IP_PROTOCOLS_ICMPV6 58

#define UDP_PORT_VXLAN  4789
#define UDP_PORT_ROCEV2 4791
#define UDP_PORT_GENV   6081
#define UDP_PORT_SFLOW  6343
#define UDP_PORT_MPLS   6635

#define GRE_PROTOCOLS_ERSPAN_TYPE_3 0x22EB
#define GRE_PROTOCOLS_NVGRE         0x6558
#define GRE_PROTOCOLS_IP            0x0800
#define GRE_PROTOCOLS_ERSPAN_TYPE_2 0x88BE

#define VLAN_DEPTH 2
#define MPLS_DEPTH 3

// ----------------------------------------------------------------------------
// 2. bgw custom macro
//-----------------------------------------------------------------------------
#define PKT_BRIDGE  2
#define PKT_MIRR_MD_IG 3
#define PKT_MIRR_MD_EG 4

#define RESUBMIT_WITH_DATA 1

#define VXLAN_TYPE_STD  1
#define VXLAN_TYPE_JD   2

#define SKIP_TUNNEL_MAC_REWRITE 2
#define SKIP_PACKET 3
#define MATCH_PACKET 0

#define __2_PIPELINE_BOX__  1
#ifdef __2_PIPELINE_BOX__
    #define CPU_PORT 192
    #define EGR_PIPELINE 1
#else
    #define CPU_PORT 320
    #define EGR_PIPELINE 1
#endif

#define TYPE_INGRESS_INTERNET_IN 1
#define TYPE_INGRESS_INTERNET_OUT 2

#define EGRESS_TUNNEL_TYPE_VXLAN 1

#define CPU_MIRROR_SESSION_ID  10 

#define MIRROR_INPUT  1
#define MIRROR_OUTPUT 2

#define MIRROR_DEV_PORT 0x1FF

#define  NO_NEXTHOP  1
#define  NO_DSTIP    2
#define  NO_ROUTE    3

#define __SDE_9_7_SUPPORT__ 1

#define CRC_ADJUST_BYTES 4
#define BR_ADJUST_BYTES 19

#endif /* _BGW_DEFINES_ */
