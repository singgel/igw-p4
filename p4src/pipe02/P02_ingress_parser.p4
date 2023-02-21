/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

//=============================================================================
// P02 Ingress parser
//=============================================================================
parser P02_IngressParser(
        packet_in pkt,
        out headers_t hdr,
        out common_metadata_t meta,
        out ingress_intrinsic_metadata_t ig_intr_md) {

    state start {
        pkt.extract(ig_intr_md);
        transition select(ig_intr_md.resubmit_flag) {
            1 : parse_resubmit;
            0 : parse_port_metadata;
        }
    }

    //@pa_solitary do not share phv, install/bin/bf-p4c --help-pragmas 
    state parse_resubmit {
        pkt.extract(meta.resubmit);
        transition parse_ethernet;  
    }

    state parse_port_metadata {
        pkt.advance(PORT_METADATA_SIZE); //tna: 64
        transition parse_ethernet;
    }

    state parse_ethernet {
        pkt.extract(hdr.ethernet);
        transition select(hdr.ethernet.etherType) {
            ETHERTYPE_IPV4 : parse_ipv4;
            ETHERTYPE_BFN : parse_cpu;
            default : accept;
        }
    }
    
    state parse_cpu {
        pkt.extract(hdr.fabric);
        pkt.extract(hdr.cpu);
        transition select(hdr.cpu.ether_type) {
            ETHERTYPE_IPV4 : parse_ipv4;
            default : accept;
        }
    }

    state parse_ipv4 {
        pkt.extract(hdr.ipv4);
        transition select(hdr.ipv4.ihl) {
            (4w0x5) : parse_ipv4_no_options;
            default : accept;
        }
    }

    state parse_ipv4_no_options {
        transition select(hdr.ipv4.protocol, hdr.ipv4.ihl, hdr.ipv4.fragOffset) {
            (IP_PROTOCOLS_ICMP, 5, 0) : parse_icmp;
            (IP_PROTOCOLS_TCP, 5, 0) : parse_tcp;
            (IP_PROTOCOLS_UDP, 5, 0) : parse_udp;
            // Do NOT parse the next header if IP packet is fragmented.
            default : accept;
        }
    }

    state parse_udp {
        pkt.extract(hdr.udp);
        meta.l3.lkp_outer_l4_sport = hdr.udp.srcPort;
        meta.l3.lkp_outer_l4_dport = hdr.udp.dstPort;
        transition select(hdr.udp.dstPort) {
            UDP_PORT_VXLAN : parse_std_vxlan;  //4789
            default : accept;
        }
    }

    state parse_tcp {
        pkt.extract(hdr.tcp);        
        meta.l3.lkp_outer_l4_sport = hdr.tcp.srcPort;
        meta.l3.lkp_outer_l4_dport = hdr.tcp.dstPort;
        transition accept;
    }

    state parse_icmp {
        pkt.extract(hdr.icmp);
        meta.l3.lkp_outer_l4_sport = 0;
        meta.l3.lkp_outer_l4_dport = 0;
        transition accept;
    }
    
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
}

//-----------------------------------------------------------------------------
// P02 Ingress Deparser
//-----------------------------------------------------------------------------
control P02_IngressDeparser(
    packet_out pkt,
    inout headers_t hdr,
    in common_metadata_t meta,
    in ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr) {
    Mirror() mirror;
    Resubmit() resubmit;

    apply {
        if (ig_intr_md_for_dprsr.resubmit_type == RESUBMIT_WITH_DATA) {
            resubmit.emit(meta.resubmit);
        }
        
        if (ig_intr_md_for_dprsr.mirror_type == MIRROR_TYPE_I2E) {
            mirror.emit(meta.tunnel.session_id, meta.mirror);
        }
        pkt.emit(hdr);
    }
}

