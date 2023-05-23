#!/usr/bin/env python
# -*- coding:utf-8 -*-

import logging
import grpc
import time
import random
from jd_bfrt import *

UDP_PROTOCOL = 17

HIGHEST_PRI = 0
MIRROR_PRI = 10
SYSTEM_PRI = 20
USER_PRI = 30

class EgressSystemAcl(BfRuntimeBase):
    def create(self):
        self.bfrt_info = self.interface.bfrt_info_get()
        self.system_acl_table = self.bfrt_info.table_get("P02_Egress.egress_system_acl.egress_system_acl")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)

    def setUp(self, p4_name, grpc_server = 'localhost'):
        client_id = 0
        BfRuntimeBase.setUp(self, client_id, p4_name, grpc_server, True)
        self.create()

    def entry_add_with_drop(self, pri=USER_PRI):
        self.system_acl_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        self.system_acl_table.info.key_field_annotation_add("hdr.ipv4.srcAddr", "ipv4")
        self.system_acl_table.info.key_field_annotation_add("meta.l3.lkp_dip", "ipv6")
        self.system_acl_table.info.key_field_annotation_add("meta.l3.lkp_sip", "ipv6")
        key_list = [self.system_acl_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.bg_md.meter_packet_color', 0, 0),
                                             gc.KeyTuple('hdr.ipv4.$valid', 1, 0x1),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', "0.0.0.0", "0.0.0.0"),
                                             gc.KeyTuple('hdr.ipv4.srcAddr', "0.0.0.0", "0.0.0.0"),
                                             gc.KeyTuple('hdr.ipv4.protocol', UDP_PROTOCOL, 0xFF),
                                             gc.KeyTuple('meta.l3.lkp_outer_l4_sport', 0, 0),
                                             gc.KeyTuple('meta.l3.lkp_outer_l4_dport', 0, 0),
                                             #gc.KeyTuple('meta.l3.lkp_dip', "0.0.0.0", "0.0.0.0"),
                                             gc.KeyTuple('meta.l3.lkp_l4_dport', 0, 0),
                                             #gc.KeyTuple('meta.l3.lkp_sip', "0.0.0.0", "0.0.0.0"),
                                             gc.KeyTuple('meta.l3.lkp_l4_sport', 0, 0),
                                             gc.KeyTuple('meta.l3.lkp_ip_proto', 0, 0),
                                             gc.KeyTuple('hdr.bg_md.lkp_vni', 0, 0),  
                                             gc.KeyTuple('hdr.bg_md.need_drop', 1, 1),  
                                             gc.KeyTuple('eg_intr_md.egress_port', 0, 0)])]
        data_list = [self.system_acl_table.make_data([],"system_acl_drop_packet")]
        try:
            self.system_acl_table.entry_add(self.target, key_list, data_list)
        except:
            print("EgressSystemAcl entry_add_with_nop fail")
        else:
            print("EgressSystemAcl entry_add_with_nop ok")

    def entry_del_all(self):
        try:
            self.system_acl_table.entry_del(
                self.target,
                None)
        except:
            print("EgressSystemAcl entry_del_all fail")
        else:
            print("EgressSystemAcl entry_del_all ok")

if __name__ == "__main__":
    bf = EgressSystemAcl()
    bf.setUp('bgw_switch','127.0.0.1')
    bf.entry_del_all()
    bf.entry_add_with_drop()
    bf.tearDown()