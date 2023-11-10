/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EipOutEgressPktStats(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats6;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) drop_stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) drop_stats6;

    action count() { stats.count(EIP46_OUT_EGRESS_ADJUST); }
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

    action drop_count() { drop_stats.count(); }
    table eip_out_drop_pkt_stats {
        key = {
            hdr.inner_ipv4.srcAddr : exact;
        }

        actions = {
            drop_count;
        }

        size = EIP_SIZE;
        counters = drop_stats;
    }

    action count6() { stats6.count(EIP46_OUT_EGRESS_ADJUST); }
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

    action drop_count6() { drop_stats6.count(); }
    table eip6_out_drop_pkt_stats {
        key = {
            hdr.inner_ipv6.srcAddr : exact;
        }

        actions = {
            drop_count6;
        }

        size = EIP6_SIZE;
        counters = drop_stats6;
    }

    apply {
        if (hdr.inner_ipv4.isValid() && (hdr.bg_md.meter_packet_color != COLOR_RED)) {
	        eip_out_egress_pkt_stats.apply();
	    } else if(hdr.inner_ipv4.isValid() && (hdr.bg_md.meter_packet_color == COLOR_RED)) {
	        eip_out_drop_pkt_stats.apply();
	    } else if (hdr.inner_ipv6.isValid()&& (hdr.bg_md.meter_packet_color != COLOR_RED)) {
	        eip6_out_egress_pkt_stats.apply();
	    } else if (hdr.inner_ipv6.isValid()&& (hdr.bg_md.meter_packet_color == COLOR_RED)) {
	        eip6_out_drop_pkt_stats.apply();
	    }
    }
}

control EipInEgressPktStats(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS_AND_BYTES) stats6;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) drop_stats;
    DirectCounter<bit<32>>(CounterType_t.PACKETS) drop_stats6;

    action count() { stats.count(EIP46_IN_EGRESS_ADJUST); }
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

    action drop_count() { drop_stats.count(); }
    table eip_in_drop_pkt_stats {
        key = {
            hdr.bg_md.eip_or_bwid : exact;
        }

        actions = {
            drop_count;
        }

        size = EIP_SIZE;
        counters = drop_stats;
    }

    action count6() { stats6.count(EIP46_IN_EGRESS_ADJUST); }
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

    action drop_count6() { drop_stats6.count(); }
    table eip6_in_drop_pkt_stats {
        key = {
            hdr.inner_ipv6.dstAddr : exact;
        }

        actions = {
            drop_count6;
        }

        size = EIP6_SIZE;
        counters = drop_stats6;
    }

    apply {
        if (hdr.inner_ipv4.isValid() && (hdr.bg_md.need_drop != 1) && (hdr.bg_md.meter_packet_color != COLOR_RED)) {
	        eip_in_egress_pkt_stats.apply();
	    } else if(hdr.inner_ipv4.isValid() &&(hdr.bg_md.need_drop != 1) && (hdr.bg_md.meter_packet_color == COLOR_RED)) {
	        eip_in_drop_pkt_stats.apply();
	    } else if (hdr.inner_ipv6.isValid() &&(hdr.bg_md.need_drop != 1) && (hdr.bg_md.meter_packet_color != COLOR_RED)) {
	        eip6_in_egress_pkt_stats.apply();
	    } else if (hdr.inner_ipv6.isValid() &&(hdr.bg_md.need_drop != 1) && (hdr.bg_md.meter_packet_color == COLOR_RED)) {
	        eip6_in_drop_pkt_stats.apply();
	    }
    }
}