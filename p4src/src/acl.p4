/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control IngressSystemAcl(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
                
    action copy_ingress_to_cpu() {
        meta.tunnel.session_id = CPU_MIRROR_SESSION_ID;
        ig_intr_md_for_dprsr.mirror_type = MIRROR_TYPE_I2E;
        meta.mirror.proto = PKT_MIRR_MD_IG;
        meta.mirror.flag = 1;
        meta.mirror.res = meta.mirror.res | MIRROR_INPUT;
        meta.mirror.timestamp = ig_intr_from_prsr.global_tstamp;
        meta.mirror.port = ig_intr_md.ingress_port;
    }

    action copy_ingress_to_sid(bit<10> sid) {
        meta.tunnel.session_id = sid;
        ig_intr_md_for_dprsr.mirror_type = MIRROR_TYPE_I2E;
        meta.mirror.proto = PKT_MIRR_MD_IG;
        meta.mirror.flag = 1;
        meta.mirror.res = meta.mirror.res | MIRROR_INPUT;
        meta.mirror.timestamp = ig_intr_from_prsr.global_tstamp;
        meta.mirror.port = ig_intr_md.ingress_port;
    }

    action system_acl_drop_packet() {
        ig_intr_md_for_dprsr.drop_ctl = 0x1;
    }

    action nop() {}

    table ingress_system_acl {
        key = {
            hdr.ipv6.isValid()          : ternary;
            hdr.ipv4.isValid()          : ternary;
            hdr.ipv4.dstAddr            : ternary;
            hdr.ipv4.srcAddr            : ternary;
            hdr.ipv4.protocol           : ternary;
            meta.l3.lkp_outer_l4_sport  : ternary;
            meta.l3.lkp_outer_l4_dport  : ternary;
            meta.l3.lkp_sip             : ternary;
            meta.l3.lkp_l4_sport        : ternary;
            meta.l3.lkp_dip             : ternary;
            meta.l3.lkp_l4_dport        : ternary;
            meta.l3.lkp_ip_proto        : ternary;
            hdr.vxlan.isValid()         : ternary;
            hdr.vxlan.vni               : ternary;
            hdr.bg_md.dl_pkt            : ternary;
            hdr.vxlan.tof               : ternary;
            hdr.bg_md.igr_tunnel_type   : ternary;
        }

        actions = {
            copy_ingress_to_cpu;
            copy_ingress_to_sid;
            system_acl_drop_packet;
            nop;
        }

        size = INGRESS_SYSTEM_ACL_SIZE;
        const default_action = nop();
    }
    
    apply {
        if ((ig_intr_md.ingress_port != CPU_PORT)) {
            ingress_system_acl.apply();
        }
    }
}

control EgressSystemAcl(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DirectCounter<bit<32>>(CounterType_t.PACKETS) egress_system_acl_stats;

    action copy_egress_to_cpu() {
        meta.tunnel.session_id = CPU_MIRROR_SESSION_ID;
        eg_dprsr_md.mirror_type = MIRROR_TYPE_E2E;
        meta.mirror.proto = PKT_MIRR_MD_EG;
        meta.mirror.flag = 1;
        meta.mirror.res = meta.mirror.res | MIRROR_OUTPUT;
        meta.mirror.timestamp = eg_prsr_md.global_tstamp;
        meta.mirror.port = eg_intr_md.egress_port;
        egress_system_acl_stats.count();
    }

    action copy_egress_to_sid(bit<10> sid) {
        meta.tunnel.session_id = sid;
        eg_dprsr_md.mirror_type = MIRROR_TYPE_E2E;
        meta.mirror.proto = PKT_MIRR_MD_EG;
        meta.mirror.flag = 1;
        meta.mirror.res = meta.mirror.res | MIRROR_OUTPUT;
        meta.mirror.timestamp = eg_prsr_md.global_tstamp;
        meta.mirror.port = eg_intr_md.egress_port;
        egress_system_acl_stats.count();
    }

    action system_acl_drop_packet() {
        eg_dprsr_md.drop_ctl = 5;     
        egress_system_acl_stats.count();
    }

    action system_acl_drop_mirror_packet() {
        eg_dprsr_md.drop_ctl = 1;     
        copy_egress_to_cpu();
    }

    action nop() {
        egress_system_acl_stats.count();
    }

    table egress_system_acl {
        key = {
            hdr.bg_md.meter_packet_color: ternary;
            hdr.ipv4.isValid()          : ternary;
            hdr.ipv4.dstAddr            : ternary;
            hdr.ipv4.srcAddr            : ternary;
            hdr.ipv4.protocol           : ternary;
            meta.l3.lkp_outer_l4_sport  : ternary;
            meta.l3.lkp_outer_l4_dport  : ternary;
            meta.l3.lkp_dip             : ternary;
            meta.l3.lkp_l4_dport        : ternary;
            meta.l3.lkp_sip             : ternary;
            meta.l3.lkp_l4_sport        : ternary;
            meta.l3.lkp_ip_proto        : ternary;
            hdr.bg_md.lkp_vni           : ternary;
            hdr.bg_md.need_drop         : ternary;
            hdr.bg_md.dl_pkt            : ternary;
            hdr.vxlan.tof               : ternary;
            hdr.bg_md.igr_tunnel_type   : ternary;
        }

        actions = {
            copy_egress_to_cpu;
            copy_egress_to_sid;
            system_acl_drop_packet;
            system_acl_drop_mirror_packet;
            nop;
        }

        size = EGRESS_SYSTEM_ACL_SIZE;
        const default_action = nop();
        counters = egress_system_acl_stats;
    }
    
    apply {
        if (eg_intr_md.egress_port != CPU_PORT) {
            egress_system_acl.apply();
        }
    }
}