#!/usr/bin/env python
# -*- coding:utf-8 -*-

import logging
import grpc
import time
import random
from jd_bfrt import *

class EcmpGroup02(BfRuntimeBase):
    def create(self):
        self.bfrt_info = self.interface.bfrt_info_get()
        self.forward_table = self.bfrt_info.table_get("P02_Ingress.ecmp_group.ecmp_group_02_v2")
        self.sel_table = self.bfrt_info.table_get("P02_Ingress.ecmp_group.pipe02_group_selector")
        self.action_table = self.bfrt_info.table_get("P02_Ingress.ecmp_group.pipe02_group_action_profile")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)

    def setUp(self, p4_name, grpc_server = 'localhost'):
        client_id = 0
        BfRuntimeBase.setUp(self, client_id, p4_name, grpc_server, True)
        self.create()

    def action_table_entry_add(self, memberid=0,egress_port=0):
        key_list = [self.action_table.make_key([gc.KeyTuple('$ACTION_MEMBER_ID', memberid)])]
        data_list = [self.action_table.make_data([gc.DataTuple('egress_port', egress_port)],'ecmp_group_entry_lag_v2')]
        try:
            self.action_table.entry_add(self.target, key_list, data_list)
        except:
            print("EcmpGroup02 action_table_entry_add fail")
        else:
            print("EcmpGroup02 action_table_entry_add ok")

    def sel_table_entry_add(self, group_id=0, max_grp_size=0,members=[],member_status=[]):
        key_list = [self.sel_table.make_key([gc.KeyTuple('$SELECTOR_GROUP_ID', group_id)])]
        data_list = [self.sel_table.make_data([gc.DataTuple('$MAX_GROUP_SIZE', max_grp_size),
                             gc.DataTuple('$ACTION_MEMBER_ID', int_arr_val=members),
                             gc.DataTuple('$ACTION_MEMBER_STATUS', bool_arr_val=member_status)])]
        try:
            self.sel_table.entry_add(self.target, key_list, data_list)
        except:
            print("EcmpGroup02 sel_table_entry_add fail")
        else:
            print("EcmpGroup02 sel_table_entry_add ok")

    def forward_table_entry_add(self, egr_pipeline=0,group_id=0):
        key_list = [self.forward_table.make_key([gc.KeyTuple('meta.l3.egr_pipeline', egr_pipeline)])]
        data_list = [self.forward_table.make_data([gc.DataTuple('$SELECTOR_GROUP_ID', group_id)])]
        try:
            self.forward_table.entry_add(self.target, key_list, data_list)
        except:
            print("EcmpGroup02 forward_table_entry_add fail")
        else:
            print("EcmpGroup02 forward_table_entry_add ok")

    def entry_del_all(self):
        try:        
            self.forward_table.entry_del(
                self.target,
                None)
            self.sel_table.entry_del(
                self.target,
                None)    
            self.action_table.entry_del(
                self.target,
                None)
        except:
            print("EcmpGroup02 entry_del_all fail")
        else:
            print("EcmpGroup02 entry_del_all ok")

if __name__ == "__main__":
    bf = EcmpGroup02()
    """
    bf.setUp('bgw_switch','10.226.137.238')
    bf.entry_del_all()
    bf.action_table_entry_add(memberid=1,egress_port=160)
    bf.action_table_entry_add(memberid=2,egress_port=161)
    bf.action_table_entry_add(memberid=3,egress_port=162)
    bf.action_table_entry_add(memberid=4,egress_port=163)
    max_grp_size_v = 4
    member_status_v = [True] * max_grp_size_v
    members_v = [1,2,3,4]
    bf.sel_table_entry_add(group_id=1, max_grp_size=max_grp_size_v,members=members_v,member_status=member_status_v)
    bf.forward_table_entry_add(egr_pipeline=1,group_id=1)
    """
    bf.tearDown()