/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/
control IngressRoute(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
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
        size = 8192;
    }

    apply {
        if (hdr.inner_ipv4.isValid()) {
            vxlan_route.apply();
        }
    } 
}

control PipeLineFix(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    action set_pipeline(bit<3> egr_pipeline) {
        meta.l3.egr_pipeline = egr_pipeline;
    }   

    table pipeline_fix {
        key = {
            hdr.inner_ipv4.isValid()    : ternary;
            hdr.inner_ipv4.srcAddr      : ternary;
        }

        actions = {
            set_pipeline;
        }
        size = 4;
    }

    apply {
        pipeline_fix.apply();
    }
}

control IgwIpType(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    action ip_from_internet_in_hit(bit<3> egr_pipeline) {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_IN;
        meta.l3.egr_pipeline = egr_pipeline;
        hdr.bg_md.dl_pkt = 0;
    }   

    action ip_from_internet_in_dl_hit(bit<3> egr_pipeline) {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_IN;
        meta.l3.egr_pipeline = egr_pipeline;
        hdr.bg_md.dl_pkt = 1;
    }    

    action ip_from_internet_out_hit(bit<3> egr_pipeline) {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_OUT;
        meta.l3.egr_pipeline = egr_pipeline;
        hdr.bg_md.dl_pkt = 0;
    }

    action ip_from_internet_out_dl_hit(bit<3> egr_pipeline) {
        hdr.bg_md.igr_tunnel_type = TYPE_INGRESS_INTERNET_OUT;
        meta.l3.egr_pipeline = egr_pipeline;
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
            hdr.inner_ipv4.dstAddr      : ternary;
            hdr.inner_ipv6.isValid()    : ternary;
            hdr.inner_ipv6.dstAddr[1:0] : ternary @name("inner_ipv6_dstaddr");
            hdr.inner_ipv6.srcAddr[1:0] : ternary @name("inner_ipv6_srcaddr");
            meta.tunnel.vxlan_type      : ternary;
            hdr.vxlan.tof               : ternary;
            hdr.ipv6.isValid()          : ternary;
            hdr.ipv6.dstAddr            : ternary;
        }

        actions = {
            ip_from_internet_in_hit;
            ip_from_internet_in_dl_hit;
            ip_from_internet_out_hit;
            ip_from_internet_out_dl_hit;
            need_drop;
        }
        size = 64;
        const default_action = need_drop();
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