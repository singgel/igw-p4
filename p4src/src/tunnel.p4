/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/


control RewriteVxlan(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    Counter<bit<32>, bit<1>>(2, CounterType_t.PACKETS) dstip_drop_stats;

    action set_std_vxlan() {
        hdr.udp.srcPort = hdr.bg_md.l3_ecmp_entry_idx;
        hdr.udp.dstPort = UDP_PORT_VXLAN;
        hdr.udp.checksum = 0;
        hdr.inner_ipv4.ttl = hdr.inner_ipv4.ttl -1;
        meta.tunnel.inner_ipv4_checksum_en = true;
    }

    table vxlan_gw_process {
        key = {
            meta.tunnel.vxlan_type      : exact;
            hdr.bg_md.egr_tunnel_type   : exact;
            hdr.inner_ipv4.isValid()    : exact;
        }

        actions = {
            set_std_vxlan;
        }
        size = VXLAN_GW_SIZE;
    }

    action rewrite_tunnel_ipv4_dst(bit<32> ip) {
        hdr.ipv4.dstAddr = ip;
    }

    table tunnel_dst_rewrite {
        key = {
            hdr.bg_md.tunnel_dst_id: exact;
        }

        actions = {
            rewrite_tunnel_ipv4_dst;
        }
        size = TUNNEL_DST_TABLE_SIZE;
    }

    action rewrite_tunnel_ipv4_src(bit<32> ip) {
        hdr.ipv4.srcAddr = ip;
    }

    table tunnel_src_rewrite {
        key = {
            hdr.bg_md.tunnel_src_id: exact;
        }

        actions = {
            rewrite_tunnel_ipv4_src;
        }
        size = TUNNEL_SRC_TABLE_SIZE;
    }

    apply {
        if (tunnel_dst_rewrite.apply().hit) {
            //do nothing
        } else if (hdr.bg_md.tunnel_dst_id != 0) {
            //not hit and tunnel_dst_id is exist
            hdr.bg_md.need_drop = 1;
            dstip_drop_stats.count(1);
        }

        tunnel_src_rewrite.apply();
        
        switch (vxlan_gw_process.apply().action_run){
            set_std_vxlan:{
                hdr.vxlan.vni = hdr.bg_md.lkp_vni;
            }
        }
    }
}

control InternetOutProcess(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {

    action internet_out_decap_vxlan() {
        hdr.inner_ethernet.setInvalid();  
        hdr.vxlan.setInvalid();  
        hdr.udp.setInvalid();  
        hdr.ipv4.setInvalid();  
        meta.tunnel.inner_ipv4_checksum_en = true;
    }

    table internet_out_process {
        key = {
            meta.tunnel.vxlan_type      : exact;
            hdr.inner_ipv4.isValid()    : exact;
        }

        actions = {
            internet_out_decap_vxlan;
        }
        size = VXLAN_GW_SIZE;
    }

    apply {
        internet_out_process.apply();
    }
}