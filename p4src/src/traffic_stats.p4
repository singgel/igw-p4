/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EipInMeterDropStats(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS) meter_drop_stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) ipv6_meter_drop_stats;

    action drop_stats() {
        meter_drop_stats.count();
    }

    table meter_drop_show {
        key = {
            hdr.inner_ipv4.dstAddr  : exact;
        }

        actions = {
            drop_stats;
        }
        size = 100000;
        counters = meter_drop_stats;
    }

    action ipv6_drop_stats() {
        ipv6_meter_drop_stats.count();
    }

    table ipv6_meter_drop_show {
        key = {
            hdr.inner_ipv6.dstAddr  : exact;
        }

        actions = {
            ipv6_drop_stats;
        }
        size = 2000;
        counters = ipv6_meter_drop_stats;
    }

    apply {
        if (hdr.inner_ipv4.isValid() && (hdr.bg_md.meter_packet_color == COLOR_RED)) {
            meter_drop_show.apply();
        } else if (hdr.inner_ipv6.isValid() && (hdr.bg_md.meter_packet_color == COLOR_RED)){
            ipv6_meter_drop_show.apply();
        }
    }
}

control EipOutMeterDropStats(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS) meter_drop_stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) ipv6_meter_drop_stats;

    action drop_stats() {
        meter_drop_stats.count();
    }

    table meter_drop_show {
        key = {
            hdr.inner_ipv4.srcAddr  : exact;
        }

        actions = {
            drop_stats;
        }
        size = 100000;
        counters = meter_drop_stats;
    }

    action ipv6_drop_stats() {
        ipv6_meter_drop_stats.count();
    }

    table ipv6_meter_drop_show {
        key = {
            hdr.inner_ipv6.srcAddr  : exact;
        }

        actions = {
            ipv6_drop_stats;
        }
        size = 2000;
        counters = ipv6_meter_drop_stats;
    }
    
    apply {
        if (hdr.inner_ipv4.isValid() && (hdr.bg_md.meter_packet_color == COLOR_RED)) {
            meter_drop_show.apply();
        } else if (hdr.inner_ipv6.isValid() && (hdr.bg_md.meter_packet_color == COLOR_RED)) {
            ipv6_meter_drop_show.apply();
        }
    }
}

control EipInIngressPktStats(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats6;

    action count() { stats.count(BR_ADJUST_BYTES); }
    table eip_in_ingress_pkt_stats  {
        key = {
            hdr.inner_ipv4.dstAddr : exact;
        }

        actions = {
            count;
        }

        size = EIP_SIZE;
        counters = stats;
    }

    action count6() { stats6.count(BR_ADJUST_BYTES); }
    table eip6_in_ingress_pkt_stats  {
        key = {
            hdr.inner_ipv6.dstAddr : exact;
        }

        actions = {
            count6;
        }

        size = EIP6_SIZE;
        counters = stats6;
    }

    apply {
        if (hdr.inner_ipv4.isValid()) {
            eip_in_ingress_pkt_stats.apply();
        } else if (hdr.inner_ipv6.isValid()) {
            eip6_in_ingress_pkt_stats.apply();
        }
    }
}

control EipInEgressPktStats(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats6;

    action count() { stats.count(BR_ADJUST_BYTES); }
    table eip_in_egress_pkt_stats  {
        key = {
            hdr.bg_md.eip_or_bwid    : exact;
        }

        actions = {
            count;
        }

        size = EIP_SIZE;
        counters = stats;
    }

    action count6() { stats6.count(BR_ADJUST_BYTES); }
    table eip6_in_egress_pkt_stats  {
        key = {
            hdr.inner_ipv6.dstAddr    : exact;
        }

        actions = {
            count6;
        }

        size = EIP6_SIZE;
        counters = stats6;
    }

    apply {
        if (hdr.inner_ipv4.isValid() &&(hdr.bg_md.need_drop != 1)) {
            eip_in_egress_pkt_stats.apply();
        } else if (hdr.inner_ipv6.isValid()&&(hdr.bg_md.need_drop != 1)) {
            eip6_in_egress_pkt_stats.apply();
        }
    }
}

control EipOutIngressPktStats(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats6;

    action count() { stats.count(BR_ADJUST_BYTES); }
    table eip_out_ingress_pkt_stats  {
        key = {
            hdr.inner_ipv4.srcAddr : exact;
        }

        actions = {
            count;
        }

        size = EIP_SIZE;
        counters = stats;
    }

    action count6() { stats6.count(BR_ADJUST_BYTES); }
    table eip6_out_ingress_pkt_stats  {
        key = {
            hdr.inner_ipv6.srcAddr : exact;
        }

        actions = {
            count6;
        }

        size = EIP6_SIZE;
        counters = stats6;
    }

    apply {
        if (hdr.inner_ipv4.isValid()) {
            eip_out_ingress_pkt_stats.apply();
        } else if (hdr.inner_ipv6.isValid()) {
            eip6_out_ingress_pkt_stats.apply();
        }
    }
}

control EipOutEgressPktStats(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats6;

    action count() { stats.count(BR_ADJUST_BYTES); }
    table eip_out_egress_pkt_stats  {
        key = {
            hdr.inner_ipv4.srcAddr : exact;
        }

        actions = {
            count;
        }

        size = EIP_SIZE;
        counters = stats;
    }

    action count6() { stats6.count(BR_ADJUST_BYTES); }
    table eip6_out_egress_pkt_stats  {
        key = {
            hdr.inner_ipv6.srcAddr : exact;
        }

        actions = {
            count6;
        }

        size = EIP6_SIZE;
        counters = stats6;
    }

    apply {
        if (hdr.inner_ipv4.isValid() &&(hdr.bg_md.meter_packet_color != COLOR_RED)) {
            eip_out_egress_pkt_stats.apply();
        } else if (hdr.inner_ipv6.isValid() &&(hdr.bg_md.meter_packet_color != COLOR_RED)) {
            eip6_out_egress_pkt_stats.apply();
        }
    }
}