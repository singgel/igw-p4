/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control P13_Ingress(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {
    NexthopProcess()  nexthop_process;

    apply {
        ig_tm_md.ucast_egress_port =  hdr.bg_md.igr_port;
        if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
            nexthop_process.apply(IPP_META);
        }
    }
}
