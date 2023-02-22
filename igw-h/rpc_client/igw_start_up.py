#!/usr/bin/python
# -*- coding: utf-8 -*-

import sys
if sys.version_info < (3, 0):
    sys.path.append("/root/sde/bf-sde-9.3.1/install/lib/python2.7/site-packages/tofino")
else:
    sys.path.append("/root/sde/bf-sde-9.7.3/install/lib/python3.5/site-packages/tofino")
    sys.path.append("/root/sde/bf-sde-9.7.3/install/lib/python3.5/site-packages/tofino/bfrt_grpc")
    sys.path.append("/root/sde/bf-sde-9.7.3/install/lib/python3.5/dist-packages/rpc_client/table/jd_bfrt")
    sys.path.append("/root/sde/bf-sde-9.7.3/install/lib/python3.5/dist-packages/rpc_client/table")
    sys.path.append("/root/sde/bf-sde-9.7.3/install/lib/python3.5/dist-packages/rpc_client/pm")
from pm import config, portup
from table import jd_bfrt, system_acl,process_local, rewrite_vxlan, process_mirror, ecmp_group02,port_stats
import datetime

P4_NAME = "igw_switch"
GRPC_SERVER ="127.0.0.1"
PIPELINE_NUM = 4

def drop_init():
    bf = system_acl.EgressSystemAcl()
    bf.setUp(P4_NAME, GRPC_SERVER)
    bf.entry_del_all()
    bf.entry_add_with_drop(pri=system_acl.SYSTEM_PRI)
    bf.tearDown()

def port_stats_get_usage():
    bf = port_stats.PortStats()
    bf.setUp(P4_NAME, GRPC_SERVER)
    bf.ingress_port_usage()
    bf.tearDown()

def port_stats_get_all():
    bf = port_stats.PortStats()
    bf.setUp(P4_NAME, GRPC_SERVER)
    start = datetime.datetime.now()
    bf.ingress_port_stats_entry_get_all_from_hw()
    end = datetime.datetime.now()
    r = end - start
    print('---from_hw runtime is %d s %d ms---\n'%(float(r.seconds),float(r.microseconds)))
    start = datetime.datetime.now()
    bf.ingress_port_stats_entry_get_all()
    end = datetime.datetime.now()
    r = end - start
    print('---memory runtime is %d s %d ms---\n'%(float(r.seconds),float(r.microseconds)))
    start = datetime.datetime.now()
    bf.ingress_port_stats_entry_get_all_sync()
    end = datetime.datetime.now()
    r = end - start
    print('---syn runtime is %d s %d ms---\n'%(float(r.seconds),float(r.microseconds)))
    bf.tearDown()

def port_stats_init():
    bf = port_stats.PortStats()
    bf.setUp(P4_NAME, GRPC_SERVER)
    bf.entry_del_all()
    
    if PIPELINE_NUM == 4:
        for i in range(1, 16 + 1):
            bf.entry_add_ingress_port_stats(isvxlan=1,ingress_port=(0 + (i-1)*4))
            bf.entry_add_ingress_port_stats(isvxlan=0,ingress_port=(0 + (i-1)*4))
            bf.entry_add_egress_port_stats(isvxlan=1,egress_port=(0 + (i-1)*4))
            bf.entry_add_egress_port_stats(isvxlan=0,egress_port=(0 + (i-1)*4))
        for i in range(17, 32 + 1):
            bf.entry_add_ingress_port_stats(isvxlan=1,ingress_port=(256 + (i-17)*4))
            bf.entry_add_ingress_port_stats(isvxlan=0,ingress_port=(256 + (i-17)*4))
            bf.entry_add_egress_port_stats(isvxlan=1,egress_port=(256 + (i-17)*4))
            bf.entry_add_egress_port_stats(isvxlan=0,egress_port=(256 + (i-17)*4))
    else:
        for i in range(1, 16 + 1):
            bf.entry_add_ingress_port_stats(isvxlan=1,ingress_port=(128 + (i-1)*4))
            bf.entry_add_ingress_port_stats(isvxlan=0,ingress_port=(128 + (i-1)*4))
            bf.entry_add_egress_port_stats(isvxlan=1,egress_port=(128 + (i-1)*4))
            bf.entry_add_egress_port_stats(isvxlan=0,egress_port=(128 + (i-1)*4))
    bf.tearDown()

def rewrite_vxlan_init():
    bf = rewrite_vxlan.RewriteVxlan()
    bf.setUp(P4_NAME, GRPC_SERVER)
    bf.entry_del_all()
    bf.entry_add_with_set_std_vxlan(vxlan_type=rewrite_vxlan.VXLAN_TYPE_STD, egr_tunnel_type=rewrite_vxlan.EGRESS_TUNNEL_TYPE_VXLAN, 
                    inner_ipv4_isvlaid=1)
    bf.tearDown()

def mirror_init():
    bf = process_mirror.ProcessMirror()
    bf.setUp(P4_NAME, GRPC_SERVER, PIPELINE_NUM)
    bf.entry_del_all()
    bf.entry_add_with_clone_to_cpu(mirror_flag=1,cir=process_mirror.MIRROR_PPS,pir=process_mirror.MIRROR_PPS,cbs=process_mirror.MIRROR_PPS,pbs=process_mirror.MIRROR_PPS)
    bf.entry_add_with_drop_packet(color=process_mirror.RED)  
    if PIPELINE_NUM == 4:
        cpu_port = process_mirror.CPU_MIRROR_PORT_4P
    else:
        cpu_port = process_mirror.CPU_MIRROR_PORT_2P
    bf.cpu_mirror_session_create(process_mirror.CPU_MIRROR_SESSION_ID, cpu_port)
    bf.tearDown()

def ecmp_group02_init_4p():
    bf = ecmp_group02.EcmpGroup02()
    bf.setUp(P4_NAME, GRPC_SERVER)
    bf.entry_del_all()
    members_v = []
    for i in range(1, 16 + 1):
        bf.action_table_entry_add(memberid=i,egress_port=(128 + (i-1)*4))
        members_v.append(i)
    member_status_v = [True] * 16
    bf.sel_table_entry_add(group_id=1, max_grp_size=16,members=members_v,member_status=member_status_v)
    bf.forward_table_entry_add(egr_pipeline=1, group_id=1)

    members_v = []
    for i in range(17, 32 + 1):
        bf.action_table_entry_add(memberid=i,egress_port=(384 + (i-17)*4))
        members_v.append(i)
    member_status_v = [True] * 16
    bf.sel_table_entry_add(group_id=2, max_grp_size=16,members=members_v,member_status=member_status_v)
    bf.forward_table_entry_add(egr_pipeline=3, group_id=2)
    bf.tearDown()

def ecmp_group02_init_2p():
    bf = ecmp_group02.EcmpGroup02()
    bf.setUp(P4_NAME, GRPC_SERVER)
    bf.entry_del_all()
    members_v = []
    for i in range(1, 16 + 1):
        bf.action_table_entry_add(memberid=i,egress_port=(0 + (i-1)*4))
        members_v.append(i)
    member_status_v = [True] * 16
    bf.sel_table_entry_add(group_id=1, max_grp_size=16, members=members_v,member_status=member_status_v)
    bf.forward_table_entry_add(egr_pipeline=1, group_id=1)
    bf.tearDown()

if __name__ == "__main__":
    rewrite_vxlan_init()
    mirror_init()
    if PIPELINE_NUM == 4:
        ecmp_group02_init_4p()
    else:
        ecmp_group02_init_2p()
    port_stats_init()
    drop_init()
    print("BGW init ok")
