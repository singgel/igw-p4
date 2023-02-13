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

control IngressVifStats(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;

    action count() { stats.count(54); }
    table ingress_vif_stats  {
        key = {
            hdr.vxlan.vni    : exact;
            hdr.ipv4.srcAddr : exact;
        }

        actions = {
            count;
        }

        size = VIF_STATS_SIZE;
        counters = stats;
    }

    apply {
        if (hdr.vxlan.isValid() && hdr.inner_ipv4.isValid()) {
            ingress_vif_stats.apply();
        }
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

control EgressVifStats(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;

    action count() { stats.count(69); }

    action dscp_replace() {
        stats.count();
        hdr.ipv4.dscp = hdr.inner_ipv4.dscp;
    }
    action dscp_set(bit<6> dscp_val) {
        stats.count();
        hdr.ipv4.dscp = dscp_val;
    }

    table egress_vif_stats  {
        key = {
            hdr.bg_md.lkp_vni  : exact;
            hdr.ipv4.dstAddr : exact;
        }

        actions = {
            count;
            dscp_replace;
            dscp_set;
        }

        size = VIF_STATS_SIZE;
        counters = stats;
    }

    apply {
        if (hdr.vxlan.isValid() && hdr.inner_ipv4.isValid()) {
            egress_vif_stats.apply();
        }
    }
}