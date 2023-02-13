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
    BgwIpType()                 bgw_ip_type;   
    EcmpGroup02()               ecmp_group;
    Vlinklogic()                vlink_logic;
    IngressSystemAcl()          system_acl;
    VxlanRoute()                vxlan_route;
    DecapMetaData_02()          decap_md;
    EncapMetaData_02()          encap_md;
    IngressPortStats()          ingress_port_stats;
    IngressVifStats()           ingress_vif_stats;

    apply {
        decap_md.apply(IPP_META);
        if (ig_intr_md.resubmit_flag == 0) {
            ingress_port_stats.apply(IPP_META);
            process_local_packet.apply(IPP_META);
            system_acl.apply(IPP_META);
        }

        if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
            ingress_vif_stats.apply(IPP_META);
            bgw_ip_type.apply(IPP_META);
        }
        
        if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
            if ((hdr.bg_md.igr_tunnel_type == TYPE_INGRESS_TUNNEL_VBR) && 
                (ig_intr_md.resubmit_flag == 0)) {
                vlink_logic.apply(IPP_META);
            }
            vxlan_route.apply(IPP_META);
        }
        
        if (!meta.tunnel.resubmit) {
            ecmp_group.apply(IPP_META);
            encap_md.apply(IPP_META);
        }
    }
}

