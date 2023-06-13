/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control ProcessMirror(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectMeter(MeterType_t.PACKETS) mirror_meter;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) mirror_stats;

    action add_cpu_header() {
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
        hdr.cpu.ingress_port = MIRROR_DEV_PORT;
        hdr.cpu.ether_type = hdr.ethernet.etherType;

        hdr.ethernet.etherType = ETHERTYPE_BFN;
    }
    
    action clone_to_cpu() {
        hdr.ethernet.dstAddr[47:40] = meta.mirror.res;
        hdr.ethernet.dstAddr[32:24] = meta.mirror.port;
        hdr.ethernet.srcAddr[47:00] = meta.mirror.timestamp;
        add_cpu_header();
        mirror_stats.count();
        hdr.bg_md.meter_packet_color = (bit<2>) mirror_meter.execute();       
    }

    table mirror {
        key = {
            meta.mirror.flag : exact;
        }

        actions = {
            clone_to_cpu;
        }

        size = 2;
        counters = mirror_stats;
        meters = mirror_meter;
    }

    action drop_packet() {
        eg_dprsr_md.drop_ctl = 0x1;
    }

    action nop() {}

    table mirror_drop {
        key = {
            hdr.bg_md.meter_packet_color : exact;
        }

        actions = {
            drop_packet;
            nop;
        }
        size = 2;
    }

    apply {
        mirror.apply();
        mirror_drop.apply();
    }
}