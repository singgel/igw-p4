/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include <core.p4>
#if __TARGET_TOFINO__ == 2
#include <t2na.p4>
#else
#include <tna.p4>
#endif

#include "common/defines.h"
#include "common/header.h"
#include "common/meta.h"
#include "common/types.h"
#include "common/table_sizes.h"
#include "common/platform.p4"

#include "src/acl.p4"
#include "src/bridge.p4"
#include "src/hashes.p4"
#include "src/meter.p4"
#include "src/mirror.p4"
#include "src/nexthop.p4"
#include "src/tunnel.p4"
#include "src/vmipv4.p4"
#include "src/vxlan_route.p4"
#include "src/traffic_stats.p4"
#include "src/nat.p4"
#include "src/ratelimit.p4"

#include  "pipe02/P02_ingress.p4"
#include  "pipe02/P02_ingress_parser.p4"
#include  "pipe02/P02_egress.p4"
#include  "pipe02/P02_egress_parser.p4"
#include  "pipe13/P13_ingress.p4"
#include  "pipe13/P13_ingress_parser.p4"
#include  "pipe13/P13_egress.p4"
#include  "pipe13/P13_egress_parser.p4"

Pipeline(P02_IngressParser(),
        P02_Ingress(),
        P02_IngressDeparser(),

        P02_EgressParser(),
        P02_Egress(),
        P02_EgressDeparser()) pipe_ext;

Pipeline(P13_IngressParser(),
        P13_Ingress(),
        P13_IngressDeparser(),

        P13_EgressParser(),
        P13_Egress(),
        P13_EgressDeparser()) pipe_int;

Switch(pipe_ext, pipe_int) main;
