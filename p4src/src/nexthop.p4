/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control EcmpGroup02(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {

    Hash<bit<16>>(HashAlgorithm_t.CRC16) selector_hash;
    ActionProfile(ECMP_SELECTION_TABLE_SIZE_V2) pipe02_group_action_profile;
    ActionSelector(pipe02_group_action_profile, selector_hash, SelectorMode_t.FAIR,
                   ECMP_MAX_MEMBERS_PER_GROUP_V2,
                   ECMP_GROUP_TABLE_SZIE_V2) pipe02_group_selector;

    action ecmp_group_entry_lag_v2(bit<9> egress_port) {
        ig_tm_md.ucast_egress_port = egress_port;
    }

    table ecmp_group_02_v2 {
        key = {
            meta.l3.egr_pipeline     : exact;
            hdr.bg_md.dl_pkt         : ternary;
            hdr.ipv4.isValid()       : ternary;
            hdr.ipv4.dstAddr         : ternary;
            hdr.vxlan.isValid()      : ternary;
            hdr.inner_ipv4.isValid() : ternary;
            hdr.inner_ipv4.dstAddr   : ternary;
            hdr.inner_ipv4.srcAddr   : ternary;
            meta.l3.lkp_sip         : selector;
            meta.l3.lkp_l4_sport    : selector;
            meta.l3.lkp_dip         : selector;
            meta.l3.lkp_l4_dport    : selector;
            meta.l3.lkp_ip_proto    : selector;
        }

        actions = {
            ecmp_group_entry_lag_v2;
            NoAction;
        }
        const default_action = NoAction();
        implementation = pipe02_group_selector;
        size = ECMP_GROUP_TABLE_SZIE_V2;
    }

    apply{
        if ((hdr.bg_md.tunnel_direct_send == MATCH_PACKET) || 
            (hdr.bg_md.tunnel_direct_send == DL_PACKET)) {
            ig_tm_md.ucast_egress_port = ig_intr_md.ingress_port;
            ecmp_group_02_v2.apply();
        }
    }
}