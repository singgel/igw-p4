/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control ComputeIpHashes(inout headers_t hdr,
            inout common_metadata_t meta,
            in egress_intrinsic_metadata_t eg_intr_md,
            in egress_intrinsic_metadata_from_parser_t eg_intr_from_prsr,
            inout egress_intrinsic_metadata_for_deparser_t eg_intr_md_for_dprsr,
            inout egress_intrinsic_metadata_for_output_port_t  eg_output_md) {
    Hash<bit<16>>(HashAlgorithm_t.CRC16) ip_hash1;
    Hash<bit<16>>(HashAlgorithm_t.CRC16) ip_hash2;

    action compute_inner_ip_hashes(){
        hdr.bg_md.l3_ecmp_entry_idx = ip_hash1.get({meta.l3.lkp_sip,
                        meta.l3.lkp_l4_sport,
                        meta.l3.lkp_dip,
                        meta.l3.lkp_l4_dport,
                        meta.l3.lkp_ip_proto});
    }

    action compute_ip_hashes(){
        hdr.bg_md.l3_ecmp_entry_idx = ip_hash2.get({hdr.ipv4.srcAddr,
                        meta.l3.lkp_outer_l4_sport,
                        hdr.ipv4.dstAddr,
                        meta.l3.lkp_outer_l4_dport,
                        meta.l3.lkp_outer_ip_proto});
    }

    apply {
        if (hdr.vxlan.isValid() && hdr.inner_ipv4.isValid() &&
            (hdr.bg_md.dl_pkt == 1)) { //dl_packet
           compute_inner_ip_hashes();
        } else if (hdr.ipv4.isValid()) {
           compute_ip_hashes();
        } 
    }
}