/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control P02_Ingress(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in ingress_intrinsic_metadata_t ig_intr_md,
        in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
        inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
        inout ingress_intrinsic_metadata_for_tm_t ig_tm_md) {
    ProcessLocal()              process_local_packet;
    IgwIpType()                 igw_ip_type;   
    EcmpGroup02()               ecmp_group;
    IngressSystemAcl()          system_acl;
    DecapMetaData_02()          decap_md;
    EncapMetaData_02()          encap_md;
    FipInnerIpSnat()            fip_snat;
    EipOutRedirect()            eip_out_redirect;
    EipOutIngressPktStats()     eip_out_ingress_pkt_stats;

    apply {
        decap_md.apply(IPP_META);
        process_local_packet.apply(IPP_META);
        system_acl.apply(IPP_META);

        if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
            igw_ip_type.apply(IPP_META);
        }
        
        if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
            if (hdr.bg_md.igr_tunnel_type == TYPE_INGRESS_INTERNET_OUT) {//internet out
                fip_snat.apply(IPP_META);
                if (hdr.vxlan.isValid() && (hdr.vxlan.tof != TOF_EIP_OUT) 
                    && hdr.inner_ipv4.isValid())  {
                    eip_out_redirect.apply(IPP_META); 
                } 
                if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
                    eip_out_ingress_pkt_stats.apply(IPP_META); 
                }
            }
        }
        
        ecmp_group.apply(IPP_META);
        encap_md.apply(IPP_META);
    }
}

