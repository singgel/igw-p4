/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control VxlanRoute(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    Counter<bit<32>, bit<1>>(2, CounterType_t.PACKETS) ttl_drop_stats;

    action vxlan_tunnel_loop(bit<24> tunnel_id) {
        ig_intr_md_for_dprsr.resubmit_type = RESUBMIT_WITH_DATA;
        meta.resubmit.tunnel_route_idx = tunnel_id;
        meta.tunnel.resubmit = true;
    }
    
    action vxlan_tunnel_fwd() {
        meta.tunnel.resubmit = false;
    }

    action vxlan_cen_to_br(bit<24> tunnel_id) {
        vxlan_tunnel_loop(tunnel_id);
    }

    action vxlan_to_nexthop(bit<16> nexthop, bit<3> egress_pipe) {
        meta.l3.egr_pipeline = egress_pipe;
        hdr.bg_md.tunnel_nexthop = nexthop;
        vxlan_tunnel_fwd();
    }

    action vxlan_to_nexthop_overlay(bit<16> nexthop) {
        hdr.bg_md.tunnel_nexthop = nexthop;
        hdr.bg_md.dl_pkt = 1;
        vxlan_tunnel_fwd();
    }

    action vxlan_to_loacl(bit<3> egress_pipe) {
        meta.l3.egr_pipeline = egress_pipe;
        hdr.bg_md.tunnel_nexthop = 0;
        vxlan_tunnel_fwd();
    }
    
    action vxlan_to_loacl_overlay() {
        hdr.bg_md.tunnel_nexthop = 0;
        vxlan_tunnel_fwd();
    }

    #ifdef BGW_USE_ALPM
    @pragma alpm 1
    @pragma alpm_partitions 2048
    @pragma alpm_subtrees_per_partition 2
    #endif
    table vxlan_route {
        key = {
            meta.tunnel.route_idx : exact;
            hdr.inner_ipv4.dstAddr : lpm;
        }

        actions = {
            vxlan_to_nexthop;
            vxlan_to_nexthop_overlay;
            vxlan_to_loacl;
            vxlan_to_loacl_overlay;
        }
        size = VXLAN_RT_TABLE_SIZE;
    }

    apply {
        if (hdr.inner_ipv4.isValid()) {
            if (hdr.inner_ipv4.ttl > 1) {
                vxlan_route.apply();
            } else {
                ig_intr_md_for_dprsr.drop_ctl = 0x1;
                ttl_drop_stats.count(1);
            }
        } 

        if (meta.tunnel.resubmit) {
            if (ig_intr_md.resubmit_flag != 0) {
                ig_intr_md_for_dprsr.drop_ctl = 0x5;
            }
        } else {
            hdr.bg_md.lkp_vni = (bit<24>) meta.tunnel.route_idx;
        }
    }
}

control IgwIpType(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    action ip_from_internet_in_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_IN;
        hdr.bg_md.egr_tunnel_type = EGRESS_TUNNEL_TYPE_VXLAN;
        meta.l3.egr_pipeline = EGR_PIPELINE;
        hdr.bg_md.dl_pkt = 0;
    }    

    action ip_from_internet_in_dl_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_IN;
        hdr.bg_md.egr_tunnel_type = EGRESS_TUNNEL_TYPE_VXLAN;
        meta.l3.egr_pipeline = EGR_PIPELINE;
        hdr.bg_md.dl_pkt = 1;
    }    

    action ip_from_internet_out_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_OUT;
        hdr.bg_md.egr_tunnel_type = EGRESS_TUNNEL_TYPE_VXLAN;
        meta.l3.egr_pipeline = EGR_PIPELINE_TWO;
        hdr.bg_md.dl_pkt = 0;
    }

    action ip_from_internet_out_dl_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_OUT;
        hdr.bg_md.egr_tunnel_type = EGRESS_TUNNEL_TYPE_VXLAN;
        meta.l3.egr_pipeline = EGR_PIPELINE_TWO;
        hdr.bg_md.dl_pkt = 1;
    }

    action need_drop() {
        ig_intr_md_for_dprsr.drop_ctl = 0x1;
    }

    table igw_ip_type {
        key = {
            hdr.ipv4.isValid()          : ternary;
            hdr.ipv4.dstAddr            : ternary;
            hdr.vxlan.isValid()         : ternary;
            hdr.inner_ipv4.isValid()    : ternary;
            meta.tunnel.vxlan_type      : ternary;
            hdr.vxlan.tof               : ternary;
        }

        actions = {
            ip_from_internet_in_hit;
            ip_from_internet_in_dl_hit;
            ip_from_internet_out_hit;
            ip_from_internet_out_dl_hit;
            need_drop;
        }
        size = 32;
        const default_action = ip_from_internet_in_hit();
    }

    apply {
        igw_ip_type.apply();
    }
}

control RewriteOuterMac(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action rewrite_tunnel_mac(bit<48> smac, bit<48> dmac) {
        hdr.ethernet.srcAddr = smac;
        hdr.ethernet.dstAddr = dmac;
    }  
    
    table tunnel_mac_rewrite {
        key = {
            eg_intr_md.egress_port : exact;
        }

        actions = {
            rewrite_tunnel_mac;
        }
        size = TUNNEL_MAC_SIZE;
    }

    apply {
        tunnel_mac_rewrite.apply();
    }       
}