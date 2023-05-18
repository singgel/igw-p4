/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control IngressPortStats(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;

    action count() { stats.count(CRC_ADJUST_BYTES); }
    table ingress_port_stats  {
        key = {
            hdr.vxlan.isValid()     : exact;
            ig_intr_md.ingress_port : exact;
        }

        actions = {
            count;
        }

        size = 64;
        counters = stats;
    }

    apply {
        ingress_port_stats.apply();
    }
}

control EgressPortStats(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;

    action count() { stats.count(BR_ADJUST_BYTES); }
    table egress_port_stats  {
        key = {
            hdr.vxlan.isValid()     : exact;
            eg_intr_md.egress_port  : exact;
        }

        actions = {
            count;
        }

        size = 64;
        counters = stats;
    }

    apply {
        if (hdr.bg_md.need_drop != 1) {
            egress_port_stats.apply();
        }
    }
}

control EipInMeterDropStats(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS) meter_drop_stats;

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
        size = 150000;
        counters = meter_drop_stats;
    }

    apply {
        if (hdr.inner_ipv4.isValid() && (hdr.bg_md.meter_packet_color == COLOR_RED)) {
            meter_drop_show.apply();
        }
    }
}
