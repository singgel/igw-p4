/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control P13_Egress(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    DecapMetaData_I2E13()           decap_md;
    ComputeIpHashes()               compute_ipv4_hashes;
    #ifdef __MIRROR_ON_ETH__
        ProcessMirror()             mirror;
    #endif

    apply {
        if (meta.mirror.flag != 0) {
            #ifdef __MIRROR_ON_ETH__
                mirror.apply(EPP_META);
            #endif
        } else {
            decap_md.apply(EPP_META);
            compute_ipv4_hashes.apply(EPP_META);
        }
    }
}
