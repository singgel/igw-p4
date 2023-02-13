#!/usr/bin/env python
# -*- coding:utf-8 -*-

import logging
import grpc
import time
import random
from jd_bfrt import *

class PortStats(BfRuntimeBase):
    def create(self):
        self.bfrt_info = self.interface.bfrt_info_get()
        self.ingress_port_stats_table = self.bfrt_info.table_get("P02_Ingress.ingress_port_stats.ingress_port_stats")
        self.egress_port_stats_table = self.bfrt_info.table_get("P02_Egress.egress_port_stats.egress_port_stats")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)

    def setUp(self, p4_name, grpc_server = 'localhost'):
        client_id = 0
        BfRuntimeBase.setUp(self, client_id, p4_name, grpc_server, True)
        self.create()

    def entry_add_ingress_port_stats(self, isvxlan=0,ingress_port=0):
        key_list = [self.ingress_port_stats_table.make_key([
            gc.KeyTuple('hdr.vxlan.$valid', isvxlan),
            gc.KeyTuple('ig_intr_md.ingress_port', ingress_port)])]
        data_list = [self.ingress_port_stats_table.make_data([], "count")]
        try:
            self.ingress_port_stats_table.entry_add(self.target, key_list, data_list)
        except:
            print("PortStats entry_add_ingress_port_stats fail")
        else:
            print("PortStats entry_add_ingress_port_stats ok")

    def entry_add_egress_port_stats(self, isvxlan=0, egress_port=0):
        key_list = [self.egress_port_stats_table.make_key([
            gc.KeyTuple('hdr.vxlan.$valid', isvxlan),
            gc.KeyTuple('eg_intr_md.egress_port', egress_port)])]
        data_list = [self.egress_port_stats_table.make_data([], "count")]
        try:
            self.egress_port_stats_table.entry_add(self.target, key_list, data_list)
        except:
            print("PortStats entry_add_egress_port_stats fail")
        else:
            print("PortStats entry_add_egress_port_stats ok")

    def ingress_port_stats_entry_get(self, isvxlan, ingress_port):
        try:
            resp = self.ingress_port_stats_table.entry_get(
                self.target,
                [self.ingress_port_stats_table.make_key([
                    gc.KeyTuple('hdr.vxlan.$valid', isvxlan),
                    gc.KeyTuple('ig_intr_md.ingress_port', ingress_port)])],
                {"from_hw": True})
            data, key = next(resp)
            key_fields = key.to_dict()
            data_fields = data.to_dict()
            stats_dict = {}
            stats_dict["key"] = key_fields
            stats_dict["data"] = data_fields
            return stats_dict
        except:
            print("PortStats ingress_port_stats_table fail")
            return None
        else:
            print("PortStats ingress_port_stats_table ok")
            return None

    def ingress_port_stats_entry_get_all_sync(self):
        talbe_list = []
        try:
            self.ingress_port_stats_table.operations_execute(self.target, 'SyncCounters')
            resp = self.ingress_port_stats_table.entry_get(
                self.target, None,{"from_hw": False})
            for data, key in resp:
                data_fields = data.to_dict()
                key_fields = key.to_dict()
                talbe_dict = {}
                talbe_dict["key"] = key_fields
                talbe_dict["data"] = data_fields
                talbe_list.append(talbe_dict)
            return talbe_list
        except:
            print("ingress_port_stats_entry_get_all fail")
        else:
            print("ingress_port_stats_entry_get_all ok")

    def ingress_port_stats_entry_get_all_from_hw(self):
        talbe_list = []
        try:
            resp = self.ingress_port_stats_table.entry_get(
                self.target, None,{"from_hw": True})
            for data, key in resp:
                data_fields = data.to_dict()
                key_fields = key.to_dict()
                talbe_dict = {}
                talbe_dict["key"] = key_fields
                talbe_dict["data"] = data_fields
                talbe_list.append(talbe_dict)
            return talbe_list
        except:
            print("ingress_port_stats_entry_get_all fail")
        else:
            print("ingress_port_stats_entry_get_all ok")

    def ingress_port_stats_entry_get_all(self):
        talbe_list = []
        try:
            resp = self.ingress_port_stats_table.entry_get(
                self.target, None,{"from_hw": False})
            for data, key in resp:
                data_fields = data.to_dict()
                key_fields = key.to_dict()
                talbe_dict = {}
                talbe_dict["key"] = key_fields
                talbe_dict["data"] = data_fields
                talbe_list.append(talbe_dict)
            return talbe_list
        except:
            print("ingress_port_stats_entry_get_all fail")
        else:
            print("ingress_port_stats_entry_get_all ok")

    def egress_port_stats_entry_get(self, isvxlan, egress_port):
        try:
            resp = self.egress_port_stats_table.entry_get(
                self.target,
                [self.egress_port_stats_table.make_key([
                    gc.KeyTuple('hdr.vxlan.$valid', isvxlan),
                    gc.KeyTuple('eg_intr_md.egress_port', egress_port)])],
                {"from_hw": True})
            data, key = next(resp)
            key_fields = key.to_dict()
            data_fields = data.to_dict()
            stats_dict = {}
            stats_dict["key"] = key_fields
            stats_dict["data"] = data_fields
            return stats_dict
        except:
            print("PortStats egress_port_stats_entry_get fail")
            return None
        else:
            print("PortStats egress_port_stats_entry_get ok")
            return None

    def entry_del_all(self):
        try:
            self.ingress_port_stats_table.entry_del(
                self.target,
                None)
            self.egress_port_stats_table.entry_del(
                self.target,
                None)
        except:
            print("PortStats entry_del_all fail")
        else:
            print("PortStats entry_del_all ok")
    def ingress_port_usage(self):
        usage = next(self.ingress_port_stats_table.usage_get(self.target))
        print("ingress_port_usage = ", usage)

if __name__ == "__main__":
    bf = PortStats()
    bf.setUp('bgw_switch','127.0.0.1')
    bf.ingress_port_usage()
    bf.tearDown()




