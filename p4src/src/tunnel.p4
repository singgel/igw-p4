/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EnCapVxlan(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    bit<16> payload_len;
    bit<8> ip_proto;

    action copy_ipv4_header() {
        // Copy all of the IPv4 header fields.
        hdr.inner_ipv4.setValid();
        hdr.inner_ipv4.version = hdr.ipv4.version;
        hdr.inner_ipv4.ihl = hdr.ipv4.ihl;
        hdr.inner_ipv4.dscp = hdr.ipv4.dscp;
        hdr.inner_ipv4.ecn = hdr.ipv4.ecn;
        hdr.inner_ipv4.totalLen = hdr.ipv4.totalLen;
        hdr.inner_ipv4.identification = hdr.ipv4.identification;
        hdr.inner_ipv4.flags = hdr.ipv4.flags;
        hdr.inner_ipv4.fragOffset = hdr.ipv4.fragOffset;
        hdr.inner_ipv4.ttl = hdr.ipv4.ttl;
        hdr.inner_ipv4.protocol = hdr.ipv4.protocol;
        // hdr.inner_ipv4.hdrChecksum = hdr.ipv4.hdrChecksum;
        hdr.inner_ipv4.srcAddr = hdr.ipv4.srcAddr;
        hdr.inner_ipv4.dstAddr = hdr.ipv4.dstAddr;
        hdr.ipv4.setInvalid();
        meta.tunnel.inner_ipv4_checksum_en = true;
    }

    action rewrite_inner_ipv4_udp() {
        payload_len = hdr.ipv4.totalLen;
        copy_ipv4_header();
        hdr.inner_udp = hdr.udp;
        hdr.udp.setInvalid();
        ip_proto = IP_PROTOCOLS_IPV4;
    }

    action rewrite_inner_ipv4_tcp() {
        payload_len = hdr.ipv4.totalLen;
        copy_ipv4_header();
        hdr.inner_tcp = hdr.tcp;
        hdr.tcp.setInvalid();
        ip_proto = IP_PROTOCOLS_IPV4;
    }

    action rewrite_inner_ipv4_unknown() {
        payload_len = hdr.ipv4.totalLen;
        copy_ipv4_header();
        ip_proto = IP_PROTOCOLS_IPV4;
    }

    table encap_outer {
        key = {
            hdr.ipv4.isValid() : exact;
            hdr.udp.isValid() : exact;
            hdr.tcp.isValid() : exact;
        }

        actions = {
            rewrite_inner_ipv4_udp;
            rewrite_inner_ipv4_tcp;
            rewrite_inner_ipv4_unknown;
        }
        size = 8;
        const entries = {
            (true, false, false) : rewrite_inner_ipv4_unknown();
            (true, true, false) : rewrite_inner_ipv4_udp();
            (true, false, true) : rewrite_inner_ipv4_tcp();
        }
    }

    // Add outer IP encapsulatio
    action add_udp_header(bit<16> src_port, bit<16> dst_port) {
        hdr.udp.setValid();
        hdr.udp.srcPort = src_port;
        hdr.udp.dstPort = dst_port;
        hdr.udp.checksum = 0;
    }

    action add_vxlan_header() {
        hdr.vxlan.setValid();
        hdr.vxlan.flags = 8w0x08;
        hdr.vxlan.tof = 0;
    }

    action add_ipv4_header(bit<8> proto) {
        hdr.ipv4.setValid();
        hdr.ipv4.version = 4w4;
        hdr.ipv4.ihl = 4w5;
        hdr.ipv4.identification = 0;
        hdr.ipv4.flags = 0x2;
        hdr.ipv4.fragOffset = 0;
        hdr.ipv4.protocol = proto;
        hdr.ipv4.ttl = 8w64;
        hdr.ipv4.dscp = 0;
    }

    action rewrite_ipv4_vxlan() {
        hdr.inner_ethernet.setValid();
        hdr.inner_ethernet.etherType = hdr.bg_md.outer_ethernet_type;
        add_ipv4_header(IP_PROTOCOLS_UDP);
        // Total length = packet length + 50
        //   IPv4 (20) + UDP (8) + VXLAN (8)+ Inner Ethernet (14)
        hdr.ipv4.totalLen = payload_len + 16w50;

        add_udp_header(0, UDP_PORT_VXLAN);
        // UDP length = packet length + 30
        //   UDP (8) + VXLAN (8)+ Inner Ethernet (14)
        hdr.udp.length = payload_len + 16w30;

        add_vxlan_header();
        hdr.ethernet.etherType = ETHERTYPE_IPV4;
    }
    
    table add_vxlan {
        key = {
            meta.tunnel.vxlan_type : exact;
        }
        size = 4;
        actions = {
            rewrite_ipv4_vxlan;
        }
        const default_action = rewrite_ipv4_vxlan;
    }
    
    apply {
        // Copy L3/L4 header into inner headers.
        encap_outer.apply();
        // Add outer L3/L4/Tunnel headers.
        add_vxlan.apply();
    }
}

