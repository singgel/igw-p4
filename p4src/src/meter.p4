/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EipInMeter(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectMeter(MeterType_t.BYTES) rl_meter;    

    action execute_ratelimit() {
        hdr.bg_md.meter_packet_color = (bit<2>)rl_meter.execute(); 
    }

    table bw_ratelimit {
        key = {
            meta.ratelimit.bandwidth_id : exact;
        }
        actions = {
            execute_ratelimit;
        }
        size = 150000;
        meters = rl_meter;
    }

    apply {
        bw_ratelimit.apply();
    }
}

control EipInSharedMeter(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectMeter(MeterType_t.BYTES) shared_rl_meter;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) ratelimit_drop_stats;

    action execute_shared_ratelimit() {
        hdr.bg_md.meter_packet_color = (bit<2>)shared_rl_meter.execute(); 
    }

    table shared_bw_ratelimit {
        key = {
            hdr.bg_md.shared_bandwidth_id : exact;
        }
        actions = {
            execute_shared_ratelimit;
        }
        size = 2048;
        meters = shared_rl_meter;
    }

    action drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 0x1;
        ratelimit_drop_stats.count();
    }

    action nop() {
        ratelimit_drop_stats.count();
    }

    table ratelimit_drop {
        key = {
            hdr.bg_md.meter_packet_color : exact;
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
        if (hdr.bg_md.meter_packet_color != COLOR_RED) {
            shared_bw_ratelimit.apply();
        }
        ratelimit_drop.apply();
    }
}

control EipOutMeter(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectMeter(MeterType_t.BYTES) rl_meter;    

    action execute_ratelimit() {
        hdr.bg_md.meter_packet_color = (bit<2>)rl_meter.execute(); 
    }

    table bw_ratelimit {
        key = {
            meta.ratelimit.bandwidth_id : exact;
        }
        actions = {
            execute_ratelimit;
        }
        size = 100000;
        meters = rl_meter;
    }

    apply {
        bw_ratelimit.apply();
    }
}

control EipOutSharedMeter(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectMeter(MeterType_t.BYTES) shared_rl_meter;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) ratelimit_drop_stats;

    action execute_shared_ratelimit() {
        hdr.bg_md.meter_packet_color = (bit<2>)shared_rl_meter.execute(); 
    }

    table shared_bw_ratelimit {
        key = {
            hdr.bg_md.shared_bandwidth_id : exact;
        }
        actions = {
            execute_shared_ratelimit;
        }
        size = 2048;
        meters = shared_rl_meter;
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
            hdr.bg_md.meter_packet_color : exact;
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
        if (hdr.bg_md.meter_packet_color != COLOR_RED) {
            shared_bw_ratelimit.apply();
        }
        ratelimit_drop.apply();
    }
}