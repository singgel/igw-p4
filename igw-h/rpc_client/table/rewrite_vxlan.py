#!/usr/bin/env python
# -*- coding:utf-8 -*-

import logging
import grpc
import time
import random
from jd_bfrt import *

VXLAN_TYPE_STD = 1
EGRESS_TUNNEL_TYPE_VXLAN = 1

class RewriteVxlan(BfRuntimeBase):
    def create(self):
        self.bfrt_info = self.interface.bfrt_info_get()
        self.vxlan_gw_process_table = self.bfrt_info.table_get("P02_Egress.rewrite_vxlan.vxlan_gw_process")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)

    def setUp(self, p4_name, grpc_server = 'localhost'):
        client_id = 0
        BfRuntimeBase.setUp(self, client_id, p4_name, grpc_server, True)
        self.create()

    def entry_add_with_set_std_vxlan(self, vxlan_type=0, egr_tunnel_type=0, inner_ipv4_isvlaid=0):
        key_list = [self.vxlan_gw_process_table.make_key([gc.KeyTuple('meta.tunnel.vxlan_type', vxlan_type),
                                             gc.KeyTuple('hdr.bg_md.egr_tunnel_type', egr_tunnel_type),
                                             gc.KeyTuple('hdr.inner_ipv4.$valid', inner_ipv4_isvlaid)])]
        data_list = [self.vxlan_gw_process_table.make_data([],"set_std_vxlan")]
        try:
            self.vxlan_gw_process_table.entry_add(self.target, key_list, data_list)
        except:
            print("RewriteVxlan entry_add_with_set_std_vxlan fail")
        else:
            print("RewriteVxlan entry_add_with_set_std_vxlan ok")

    def entry_del(self, vxlan_type=0, egr_tunnel_type=0, inner_ipv4_isvlaid=0):
        key_list = [self.vxlan_gw_process_table.make_key([gc.KeyTuple('meta.tunnel.vxlan_type', vxlan_type),
                                             gc.KeyTuple('hdr.bg_md.egr_tunnel_type', egr_tunnel_type),
                                             gc.KeyTuple('hdr.inner_ipv4.$valid', inner_ipv4_isvlaid)])]
        try:
            self.vxlan_gw_process_table.entry_del(self.target, key_list)
        except:
            print("RewriteVxlan entry_del fail")
        else:
            print("RewriteVxlan entry_del ok")

    def entry_get_all(self):
        talbe_list = []
        try:
            resp = self.vxlan_gw_process_table.entry_get(
                self.target, None)
            for data, key in resp:
                data_fields = data.to_dict()
                key_fields = key.to_dict()
                talbe_dict = {}
                talbe_dict["key"] = key_fields
                talbe_dict["data"] = data_fields
                talbe_list.append(talbe_dict)
            return talbe_list
        except:
            print("RewriteVxlan entry_get_all fail")
        else:
            print("RewriteVxlan entry_get_all ok")

    def entry_del_all(self):
        try:
            self.vxlan_gw_process_table.entry_del(
                self.target,
                None)
        except:
            print("RewriteVxlan entry_del_all fail")
        else:
            print("RewriteVxlan entry_del_all ok")

if __name__ == "__main__":
    bf = RewriteVxlan()
    """
    bf.setUp('bgw_switch','10.226.137.238')
    bf.entry_del_all()
    bf.entry_add_with_set_std_vxlan(vxlan_type=0, egr_tunnel_type=0, inner_ipv4_isvlaid=1)
    bf.entry_add_with_set_std_vxlan(vxlan_type=0, egr_tunnel_type=1, inner_ipv4_isvlaid=1)
    bf.entry_del(vxlan_type=0, egr_tunnel_type=1, inner_ipv4_isvlaid=1)
    print(bf.entry_get_all())
    """
    bf.tearDown()