/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

//=============================================================================
// P13 Egress parser
//=============================================================================
parser P13_EgressParser(
        packet_in pkt,
        out headers_t hdr,
        out common_metadata_t meta,
        out egress_intrinsic_metadata_t eg_intr_md) {

    state start {
        pkt.extract(eg_intr_md);
        transition parse_header_check;
    }

    state parse_header_check {
        bit<8> proto = pkt.lookahead<bit<8>>();
        transition select(proto) {
            PKT_MIRR_MD_IG : parse_ig_mirror_metadata;
            PKT_MIRR_MD_EG : parse_eg_mirror_metadata;
            PKT_BRIDGE : parse_bridge_header;
            default : accept;
        }    
    }    
    
    state parse_ig_mirror_metadata {
        pkt.extract(meta.mirror);
        transition parse_ethernet;
    }

    state parse_eg_mirror_metadata {
        pkt.extract(meta.mirror);
        transition parse_ethernet;
    } 
     
    state parse_bridge_header {
        pkt.extract(hdr.bg_md);
        transition select(hdr.bg_md.outer_ethernet_type, hdr.bg_md.outer_ethernet_invalid, hdr.bg_md.dl_pkt) {
            (ETHERTYPE_IPV4, 1w1, 1) : parse_ipv4;
            (ETHERTYPE_IPV4, 1w1, 0) : parse_ipv4_v2;
            (ETHERTYPE_IPV6, 1w1, 0) : parse_ipv6;
            (_, 1w0,_) : parse_ethernet;
            default : accept;
        }
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        meta.l3.lkp_outer_ip_proto = hdr.ipv4.protocol;
        transition select(hdr.ipv4.ihl) {
            (4w0x5) : parse_ipv4_no_options;
            default : accept;
        }
    }

    state parse_ipv4_v2 {
        pkt.extract(hdr.ipv4);
        meta.l3.lkp_outer_ip_proto = hdr.ipv4.protocol;
        transition select(hdr.ipv4.ihl) {
            (4w0x5) : parse_ipv4_no_options_v2;
            default : accept;
        }
    }

    state parse_ipv4_no_options {
        transition select(hdr.ipv4.protocol, hdr.ipv4.ihl, hdr.ipv4.fragOffset) {
            //(IP_PROTOCOLS_ICMP, 5, 0) : parse_icmp;
            (IP_PROTOCOLS_TCP, 5, 0) : parse_tcp;
            (IP_PROTOCOLS_UDP, 5, 0) : parse_udp;
            // Do NOT parse the next header if IP packet is fragmented.
            default : accept;
        }
    }

    state parse_ipv4_no_options_v2 {
        transition select(hdr.ipv4.protocol, hdr.ipv4.ihl, hdr.ipv4.fragOffset) {
            //(IP_PROTOCOLS_ICMP, 5, 0) : parse_icmp;
            (IP_PROTOCOLS_TCP, 5, 0) : parse_tcp;
            (IP_PROTOCOLS_UDP, 5, 0) : parse_udp_v2;
            // Do NOT parse the next header if IP packet is fragmented.
            default : accept;
        }
    }

    state parse_ipv6 {
        pkt.extract(hdr.ipv6);
        meta.l3.lkp_outer_ip_proto = hdr.ipv6.nextHdr;
        transition select(hdr.ipv6.nextHdr) {
            (IP_PROTOCOLS_TCP) : parse_tcp;
            (IP_PROTOCOLS_UDP) : parse_udp_v2;
            default : accept;
        }
    }
    
    state parse_udp {
        pkt.extract(hdr.udp);
        meta.l3.lkp_outer_l4_sport = hdr.udp.srcPort;
        meta.l3.lkp_outer_l4_dport = hdr.udp.dstPort;
        transition select(hdr.udp.dstPort) {
            UDP_PORT_VXLAN : parse_std_vxlan;
            default : accept;
        }
    }

    state parse_udp_v2 {
        pkt.extract(hdr.udp);
        meta.l3.lkp_outer_l4_sport = hdr.udp.srcPort;
        meta.l3.lkp_outer_l4_dport = hdr.udp.dstPort;
        transition accept;
    }

    state parse_tcp {
        pkt.extract(hdr.tcp);        
        meta.l3.lkp_outer_l4_sport = hdr.tcp.srcPort;
        meta.l3.lkp_outer_l4_dport = hdr.tcp.dstPort;
        transition accept;
    }

    /*state parse_icmp {
        pkt.extract(hdr.icmp);
        meta.l3.lkp_outer_l4_sport = 0;
        meta.l3.lkp_outer_l4_dport = 0;
        transition accept;
    }*/
    
    state parse_std_vxlan {
        pkt.extract(hdr.vxlan);
        //meta.tunnel.vxlan_type = VXLAN_TYPE_STD;
        transition select(hdr.vxlan.flags[2:2]) {
            1 : parse_jd_inner_ethernet;  
            default : parse_inner_ethernet;
        }
    }
    
    state parse_jd_inner_ethernet {
        pkt.extract(hdr.inner_ethernet);
        meta.tunnel.vxlan_type = VXLAN_TYPE_JD;
        transition select(hdr.inner_ethernet.etherType) {
            ETHERTYPE_IPV4 : parse_inner_ipv4;
            default : accept;
        }
    }  

    state parse_inner_ethernet {
        pkt.extract(hdr.inner_ethernet);
        meta.tunnel.vxlan_type = VXLAN_TYPE_STD;
        transition select(hdr.inner_ethernet.etherType) {
            ETHERTYPE_IPV4 : parse_inner_ipv4;
            default : accept;
        }
    }  

#ifdef __SDE_9_7_SUPPORT__
    state parse_inner_ipv4 {
        pkt.extract(hdr.inner_ipv4);
        meta.l3.lkp_ip_proto = hdr.inner_ipv4.protocol;
        transition accept;
    }
#else
    state parse_inner_ipv4 {
        pkt.extract(hdr.inner_ipv4);
        meta.l3.lkp_ip_proto = hdr.inner_ipv4.protocol;
        transition select(hdr.inner_ipv4.ihl) {
            (4w0x5) : parse_inner_ipv4_no_inner_options;
            (4w0x6) : parse_inner_ipv4_inner_options_1;
            (4w0x7) : parse_inner_ipv4_inner_options_2;
            (4w0x8) : parse_inner_ipv4_inner_options_3;
            (4w0x9) : parse_inner_ipv4_inner_options_4;
            (4w0xA) : parse_inner_ipv4_inner_options_5;
            (4w0xB) : parse_inner_ipv4_inner_options_6;
            (4w0xC) : parse_inner_ipv4_inner_options_7;
            (4w0xD) : parse_inner_ipv4_inner_options_8;
            (4w0xE) : parse_inner_ipv4_inner_options_9;
            (4w0xF) : parse_inner_ipv4_inner_options_10;
            default : accept;
        }
    }
#endif

#ifndef __SDE_9_7_SUPPORT__
    state parse_inner_ipv4_inner_options_1 {
        pkt.extract(hdr.inner_option_word_1);
        transition parse_inner_ipv4_no_inner_options;
    }

    state parse_inner_ipv4_inner_options_2 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        transition parse_inner_ipv4_no_inner_options;
    }
    
    state parse_inner_ipv4_inner_options_3 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        transition parse_inner_ipv4_no_inner_options;
    }
       
    state parse_inner_ipv4_inner_options_4 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        pkt.extract(hdr.inner_option_word_4);
        transition parse_inner_ipv4_no_inner_options;
    }
    
    state parse_inner_ipv4_inner_options_5 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        pkt.extract(hdr.inner_option_word_4);
        pkt.extract(hdr.inner_option_word_5);
        transition parse_inner_ipv4_no_inner_options;
    }

    state parse_inner_ipv4_inner_options_6 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        pkt.extract(hdr.inner_option_word_4);
        pkt.extract(hdr.inner_option_word_5);
        pkt.extract(hdr.inner_option_word_6);
        transition parse_inner_ipv4_no_inner_options;
    }
        
    state parse_inner_ipv4_inner_options_7 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        pkt.extract(hdr.inner_option_word_4);
        pkt.extract(hdr.inner_option_word_5);
        pkt.extract(hdr.inner_option_word_6);
        pkt.extract(hdr.inner_option_word_7);
        transition parse_inner_ipv4_no_inner_options;
    }

    state parse_inner_ipv4_inner_options_8 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        pkt.extract(hdr.inner_option_word_4);
        pkt.extract(hdr.inner_option_word_5);
        pkt.extract(hdr.inner_option_word_6);
        pkt.extract(hdr.inner_option_word_7);
        pkt.extract(hdr.inner_option_word_8);
        transition parse_inner_ipv4_no_inner_options;
    }
    
    state parse_inner_ipv4_inner_options_9 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        pkt.extract(hdr.inner_option_word_4);
        pkt.extract(hdr.inner_option_word_5);
        pkt.extract(hdr.inner_option_word_6);
        pkt.extract(hdr.inner_option_word_7);
        pkt.extract(hdr.inner_option_word_8);
        pkt.extract(hdr.inner_option_word_9);
        transition parse_inner_ipv4_no_inner_options;
    }
    
    state parse_inner_ipv4_inner_options_10 {
        pkt.extract(hdr.inner_option_word_1);
        pkt.extract(hdr.inner_option_word_2);
        pkt.extract(hdr.inner_option_word_3);
        pkt.extract(hdr.inner_option_word_4);
        pkt.extract(hdr.inner_option_word_5);
        pkt.extract(hdr.inner_option_word_6);
        pkt.extract(hdr.inner_option_word_7);
        pkt.extract(hdr.inner_option_word_8);
        pkt.extract(hdr.inner_option_word_9);
        pkt.extract(hdr.inner_option_word_10);
        transition parse_inner_ipv4_no_inner_options;
    }

    state parse_inner_ipv4_no_inner_options {
        transition select(hdr.inner_ipv4.protocol, hdr.inner_ipv4.fragOffset) {
            (IP_PROTOCOLS_TCP, 0) : parse_inner_tcp;
            (IP_PROTOCOLS_UDP, 0) : parse_inner_udp;
            default : accept;
        }
    }

    state parse_inner_tcp {
        pkt.extract(hdr.inner_tcp);        
        meta.l3.lkp_l4_sport = hdr.inner_tcp.srcPort;
        meta.l3.lkp_l4_dport = hdr.inner_tcp.dstPort;
        transition accept;
    }

    state parse_inner_udp {
        pkt.extract(hdr.inner_udp);        
        meta.l3.lkp_l4_sport = hdr.inner_udp.srcPort;
        meta.l3.lkp_l4_dport = hdr.inner_udp.dstPort;
        transition accept;
    }
#endif
}

//-----------------------------------------------------------------------------
// P13 Egress Deparser
//-----------------------------------------------------------------------------
control P13_EgressDeparser(
    packet_out pkt,
    inout headers_t hdr,
    in common_metadata_t meta,
    in egress_intrinsic_metadata_for_deparser_t eg_dprsr_md) {
    Mirror() mirror;

    apply {
        if (eg_dprsr_md.mirror_type == MIRROR_TYPE_E2E) {
            //镜像的出口在哪个Pipe，就哪个pipe egress解析，所以这里P13的egress也可以mirror，镜像报文就不会环回了。
            mirror.emit(meta.tunnel.session_id, meta.mirror);
        }
        pkt.emit(hdr);
    }
}

