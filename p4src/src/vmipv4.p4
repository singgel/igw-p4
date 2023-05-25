/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/
control VmLocationMapping(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {
    Counter<bit<32>, bit<1>>(2, CounterType_t.PACKETS) vm_host_drop_stats;

    action vm_hostroute_nexthop(bit<16> nexthop,bit<24> vni){
        hdr.bg_md.tunnel_nexthop = nexthop;
        meta.tunnel.route_idx = (bit<32>)vni;
    }
    
    action vm_hostroute_not_hit(){
        hdr.bg_md.need_drop = 1;
        meta.tunnel.route_idx = 0;
    }

    table vm_loc_mapping {
        key = {
            meta.tunnel.fip_dip   : exact;
        }
        
        actions = {
            vm_hostroute_nexthop;
            vm_hostroute_not_hit;
        }
        const default_action = vm_hostroute_not_hit();
        size = EIP_SIZE;
    }
    
    action ipv6_vm_hostroute_nexthop(bit<16> nexthop){
        hdr.bg_md.tunnel_nexthop = nexthop;
    }

    table ipv6_vm_loc_mapping {
        key = {
            hdr.inner_ipv6.dstAddr  : exact;
        }
        
        actions = {
            ipv6_vm_hostroute_nexthop;
            vm_hostroute_not_hit;
        }
        const default_action = vm_hostroute_not_hit();
        size = EIP6_SIZE;
    }

    apply {
        if (hdr.inner_ipv4.isValid()) {
            vm_loc_mapping.apply();
            hdr.inner_ipv4.dstAddr = meta.tunnel.fip_dip;
        } else if (hdr.inner_ipv6.isValid()) { 
            ipv6_vm_loc_mapping.apply();
        }

        if (hdr.bg_md.need_drop == 1) {
            vm_host_drop_stats.count(1);
        }
    }
}

control NexthopProcess(inout headers_t hdr,
            inout common_metadata_t meta,
            in ingress_intrinsic_metadata_t ig_intr_md,
            in ingress_intrinsic_metadata_from_parser_t ig_intr_from_prsr,
            inout ingress_intrinsic_metadata_for_deparser_t ig_intr_md_for_dprsr,
            inout ingress_intrinsic_metadata_for_tm_t  ig_tm_md) {               
    Counter<bit<32>, bit<1>>(2, CounterType_t.PACKETS) nexthop_drop_stats;
    Hash<bit<16>>(HashAlgorithm_t.CRC16) selector_hash;
    ActionProfile(ROUTE_NEXTHOP_SIZE) route_action_profile;
    ActionSelector(route_action_profile, selector_hash, SelectorMode_t.FAIR,
                   64, ROUTE_ECMP_GROUP_TABLE_SZIE) route_ecmp_group_selector;

    action set_ecmp_nexthop(bit<16> nexthop) {
        hdr.bg_md.tunnel_nexthop = nexthop;
    }

    table route_ecmp {
        key = {
            hdr.bg_md.tunnel_nexthop        : exact;
            meta.l3.lkp_sip                 : selector;
            meta.l3.lkp_l4_sport            : selector;
            meta.l3.lkp_dip                 : selector;
            meta.l3.lkp_l4_dport            : selector;
            meta.l3.lkp_ip_proto            : selector;
        }

        actions = {
            set_ecmp_nexthop; 
            NoAction;
        }
        const default_action = NoAction();
        implementation = route_ecmp_group_selector;
        size = ROUTE_ECMP_GROUP_TABLE_SZIE;
    }

    action process_nexthop(bit<8> srcid, bit<12> dstid, bit<24> vni, 
                bit<16> inner_mac_id) {
        hdr.bg_md.lkp_vni = vni;
        hdr.bg_md.tunnel_src_id = srcid;
        hdr.bg_md.tunnel_dst_id = dstid;
        hdr.bg_md.inner_mac_id = inner_mac_id;
    }

    table route_nexthop {
        key = {
            hdr.bg_md.tunnel_nexthop : exact;
        }
        
        actions = {
            process_nexthop;
        }
        
        size = ROUTE_NEXTHOP_SIZE;
    }

    apply {
        route_ecmp.apply();
        if (route_nexthop.apply().hit) {
            //do nothing
        } else if (hdr.bg_md.tunnel_nexthop != 0) {
            //not hit and tunnel_nexthop is exist
            hdr.bg_md.need_drop = 1;
            nexthop_drop_stats.count(1);
        }
    }
}