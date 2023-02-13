/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _P4_PLATFORM_
#define _P4_PLATFORM_

control ProcessLocal(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    DirectCounter<bit<64>>(CounterType_t.PACKETS_AND_BYTES) process_protocol_packet_stats;
    
    action add_nos_cpu_header() {
        hdr.fabric.setValid();
        hdr.fabric.reserved = 0;
        hdr.fabric.color = 0;
        hdr.fabric.qos = 0;
        hdr.fabric.reserved2 = 0;
        hdr.fabric.dst_port_or_group = 0;

        hdr.cpu.setValid();
        hdr.cpu.egress_queue = 0;
        hdr.cpu.tx_bypass = 0;
        hdr.cpu.reserved = 0;
        hdr.cpu.ingress_port = (bit<16>)ig_intr_md.ingress_port;
        hdr.cpu.ether_type = hdr.ethernet.etherType;

        hdr.ethernet.etherType = ETHERTYPE_BFN;
    }

    action copy_to_cpu_nos(bit<9> egress_port) {
        process_protocol_packet_stats.count();
        ig_tm_md.ucast_egress_port =  egress_port;
        add_nos_cpu_header();
        hdr.bg_md.tunnel_direct_send = SKIP_PACKET;
    }

    action receive_from_cpu_nos() {
        process_protocol_packet_stats.count();
        hdr.ethernet.etherType = hdr.cpu.ether_type;
        ig_tm_md.ucast_egress_port =  (bit<9>)hdr.fabric.dst_port_or_group;
        hdr.fabric.setInvalid();
        hdr.cpu.setInvalid();
        hdr.bg_md.tunnel_direct_send = SKIP_PACKET;
    }

    action receive_from_cpu_lldp() {
        process_protocol_packet_stats.count();
        hdr.ethernet.etherType = hdr.cpu.ether_type;
        ig_tm_md.ucast_egress_port =  (bit<9>)hdr.fabric.dst_port_or_group;
        hdr.fabric.setInvalid();
        hdr.cpu.setInvalid();
        hdr.bg_md.tunnel_direct_send = SKIP_TUNNEL_MAC_REWRITE;
    }

    table process_protocol_packet {
        key = {
            hdr.ipv4.isValid() : ternary;
            hdr.vxlan.isValid()   : ternary;
            hdr.ethernet.etherType : ternary;
            hdr.ipv4.dstAddr : ternary;
            ig_intr_md.ingress_port : ternary;
        }

        actions = {
            receive_from_cpu_lldp;
            copy_to_cpu_nos;
            receive_from_cpu_nos;
        }

        size = 64;
        counters = process_protocol_packet_stats;
    }

    apply {
        process_protocol_packet.apply();
    }
}

#endif /* _P4_PLATFORM_ */
