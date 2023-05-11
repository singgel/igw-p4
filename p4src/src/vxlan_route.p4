/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control IngressRoute(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action vxlan_to_nexthop_overlay(bit<16> nexthop) {
        hdr.bg_md.tunnel_nexthop = nexthop;
    }

    table vxlan_route {
        key = {
            meta.tunnel.route_idx : exact;
            hdr.inner_ipv4.dstAddr : lpm;
        }

        actions = {
            vxlan_to_nexthop_overlay;
        }
        size = 1024;
    }

    apply {
        vxlan_route.apply();
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
        meta.l3.egr_pipeline = EGR_PIPELINE_13;
        hdr.bg_md.dl_pkt = 0;
    }    

    action ip_from_internet_in_dl_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_IN;
        meta.l3.egr_pipeline = EGR_PIPELINE_13;
        hdr.bg_md.dl_pkt = 1;
    }    

    action ip_from_internet_out_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_OUT;
        meta.l3.egr_pipeline = EGR_PIPELINE_02;
        hdr.bg_md.dl_pkt = 0;
    }

    action ip_from_internet_out_dl_hit() {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_OUT;
        meta.l3.egr_pipeline = EGR_PIPELINE_02;
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