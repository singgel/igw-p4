/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control DecapMetaData_02(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    action ig_init_bridge() {
        hdr.bg_md.tunnel_nexthop = 0;
        hdr.bg_md.tunnel_direct_send = MATCH_PACKET;
        hdr.bg_md.tunnel_src_id = 0;
        hdr.bg_md.tunnel_dst_id = 0;
        hdr.bg_md.inner_mac_id = 0;
        hdr.bg_md.need_drop = 0;
        hdr.bg_md.outer_ethernet_invalid = 0;
    }

    action ig_decap_md02_v4() {
        hdr.bg_md.setValid();
        hdr.bg_md.lkp_vni = hdr.vxlan.vni;
    #ifdef __SDE_9_7_SUPPORT__
        hdr.bg_md.lkp_l4_sport =  meta.l3.lkp_l4_sport;
        hdr.bg_md.lkp_l4_dport =  meta.l3.lkp_l4_dport;
    #endif
        meta.l3.lkp_sip = (bit<128>)hdr.inner_ipv4.srcAddr;
        meta.l3.lkp_dip = (bit<128>)hdr.inner_ipv4.dstAddr;
        ig_init_bridge();
    }   

    action ig_decap_md02_v6() {
        hdr.bg_md.setValid();
        hdr.bg_md.lkp_vni = hdr.vxlan.vni;
    #ifdef __SDE_9_7_SUPPORT__
        hdr.bg_md.lkp_l4_sport =  meta.l3.lkp_l4_sport;
        hdr.bg_md.lkp_l4_dport =  meta.l3.lkp_l4_dport;
    #endif
        meta.l3.lkp_sip = hdr.inner_ipv6.srcAddr;
        meta.l3.lkp_dip = hdr.inner_ipv6.dstAddr;
        ig_init_bridge();
    }   

    action ig_decap_md02_nop() {
        hdr.bg_md.setValid();
        hdr.bg_md.lkp_vni = 0;     
        ig_init_bridge();
    }   

    table ig_decap_md02 {
        key = {
            hdr.inner_ipv4.isValid() : exact;           
            hdr.inner_ipv6.isValid() : exact;
        }

        actions = {
            ig_decap_md02_v4;
            ig_decap_md02_v6;
            ig_decap_md02_nop;
        }

        size = 4;
        const entries = {
            {true,false}   : ig_decap_md02_v4;
            {false,true}   : ig_decap_md02_v6;
            {false,false}   : ig_decap_md02_nop;
        }
    }

    apply {
        ig_decap_md02.apply();
    }
}

control EncapMetaData_02(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    apply {
        hdr.bg_md.igr_port = ig_intr_md.ingress_port;
        hdr.bg_md.proto = PKT_BRIDGE;
        hdr.bg_md.outer_ethernet_type = hdr.ethernet.etherType;
        if ((hdr.bg_md.tunnel_direct_send == MATCH_PACKET) || 
            (hdr.bg_md.tunnel_direct_send == DL_PACKET)) {
            hdr.bg_md.outer_ethernet_invalid = 1w1;
            hdr.ethernet.setInvalid();
        }
    }              
}

control DecapMetaData_I2E13(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action eg_decap_md13_v4() {
        meta.l3.lkp_sip = (bit<128>)hdr.inner_ipv4.srcAddr;
        meta.l3.lkp_dip = (bit<128>)hdr.inner_ipv4.dstAddr;
    #ifdef __SDE_9_7_SUPPORT__
        meta.l3.lkp_l4_sport = hdr.bg_md.lkp_l4_sport;
        meta.l3.lkp_l4_dport = hdr.bg_md.lkp_l4_dport;
    #endif
        hdr.bg_md.eip_or_bwid = hdr.inner_ipv4.dstAddr;
    }  

    action eg_decap_md13_v4_no_dl() {
        meta.l3.lkp_sip = (bit<128>)hdr.ipv4.srcAddr;
        meta.l3.lkp_dip = (bit<128>)hdr.ipv4.dstAddr;
        meta.l3.lkp_ip_proto = meta.l3.lkp_outer_ip_proto;
        meta.l3.lkp_l4_sport = meta.l3.lkp_outer_l4_sport;
        meta.l3.lkp_l4_dport =  meta.l3.lkp_outer_l4_dport;
        hdr.bg_md.eip_or_bwid = hdr.ipv4.dstAddr;
    }  

    action eg_decap_md13_v6() {
        meta.l3.lkp_sip = hdr.ipv6.srcAddr;
        meta.l3.lkp_dip = hdr.ipv6.dstAddr;
        meta.l3.lkp_ip_proto = meta.l3.lkp_outer_ip_proto;
        meta.l3.lkp_l4_sport = meta.l3.lkp_outer_l4_sport;
        meta.l3.lkp_l4_dport =  meta.l3.lkp_outer_l4_dport;
    }  

    action nop() {}
    
    table eg_decap_md13 {
        key = {
            hdr.vxlan.isValid() : exact;
            hdr.inner_ipv4.isValid() : exact;
            hdr.ipv6.isValid() : exact;
        }

        actions = {
            eg_decap_md13_v4;
            eg_decap_md13_v4_no_dl;
            eg_decap_md13_v6;
            nop;
        }
        
        size = 4;
        const entries = {
            {true, true, false}   : eg_decap_md13_v4;
            {false, false, false}   : eg_decap_md13_v4_no_dl;
            {false, false,true}   : eg_decap_md13_v6;
        }
    }

    apply {
        eg_decap_md13.apply();
    }       
}

control DecapMetaData_I2E02(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action eg_decap_md02_v4() {
        meta.l3.lkp_sip = (bit<128>)hdr.inner_ipv4.srcAddr;
        meta.l3.lkp_dip = (bit<128>)hdr.inner_ipv4.dstAddr;
        meta.ratelimit.bandwidth_id = hdr.bg_md.eip_or_bwid;
    #ifdef __SDE_9_7_SUPPORT__
        meta.l3.lkp_l4_sport = hdr.bg_md.lkp_l4_sport;
        meta.l3.lkp_l4_dport = hdr.bg_md.lkp_l4_dport;
    #endif
    }  

    action eg_decap_md02_v6() {
        meta.l3.lkp_sip = hdr.inner_ipv6.srcAddr;
        meta.l3.lkp_dip = hdr.inner_ipv6.dstAddr;
        meta.ratelimit.bandwidth_id = hdr.bg_md.eip_or_bwid;
    #ifdef __SDE_9_7_SUPPORT__
        meta.l3.lkp_l4_sport = hdr.bg_md.lkp_l4_sport;
        meta.l3.lkp_l4_dport = hdr.bg_md.lkp_l4_dport;
    #endif
    }  

    action nop() {
        meta.l3.lkp_sip = 0;
        meta.l3.lkp_dip = 0;
    #ifdef __SDE_9_7_SUPPORT__
        meta.l3.lkp_l4_sport = 0;
        meta.l3.lkp_l4_dport = 0;
    #endif    
    }

    table eg_decap_md02 {
        key = {
            hdr.inner_ipv4.isValid() : exact;
            hdr.inner_ipv6.isValid() : exact;
        }

        actions = {
            eg_decap_md02_v4;
            eg_decap_md02_v6;
            nop;
        }

        size = 4;
        const entries = {
            {true,false}    : eg_decap_md02_v4;
            {false,true}    : eg_decap_md02_v6;
            {false,false}   : nop;
        }
    }

    apply {
        eg_decap_md02.apply();
    }       
}
