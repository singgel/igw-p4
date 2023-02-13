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
        meta.l3.lkp_sip = hdr.inner_ipv4.srcAddr;
        meta.l3.lkp_dip = hdr.inner_ipv4.dstAddr;
        ig_init_bridge();
    }   

    action ig_decap_md02_nop() {
        hdr.bg_md.setValid();
        hdr.bg_md.lkp_vni = 0;     
        ig_init_bridge();
    }   
    
    action ig_resubmit_decap_md02_v4() {
        //to do
    }    
    
    action ig_resubmit_decap_md02_nop() {
        //to do
    }

    table ig_decap_md02 {
        key = {
            hdr.inner_ipv4.isValid() : exact;
            ig_intr_md.resubmit_flag : exact;
        }

        actions = {
            ig_decap_md02_v4;
            ig_decap_md02_nop;
            ig_resubmit_decap_md02_v4;
            ig_resubmit_decap_md02_nop;
        }

        size = 4;
        const entries = {
            {true, 0}   : ig_decap_md02_v4;
            {false, 0}   : ig_decap_md02_nop;
            {true, 1}   : ig_resubmit_decap_md02_v4;
            {false, 1}   : ig_resubmit_decap_md02_nop;
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
        if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
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
        meta.l3.lkp_sip = hdr.inner_ipv4.srcAddr;
        meta.l3.lkp_dip = hdr.inner_ipv4.dstAddr;
    #ifdef __SDE_9_7_SUPPORT__
        meta.l3.lkp_l4_sport = hdr.bg_md.lkp_l4_sport;
        meta.l3.lkp_l4_dport = hdr.bg_md.lkp_l4_dport;
    #endif
        hdr.bg_md.lkp_dip = hdr.inner_ipv4.dstAddr;
    }  

    action nop() {}
    
    table eg_decap_md13 {
        key = {
            hdr.inner_ipv4.isValid() : exact;
        }

        actions = {
            eg_decap_md13_v4;
            nop;
        }

        size = 2;
        const entries = {
            {true}    : eg_decap_md13_v4;
            {false}   : nop;
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
        meta.l3.lkp_sip = hdr.inner_ipv4.srcAddr;
        meta.l3.lkp_dip = hdr.inner_ipv4.dstAddr;
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
        }

        actions = {
            eg_decap_md02_v4;
            nop;
        }

        size = 2;
        const entries = {
            {true}    : eg_decap_md02_v4;
            {false}   : nop;
        }
    }

    apply {
        eg_decap_md02.apply();
    }       
}