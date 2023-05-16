/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EipInRedirect(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action nop() {}

    action set_bw_id(bit<18> bandwidth_id, bit<11> shared_bandwidth_id,
            bit<1> within_cluster, bit<1> between_cluster) {
        meta.ratelimit.bandwidth_id = bandwidth_id;
        meta.ratelimit.shared_bandwidth_id = shared_bandwidth_id;
        meta.ratelimit.within_cluster = within_cluster;
        meta.ratelimit.between_cluster = between_cluster;
    }

    table eip_in_redirect {
        key = {
            hdr.inner_ipv4.dstAddr  : exact;
        }

        actions = {
            set_bw_id;
            nop;
        }

        size = 200000;
        const default_action = nop();
    }
    
    action nop2() {}

    action rewrite_az_in_jd_vxlan(bit<32> vip) {
        hdr.vxlan.flags = 0x0c;
        hdr.vxlan.version = 1;
        hdr.vxlan.vni = 99;
        hdr.vxlan.tof = TOF_AZ_IN;
        hdr.udp.srcPort = 50;
        hdr.ipv4.dstAddr = vip; 
        hdr.bg_md.tunnel_direct_send = DL_PACKET;
    }

     action rewrite_eip_in_jd_vxlan(bit<32> dstip) {
        hdr.vxlan.flags = 0x0c;
        hdr.vxlan.version = 1;
        hdr.vxlan.vni = 125;
        hdr.vxlan.tof = TOF_EIP_IN;
        hdr.udp.srcPort = 250;
        hdr.ipv4.dstAddr = dstip; 
        hdr.bg_md.tunnel_direct_send = DL_PACKET;
    }

    table modify_jd_vxlan {
        key = {
            hdr.vxlan.isValid() : ternary;
            hdr.vxlan.tof: ternary;
            meta.ratelimit.between_cluster : ternary;
            meta.ratelimit.within_cluster : ternary;
        }
        size = 6;
        actions = {
            rewrite_eip_in_jd_vxlan;
            rewrite_az_in_jd_vxlan;
            nop2;
        }
    }

    apply {
        eip_in_redirect.apply();
        modify_jd_vxlan.apply();
    }
}

control EipInRatelimit(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectMeter(MeterType_t.BYTES) shared_rl_meter;
    DirectMeter(MeterType_t.BYTES) rl_meter;    
    DirectCounter<bit<32>>(CounterType_t.PACKETS) ratelimit_drop_stats;

    action execute_shared_ratelimit() {
        meta.ratelimit.shared_color = (bit<2>)shared_rl_meter.execute(); 
    }

    table shared_bw_ratelimit {
        key = {
            meta.ratelimit.shared_bandwidth_id : exact;
        }
        actions = {
            execute_shared_ratelimit;
        }
        size = 2048;
        meters = shared_rl_meter;
    }

    action execute_ratelimit() {
        meta.ratelimit.color = (bit<2>)rl_meter.execute(); 
    }

    table bw_ratelimit {
        key = {
            meta.ratelimit.bandwidth_id : exact;
        }
        actions = {
            execute_ratelimit;
        }
        size = 200000;
        meters = rl_meter;
    }

    action drop_packet() {
        eg_dprsr_md.drop_ctl = 0x1;
        ratelimit_drop_stats.count();
    }

    action nop() {
        ratelimit_drop_stats.count();
    }

    table ratelimit_drop {
        key = {
            meta.ratelimit.shared_color : exact;
            meta.ratelimit.color : exact;
        }

        actions = {
            drop_packet;
            nop;
        }
        size = 4;
        const default_action = nop();
        counters = ratelimit_drop_stats;
    }

    apply {
        shared_bw_ratelimit.apply();
        bw_ratelimit.apply();
        ratelimit_drop.apply();
    }
}