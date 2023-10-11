/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _BGW_HEADER_
#define _BGW_HEADER_

typedef bit<48> mac_addr_t;
typedef bit<32> ipv4_addr_t;
typedef bit<128> ipv6_addr_t;
typedef bit<12> vlan_id_t;

header ethernet_t {
    mac_addr_t dstAddr;
    mac_addr_t srcAddr;
    bit<16> etherType;
}

header vlan_tag_t {
    bit<3> pcp;
    bit<1> cfi;
    vlan_id_t vid;
    bit<16> etherType;
}

header ipv4_t {
    bit<4> version;
    bit<4> ihl;
    bit<6> dscp;
    bit<2> ecn;
    bit<16> totalLen;
    bit<16> identification;
    bit<3> flags;
    bit<13> fragOffset;
    bit<8> ttl;
    bit<8> protocol;
    bit<16> hdrChecksum;
    ipv4_addr_t srcAddr;
    ipv4_addr_t dstAddr;
}

header option_word_t {
    bit<32> data;
}

header ipv6_t {
    bit<4> version;
    bit<8> trafficClass;
    bit<20> flowLabel;
    bit<16> payloadLen;
    bit<8> nextHdr;
    bit<8> hopLimit;
    ipv6_addr_t srcAddr;
    ipv6_addr_t dstAddr;
}

header tcp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<32> seqNo;
    bit<32> ackNo;
    bit<4> dataOffset;
    bit<4> res;
    bit<8> flags;
    bit<16> window;
    bit<16> checksum;
    bit<16> urgentPtr;
}

header udp_t {
    bit<16> srcPort;
    bit<16> dstPort;
    bit<16> length;
    bit<16> checksum;
}

header icmp_t {
    bit<8> typeNum;
    bit<8> code;
    bit<16> hdrChecksum;
}

// Address Resolution Protocol -- RFC 6747
header arp_t {
    bit<16> hwType;
    bit<16> protoType;
    bit<8> hw_addr_len;
    bit<8> proto_addr_len;
    bit<16> opcode;
}

// VXLAN -- RFC 7348
header vxlan_t {
    bit<8>  flags;
    bit<4>  version;
    bit<20> reserved;
    bit<24> vni;
    bit<4>  tof;
    bit<4>  reserved2;
}

// Barefoot Specific Headers.
header fabric_h {
    bit<8> reserved;
    bit<3> color;
    bit<5> qos;
    bit<8> reserved2;
    bit<16> dst_port_or_group;
}

// CPU header
header cpu_h {
    bit<5> egress_queue;
    bit<1> tx_bypass;
    bit<1> capture_ts;
    bit<1> reserved;
    bit<16> ingress_port;
    bit<16> ingress_ifindex;
    bit<16> ingress_bd;
    bit<16> reason_code; // Also used as a 16-bit bypass flag.
    bit<16> ether_type;
}

header mirror_metadata_t {
     bit<8>  proto;
     bit<3>  flag;
     bit<5>  __pad0;
     bit<7>  __pad1;
     bit<9>  port;
     bit<8>  res;
     bit<48> timestamp;
}

@pa_no_overlay("ingress", "hdr.bridged_md.tunnel_nexthop")
@pa_no_overlay("ingress", "hdr.bridged_md.need_drop")
@pa_no_overlay("egress", "hdr.bridged_md.need_drop")

#ifdef __SDE_9_7_SUPPORT__
header bridge_metadata_t {
    bit<8> proto;
    bit<8> tunnel_src_id;
    bit<16> l3_ecmp_entry_idx;
    bit<16> shared_bandwidth_id;
    bit<16> tunnel_nexthop;
    bit<16> inner_mac_id;
    bit<7> pad_0;
    bit<9> igr_port;
    bit<16> outer_ethernet_type;  // must be placed in 12-13 bytes
    bit<32> eip_or_bwid;
    bit<24> lkp_vni;
    
    bit<1>  dl_pkt;
    bit<3>  pad_01;
    bit<3>  igr_tunnel_type;
    bit<1>  outer_ethernet_invalid;
    bit<1>  need_drop;
    bit<3>  pad_02;
    bit<12>  tunnel_dst_id;

    bit<3> tunnel_direct_send;
    bit<1> nlb_eip;
    bit<2> egr_tunnel_type;   
    bit<2>  meter_packet_color;

    bit<16> lkp_l4_sport;
    bit<16> lkp_l4_dport;
}
#else
header bridge_metadata_t {
    bit<8> proto;
    bit<8> tunnel_src_id;
    bit<16> l3_ecmp_entry_idx;
    bit<16> shared_bandwidth_id;
    bit<16> tunnel_nexthop;
    bit<16> inner_mac_id;
    bit<7> pad_0;
    bit<9> igr_port;
    bit<16> outer_ethernet_type;  // must be placed in 12-13 bytes
    bit<32> eip_or_bwid;
    bit<24> lkp_vni;

    bit<1>  dl_pkt;
    bit<3>  pad_01;
    bit<3>  igr_tunnel_type;
    bit<1>  outer_ethernet_invalid;
    bit<1>  need_drop;
    bit<3>  pad_02;
    bit<12>  tunnel_dst_id;
    
    bit<3> tunnel_direct_send;
    bit<1> nlb_eip;
    bit<2> egr_tunnel_type;
    bit<2> meter_packet_color;
}
#endif

struct headers_t {
    bridge_metadata_t bg_md;
    ethernet_t ethernet;
    fabric_h fabric;
    cpu_h cpu;
    ipv4_t ipv4;
    ipv6_t ipv6;
    udp_t udp;
    tcp_t tcp;
    icmp_t icmp;
    vxlan_t vxlan;
    ethernet_t inner_ethernet;
    ipv4_t inner_ipv4;
    option_word_t inner_option_word_1;
    option_word_t inner_option_word_2;
    option_word_t inner_option_word_3;
    option_word_t inner_option_word_4;
    option_word_t inner_option_word_5;
    option_word_t inner_option_word_6;
    option_word_t inner_option_word_7;
    option_word_t inner_option_word_8;
    option_word_t inner_option_word_9;
    option_word_t inner_option_word_10;
    ipv6_t inner_ipv6;
    tcp_t inner_tcp;
    udp_t inner_udp;
}

#endif /* _BGW_HEADER_ */
