/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EipInRedirect(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action set_bw_id(bit<18> bandwidth_id, bit<16> shared_bandwidth_id,bit<1> redirect_flag) {
        meta.ratelimit.bandwidth_id = bandwidth_id;
        meta.ratelimit.shared_bandwidth_id = shared_bandwidth_id;
        meta.ratelimit.redirect_flag = redirect_flag;
    }

    action nop() {}

    table eip_in_redirect {
        key = {
            hdr.inner_ipv4.dstAddr  : exact;
        }

        actions = {
            set_bw_id;
            nop;
        }

        size = 1024;
        const default_action = nop();
    }

     action rewrite_jd_vxlan(bit<32> srcip) {
        hdr.vxlan.flags = 0x0c;
        hdr.vxlan.version = 1;
        hdr.vxlan.vni = 125;
        hdr.vxlan.tof = TOF_EIP_IN;
        hdr.udp.srcPort = 250;
        hdr.ipv4.srcAddr = srcip; 
        hdr.bg_md.tunnel_direct_send = DL_PACKET;
    }

    table modify_jd_vxlan {
        key = {
            hdr.vxlan.isValid() : exact;
        }
        size = 2;
        actions = {
            rewrite_jd_vxlan;
        }
        const default_action = rewrite_jd_vxlan(0xffff);
    }

    apply {
        eip_in_redirect.apply();
        if (meta.ratelimit.redirect_flag == 1) {
            modify_jd_vxlan.apply();
        }
    }
}