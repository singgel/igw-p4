/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

control ProcessGwEgress(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    EgressRouteProcess()     route_process;
    MeterBpsTable()          meter_bps_table;

    apply {
        route_process.apply(EPP_META);   
        if (hdr.bg_md.pkt_dir != 0) { 
            //cloud to idc
            meter_bps_table.apply(EPP_META);
        } else { 
            // idc to cloud
        }
    }
}

control P02_Egress(
        inout headers_t hdr,
        inout common_metadata_t meta,
        in egress_intrinsic_metadata_t eg_intr_md,
        in egress_intrinsic_metadata_from_parser_t eg_prsr_md,
        inout egress_intrinsic_metadata_for_deparser_t eg_dprsr_md,
        inout egress_intrinsic_metadata_for_output_port_t eg_output_md) {
    ProcessGwEgress()               process_gw_egress;
    RewriteVxlan()                  rewrite_vxlan;                  
    EgressSystemAcl()               egress_system_acl;
    RewriteInnerMac()               tunnel_inner_rewrite;
    RewriteOuterMac()               tunnel_mac_rewrite;
    #ifndef __MIRROR_ON_ETH__
        ProcessMirror()             mirror;
    #endif
    DecapMetaData_I2E02()           decap_md;
    EgressPortStats()               egress_port_stats;     
    EgressVifStats()                egress_vif_stats;

    apply {
        if (hdr.bg_md.isValid()) {
            if (hdr.bg_md.outer_ethernet_invalid == 1w1) {
                hdr.ethernet.setValid();  
                hdr.ethernet.etherType =  hdr.bg_md.outer_ethernet_type;           
            }
            decap_md.apply(EPP_META);
        }

        if (meta.mirror.flag != 0) {
            #ifndef __MIRROR_ON_ETH__
                mirror.apply(EPP_META);
            #endif
        } else {
            if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
               process_gw_egress.apply(EPP_META);     
               rewrite_vxlan.apply(EPP_META);    
            }
            
            egress_system_acl.apply(EPP_META);
            egress_port_stats.apply(EPP_META);
            
            if ((hdr.bg_md.tunnel_direct_send == MATCH_PACKET) && 
                    (hdr.bg_md.need_drop != 1)) {
                egress_vif_stats.apply(EPP_META);
            }
            
            if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
               tunnel_inner_rewrite.apply(EPP_META);           
            }
            
            if (hdr.bg_md.tunnel_direct_send == MATCH_PACKET) {
               tunnel_mac_rewrite.apply(EPP_META);           
            }
        }

        if (hdr.bg_md.isValid()) {
            hdr.bg_md.setInvalid();  
        }
    }
}
