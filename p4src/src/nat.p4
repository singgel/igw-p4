/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control FipInnerIpSnat(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
                
    action set_fip(bit<32> fip) {
        hdr.inner_ipv4.srcAddr = fip;
    }

    action nop() {}

    table fip_inner_ip_snat {
        key = {
            hdr.vxlan.vni        : exact;
            meta.l3.lkp_sip      : exact;
        }

        actions = {
            set_fip;
            nop;
        }

        size = 1024;
        const default_action = nop();
    }
    
    apply {
        if (hdr.inner_ipv4.isValid()) {
            fip_inner_ip_snat.apply();
        }
    }
}

control FipIpDnat(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    action set_dip(bit<32> dstip) {
        meta.tunnel.fip_dip = dstip;
    }

    action nop() { }

    table fip_dnat {
        key = {
            meta.tunnel.fip  : exact;
        }

        actions = {
            set_dip;
            nop;
        }

        size = 1024;
        const default_action = nop();
    }
    
    apply {
        if (hdr.vxlan.isValid() && hdr.inner_ipv4.isValid()) { 
            meta.tunnel.fip = hdr.inner_ipv4.dstAddr;
            meta.tunnel.fip_dip = hdr.inner_ipv4.dstAddr;
            fip_dnat.apply();
        } 
    }
}