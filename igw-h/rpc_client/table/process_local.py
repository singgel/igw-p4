#!/usr/bin/env python
# -*- coding:utf-8 -*-

import logging
import grpc
import time
import random
from jd_bfrt import *

ETHERTYPE_ARP = 0x0806
ETHERTYPE_IPV4 = 0x0800
ETHERTYPE_LLDP = 0x88CC
CPU_PCIE_PORT = 320

HIGHEST_PRI = 0
MIDDLE_PRI = 10
LOWEST_PRI = 20

class ProcessLocal(BfRuntimeBase):
    def create(self):
        self.bfrt_info = self.interface.bfrt_info_get()
        self.processlocal_table = self.bfrt_info.table_get("P02_Ingress.process_local_packet.process_protocol_packet")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)

    def setUp(self, p4_name, grpc_server = 'localhost'):
        client_id = 0
        BfRuntimeBase.setUp(self, client_id, p4_name, grpc_server, True)
        self.create()

    def entry_add_with_copy_to_cpu_nos(self, pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0,
                                        ethertype=0, ethertype_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",
                                        ingress_port=0, ingress_port_mask=0, egress_port=0):
        self.processlocal_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        key_list = [self.processlocal_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.ipv4.$valid', ipv4_isvalid, ipv4_isvalid_mask),
                                             gc.KeyTuple('hdr.ethernet.etherType', ethertype, ethertype_mask),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', dip, dip_mask),
                                             gc.KeyTuple('ig_intr_md.ingress_port', ingress_port, ingress_port_mask)])]
        data_list = [self.processlocal_table.make_data([gc.DataTuple('egress_port', egress_port)],"copy_to_cpu_nos")]
        try:
            self.processlocal_table.entry_add(self.target, key_list, data_list)
        except:
            print("ProcessLocal entry_add_with_copy_to_cpu_nos fail")
        else:
            print("ProcessLocal entry_add_with_copy_to_cpu_nos ok")

    def entry_add_with_receive_from_cpu_nos(self, pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0,
                                        ethertype=0, ethertype_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",
                                        ingress_port=0, ingress_port_mask=0):
        self.processlocal_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        key_list = [self.processlocal_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.ipv4.$valid', ipv4_isvalid, ipv4_isvalid_mask),
                                             gc.KeyTuple('hdr.ethernet.etherType', ethertype, ethertype_mask),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', dip, dip_mask),
                                             gc.KeyTuple('ig_intr_md.ingress_port', ingress_port, ingress_port_mask)])]
        data_list = [self.processlocal_table.make_data([],"receive_from_cpu_nos")]
        try:
            self.processlocal_table.entry_add(self.target, key_list, data_list)
        except:
            print("ProcessLocal entry_add_with_receive_from_cpu_nos fail")
        else:
            print("ProcessLocal entry_add_with_receive_from_cpu_nos ok")

    def entry_add_with_receive_from_cpu_lldp(self, pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0,
                                        ethertype=0, ethertype_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",
                                        ingress_port=0, ingress_port_mask=0):
        self.processlocal_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        key_list = [self.processlocal_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.ipv4.$valid', ipv4_isvalid, ipv4_isvalid_mask),
                                             gc.KeyTuple('hdr.ethernet.etherType', ethertype, ethertype_mask),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', dip, dip_mask),
                                             gc.KeyTuple('ig_intr_md.ingress_port', ingress_port, ingress_port_mask)])]
        data_list = [self.processlocal_table.make_data([],"receive_from_cpu_lldp")]
        try:
            self.processlocal_table.entry_add(self.target, key_list, data_list)
        except:
            print("ProcessLocal entry_add_with_receive_from_cpu_lldp fail")
        else:
            print("ProcessLocal entry_add_with_receive_from_cpu_lldp ok")

    def entry_get_all(self):
        talbe_list = []
        try:
            resp = self.processlocal_table.entry_get(
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
            print("ProcessLocal entry_get_all fail")
        else:
            print("ProcessLocal entry_get_all ok")

    def entry_del(self, pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0,
                                        ethertype=0, ethertype_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",
                                        ingress_port=0, ingress_port_mask=0):
        self.processlocal_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        key_list = [self.processlocal_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.ipv4.$valid', ipv4_isvalid, ipv4_isvalid_mask),
                                             gc.KeyTuple('hdr.ethernet.etherType', ethertype, ethertype_mask),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', dip, dip_mask),
                                             gc.KeyTuple('ig_intr_md.ingress_port', ingress_port, ingress_port_mask)])]
        try:
            self.processlocal_table.entry_del(self.target, key_list)
        except:
            print("ProcessLocal entry_del fail")
        else:
            print("ProcessLocal entry_del ok")

    def entry_del_all(self):
        try:
            self.processlocal_table.entry_del(
                self.target,
                None)
        except:
            print("ProcessLocal entry_del_all fail")
        else:
            print("ProcessLocal entry_del_all ok")

if __name__ == "__main__":
    bf = ProcessLocal()
    bf.setUp('bgw_switch','10.226.137.238')
    """
    bf.entry_del_all()
    bf.entry_add_with_copy_to_cpu_nos(pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0x1,
                        ethertype=0x0806, ethertype_mask=0xFFFF,
                        dip="192.168.100.2", dip_mask="255.255.255.255", 
                        ingress_port=1, ingress_port_mask=0x1FF, 
                        egress_port=64)
    bf.entry_add_with_copy_to_cpu_nos(pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0x1,
                        ethertype=0x0806, ethertype_mask=0xFFFF,
                        dip="192.168.100.3", dip_mask="255.255.255.255", 
                        ingress_port=1, ingress_port_mask=0x1FF, 
                        egress_port=65)
    bf.entry_del(pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0x1,
                        ethertype=0x0806, ethertype_mask=0xFFFF,
                        dip="192.168.100.3", dip_mask="255.255.255.255", 
                        ingress_port=1, ingress_port_mask=0x1FF)
    bf.entry_add_with_receive_from_cpu_nos(pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0x1,
                        ethertype=0x0806, ethertype_mask=0xFFFF,
                        dip="192.168.100.4", dip_mask="255.255.255.255", 
                        ingress_port=1, ingress_port_mask=0x1FF)
    bf.entry_add_with_receive_from_cpu_lldp(pri=0, ipv4_isvalid=0, ipv4_isvalid_mask=0x1,
                        ethertype=0x0806, ethertype_mask=0xFFFF,
                        dip="192.168.100.5", dip_mask="255.255.255.255", 
                        ingress_port=1, ingress_port_mask=0x1FF)
    print(bf.entry_get_all())
    """
    bf.tearDown()