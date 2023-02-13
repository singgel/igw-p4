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
        hdr.bg_md.pkt_dir = 1;
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

control Vlinklogic(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    /*update vif vni to bgw vni*/
    action update_tunnel_vni (bit<24> tunnel_id) {
        meta.tunnel.route_idx = (bit<32>)tunnel_id;
    }

    action nop() {}

    table vlink_table {
        key = {
            hdr.bg_md.lkp_vni : exact;
        }

        actions = {
            update_tunnel_vni;
            nop;
        }
        size = VLINK_TABLE_SIZE;
    }

    apply {
        if (hdr.bg_md.igr_tunnel_type == TYPE_INGRESS_TUNNEL_VBR) {
            vlink_table.apply();
        }
    }
}

control BgwIpType(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    /*
    action ip_from_cen_hit(bit<3> egress_id, bit<8> src_idx) {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_TUNNEL_CEN;
        hdr.bg_md.egr_tunnel_type = EGRESS_TUNNEL_TYPE_VXLAN;
        meta.l3.egr_pipeline = egress_id;
        meta.tunnel.route_idx = (bit<32>) hdr.bg_md.lkp_vni;
        hdr.bg_md.tunnel_src_id = src_idx;
        bgw_ip_type_stats.count();
    }

    action ip_from_vpc_hit(bit<3> egress_id) {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_TUNNEL_VPC;
        hdr.bg_md.egr_tunnel_type = EGRESS_TUNNEL_TYPE_VXLAN;
        meta.l3.egr_pipeline = egress_id;
        meta.tunnel.route_idx = (bit<32>) hdr.bg_md.lkp_vni;
        bgw_ip_type_stats.count();
    }    
    */

    action ip_from_vbr_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_TUNNEL_VBR;
        hdr.bg_md.egr_tunnel_type = EGRESS_TUNNEL_TYPE_VXLAN;
        meta.l3.egr_pipeline = EGR_PIPELINE;
        meta.tunnel.route_idx = (bit<32>) hdr.bg_md.lkp_vni;
    }

    action nop() {}

    table bgw_ip_type {
        key = {
            hdr.vxlan.isValid()         : exact;
        }

        actions = {
            ip_from_vbr_hit;
            nop;
        }
        
        size = 2;
        const entries = {
            {true}   : ip_from_vbr_hit;
            {false}   : nop;
        }
    }

    apply {
        bgw_ip_type.apply();
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

control RewriteInnerMac(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action rewrite_inner_mac(bit<48> smac, bit<48> dmac) {
        hdr.inner_ethernet.srcAddr = smac;
        hdr.inner_ethernet.dstAddr = dmac;
    }  
    
    table tunnel_inner_rewrite {
        key = {
            hdr.bg_md.inner_mac_id : exact;
        }

        actions = {
            rewrite_inner_mac;
        }
        size = TUNNEL_INNER_MAC_SIZE;
    }
    
    apply {
        tunnel_inner_rewrite.apply();
    }       
}