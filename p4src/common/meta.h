/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _BGW_META_
#define _BGW_META_

struct tunnel_metadata_t {
    bit<2> vxlan_type;
    bit<32> route_idx;
    bit<32> fip;
    bit<32> fip_dip;
    bool inner_ipv4_checksum_en;
    bit<10> session_id;
    bit<2> drop_reason;
}

struct l3_metadata_t {
    bit<8>  lkp_ip_proto;
    bit<16> lkp_l4_sport;
    bit<16> lkp_l4_dport;    
    bit<128> lkp_sip;
    bit<128> lkp_dip;
    bit<16> lkp_outer_l4_sport;
    bit<16> lkp_outer_l4_dport;  
    bit<8>  lkp_outer_ip_proto;
    bit<3>  egr_pipeline;
}

struct dscp_metadata_t {
    bit<6> val;
}

struct ratelimit_metadata_t {
    bit<18> bandwidth_id;
    bit<1> within_cluster;
    bit<1> between_cluster;
}

@pa_auto_init_metadata

struct common_metadata_t {
    tunnel_metadata_t tunnel;
    l3_metadata_t l3;
    dscp_metadata_t dscp;
    mirror_metadata_t mirror;
    ratelimit_metadata_t ratelimit;
}

#endif /* _BGW_META_ */
