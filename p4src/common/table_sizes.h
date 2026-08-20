/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _P4_TABLE_SIZE_
#define _P4_TABLE_SIZE_

const bit<32> INGRESS_SYSTEM_ACL_SIZE = 256;
const bit<32> EGRESS_SYSTEM_ACL_SIZE = 256;

const bit<32> VLINK_TABLE_SIZE = 2048;

const bit<32> ECMP_GROUP_TABLE_SZIE_V2 = 16;
const bit<32> ECMP_SELECTION_TABLE_SIZE_V2 = 64;
const bit<32> ECMP_MAX_MEMBERS_PER_GROUP_V2 = 64;

const bit<32> MPATH_NH_SIZE = 64;

const bit<32> METER_BPS_TABLE_SIZE = 256;

const bit<32> VXLAN_RT_TABLE_SIZE = 40000;

const bit<32> VM_HOST_TABLE_SIZE = 2048;

const bit<32> ROUTE_ECMP_GROUP_TABLE_SZIE = 30720;

const bit<32> ROUTE_NEXTHOP_SIZE = 65536;

const bit<32> TUNNEL_MAC_SIZE = 64;
const bit<32> TUNNEL_INNER_MAC_SIZE = 65536;

const bit<32> TUNNEL_DST_TABLE_SIZE = 2048;
const bit<32> TUNNEL_SRC_TABLE_SIZE = 16;

const bit<32> VXLAN_GW_SIZE = 16;

const bit<32> VMLOCATION_ECMP_NH_SIZE = 64;

const bit<32> VIF_STATS_SIZE = 2048;

const bit<32> EIP_SIZE = 90000;
const bit<32> EIP_SIZE_DOUBLE = 180000;

const bit<32> SHARED_BW_SIZE = 8192;

const bit<32> EIP6_SIZE = 10240;

#endif /* _P4_TABLE_SIZE_ */