control InternetInProcess(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    Counter<bit<32>, bit<1>>(2, CounterType_t.PACKETS) dstip_drop_stats;

    action rewrite_std_vxlan() {
        hdr.vxlan.flags = 0x08;
        hdr.vxlan.version = 0;
        hdr.vxlan.tof = 0;
        hdr.udp.srcPort = hdr.bg_md.l3_ecmp_entry_idx;
        hdr.udp.dstPort = UDP_PORT_VXLAN;
        hdr.udp.checksum = 0;
        hdr.inner_ipv4.ttl = hdr.inner_ipv4.ttl -1;
        meta.tunnel.inner_ipv4_checksum_en = true;
    }

    action rewrite6_std_vxlan() {
        hdr.udp.srcPort = hdr.bg_md.l3_ecmp_entry_idx;
        hdr.udp.dstPort = UDP_PORT_VXLAN;
        hdr.udp.checksum = 0;
    }

    action nop() {}

    table rewrite_vxlan_process {
        key = {
            hdr.inner_ipv4.isValid()    : exact;
            hdr.inner_ipv6.isValid()    : exact;
        }

        actions = {
            rewrite_std_vxlan;
            rewrite6_std_vxlan;
            nop;
        }

        size = 4;
        const entries = {
            (true,false) : rewrite_std_vxlan();
            (false,true) : rewrite6_std_vxlan();
            (false,false) : nop();
        }

        default_action = nop();
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
        if (hdr.vxlan.isValid()) {
            rewrite_vxlan_process.apply();
            hdr.vxlan.vni = hdr.bg_md.lkp_vni;
        } 
        
        //rewrite overlay mac
        tunnel_inner_rewrite.apply();           

        if (tunnel_dst_rewrite.apply().hit) {
            //do nothing
        } else if (hdr.bg_md.tunnel_dst_id != 0) {
            //not hit and tunnel_dst_id is exist
            hdr.bg_md.need_drop = 1;
            dstip_drop_stats.count(1);
        }
        tunnel_src_rewrite.apply();
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

    action internet6_out_decap_vxlan() {
        hdr.inner_ethernet.setInvalid();  
        hdr.vxlan.setInvalid();  
        hdr.udp.setInvalid();  
        hdr.ipv4.setInvalid();  
    }

    action nop() {}

    table internet_out_process {
        key = {
            hdr.inner_ipv4.isValid()    : exact;
            hdr.inner_ipv6.isValid()    : exact;
        }

        actions = {
            internet_out_decap_vxlan;
            internet6_out_decap_vxlan;
            nop;
        }
        size = 4;
        const entries = {
            {true,false}    : internet_out_decap_vxlan;
            {false,true}    : internet6_out_decap_vxlan;
            {false,false}    : nop;
        }
    }

    apply {
        internet_out_process.apply();
    }
}