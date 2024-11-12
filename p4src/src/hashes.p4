/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/
control ComputeIpHashes(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    Hash<bit<16>>(HashAlgorithm_t.CRC16) ip_hash;
    //Hash<bit<32>>(HashAlgorithm_t.CRC32) nlb_ip_hash;

    action compute_inner_ip_hashes(){
        hdr.bg_md.l3_ecmp_entry_idx = ip_hash.get({meta.l3.lkp_sip,
                        meta.l3.lkp_l4_sport,
                        meta.l3.lkp_dip,
                        meta.l3.lkp_l4_dport,
                        meta.l3.lkp_ip_proto});
    }
    
    action compute_nlb_ip_hashes(){
        //hdr.bg_md.l3_ecmp_entry_idx = nlb_ip_hash.get({meta.l3.lkp_sip,(bit<16>)0,(bit<128>)0,(bit<16>)0,(bit<8>)0});
        hdr.bg_md.l3_ecmp_entry_idx = (<bit16>) meta.l3.lkp_sip;
    }

    apply {
        if (hdr.bg_md.nlb_eip == 1) {
            compute_nlb_ip_hashes();
        } else {
            compute_inner_ip_hashes();
        }
    }
}