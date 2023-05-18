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
    Hash<bit<16>>(HashAlgorithm_t.CRC16) selector_hash;
    ActionProfile(64) dl_ip_group_action_profile;
    ActionSelector(dl_ip_group_action_profile, selector_hash, SelectorMode_t.FAIR,
                   256,2) dl_ip_group_selector;
    bit<32> hash_index;

    action nop() {
        hdr.bg_md.shared_bandwidth_id = 0;
    }

    action set_bw_id(bit<18> bandwidth_id, bit<12> shared_bandwidth_id,
            bit<1> within_cluster, bit<1> between_cluster) {
        meta.ratelimit.bandwidth_id = bandwidth_id;
        hdr.bg_md.shared_bandwidth_id = (bit<16>) shared_bandwidth_id;
        meta.ratelimit.within_cluster = within_cluster;
        meta.ratelimit.between_cluster = between_cluster;
    }

    table eip_in_redirect {
        key = {
            hdr.inner_ipv4.dstAddr  : exact;
        }

        actions = {
            set_bw_id;
            nop;
        }

        size = 150000;
        const default_action = nop();
    }
    
    action nop2() {}

    action rewrite_az_in_jd_vxlan(bit<32> shared_bw_vip) {
        hdr.vxlan.flags = 0x0c;
        hdr.vxlan.version = 1;
        hdr.vxlan.vni = 99;
        hdr.vxlan.tof = TOF_AZ_IN;
        hdr.udp.srcPort = 50;
        hdr.ipv4.dstAddr = shared_bw_vip; 
        hdr.bg_md.tunnel_direct_send = DL_PACKET;
    }

     action rewrite_eip_in_jd_vxlan(bit<32> srcip) {
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
            hdr.vxlan.isValid() : ternary;
            hdr.vxlan.tof: ternary;
            meta.ratelimit.between_cluster : ternary;
            meta.ratelimit.within_cluster : ternary;
        }
        actions = {
            rewrite_eip_in_jd_vxlan;
            rewrite_az_in_jd_vxlan;
            nop2;
        }        
        size = 6;
    }
  
    action ecmp_dl_ip(bit<32> dl_ip) {
        hdr.ipv4.dstAddr = dl_ip; 
    }

    table select_redirect_ip {
        key = {
            hdr.vxlan.isValid() : exact;
            hash_index         : selector;
        }
        actions = {
            ecmp_dl_ip;
        }
        implementation = dl_ip_group_selector;
        size = 2;
    }

    apply {
        eip_in_redirect.apply();
        switch (modify_jd_vxlan.apply().action_run){
            rewrite_eip_in_jd_vxlan:{
                hash_index =  (bit<32>)meta.ratelimit.bandwidth_id;
                if (hdr.bg_md.shared_bandwidth_id != 0) {
                    hash_index =  (bit<32>)hdr.bg_md.shared_bandwidth_id;
                }
                select_redirect_ip.apply();
            }
        }
    }
}

control EipOutRedirect(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    Hash<bit<16>>(HashAlgorithm_t.CRC16) selector_hash;
    ActionProfile(64) dl_ip_group_action_profile;
    ActionSelector(dl_ip_group_action_profile, selector_hash, SelectorMode_t.FAIR,
                   256,2) dl_ip_group_selector;
    bit<32> hash_index;

    action nop() {
        hdr.bg_md.shared_bandwidth_id = 0;
    }
    action set_bw_id(bit<18> bandwidth_id, bit<12> shared_bandwidth_id,
            bit<1> within_cluster, bit<1> between_cluster) {
        //meta.ratelimit.bandwidth_id = bandwidth_id;
        hdr.bg_md.eip_or_bwid = (bit<32>)bandwidth_id;
        hdr.bg_md.shared_bandwidth_id = (bit<16>) shared_bandwidth_id;
        meta.ratelimit.within_cluster = within_cluster;
        meta.ratelimit.between_cluster = between_cluster;
    }

    table eip_out_redirect {
        key = {
            hdr.inner_ipv4.srcAddr  : exact;
        }

        actions = {
            set_bw_id;
            nop;
        }

        size = 150000;
        const default_action = nop();
    }
    
    action nop2() {}

    action rewrite_az_out_jd_vxlan(bit<32> shared_bw_vip) {
        hdr.vxlan.flags = 0x0c;
        hdr.vxlan.version = 1;
        hdr.vxlan.tof = TOF_AZ_OUT;
        hdr.udp.srcPort = 50;
        hdr.ipv4.dstAddr = shared_bw_vip; 
        hdr.bg_md.tunnel_direct_send = DL_PACKET;
    }

     action rewrite_eip_out_jd_vxlan(bit<32> srcip) {
        hdr.vxlan.flags = 0x0c;
        hdr.vxlan.version = 1;
        hdr.vxlan.tof = TOF_EIP_OUT;
        hdr.udp.srcPort = 250;
        hdr.ipv4.srcAddr = srcip; 
        hdr.bg_md.tunnel_direct_send = DL_PACKET;
    }

    table modify_jd_vxlan {
        key = {
            hdr.vxlan.isValid() : ternary;
            hdr.vxlan.tof: ternary;
            meta.ratelimit.between_cluster : ternary;
            meta.ratelimit.within_cluster : ternary;
        }
        actions = {
            rewrite_eip_out_jd_vxlan;
            rewrite_az_out_jd_vxlan;
            nop2;
        }        
        size = 6;
    }
  
    action ecmp_dl_ip(bit<32> dl_ip) {
        hdr.ipv4.dstAddr = dl_ip; 
    }

    table select_redirect_ip {
        key = {
            hdr.vxlan.isValid() : exact;
            hash_index         : selector;
        }
        actions = {
            ecmp_dl_ip;
        }
        implementation = dl_ip_group_selector;
        size = 2;
    }

    apply {
        eip_out_redirect.apply();
        switch (modify_jd_vxlan.apply().action_run){
            rewrite_eip_out_jd_vxlan:{
                hash_index =  hdr.bg_md.eip_or_bwid;
                if (hdr.bg_md.shared_bandwidth_id != 0) {
                    hash_index =  (bit<32>)hdr.bg_md.shared_bandwidth_id;
                }
                select_redirect_ip.apply();
            }
        }
    }
}
