#!/usr/bin/env python
# -*- coding:utf-8 -*-

import logging
import grpc
import time
import random
from jd_bfrt import *

class BgwIpType(BfRuntimeBase):
    def create(self):
        self.bfrt_info = self.interface.bfrt_info_get()
        self.bgw_ip_type_table = self.bfrt_info.table_get("P02_Ingress.bgw_ip_type.bgw_ip_type")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)

    def setUp(self, p4_name, grpc_server = 'localhost'):
        client_id = 0
        BfRuntimeBase.setUp(self, client_id, p4_name, grpc_server, True)
        self.create()

    def entry_add_with_ip_from_vpc_hit(self, pri=0, vxlan_isvalid=0,
                                        vni=0, vni_mask=0,
                                        ipv4_dstAddr="0.0.0.0", ipv4_dstAddr_mask="0.0.0.0",
                                        protocol=0,protocol_mask=0,
                                        outer_l4_dport=0,outer_l4_dport_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",                                       
                                        ingress_port=0, ingress_port_mask=0, egress_id=0):
        self.bgw_ip_type_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        self.bgw_ip_type_table.info.key_field_annotation_add("meta.l3.lkp_dip", "ipv4")
        key_list = [self.bgw_ip_type_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.vxlan.$valid', vxlan_isvalid),
                                             gc.KeyTuple('hdr.vxlan.vni', vni, vni_mask),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', ipv4_dstAddr, ipv4_dstAddr_mask),
                                             gc.KeyTuple('hdr.ipv4.protocol', protocol, protocol_mask),
                                             gc.KeyTuple('meta.l3.lkp_outer_l4_dport', outer_l4_dport, outer_l4_dport_mask),
                                             gc.KeyTuple('meta.l3.lkp_dip', dip, dip_mask),
                                             gc.KeyTuple('ig_intr_md.ingress_port', ingress_port, ingress_port_mask)])]
        data_list = [self.bgw_ip_type_table.make_data([gc.DataTuple('egress_id', egress_id)],"ip_from_vpc_hit")]
        try:
            self.bgw_ip_type_table.entry_add(self.target, key_list, data_list)
        except:
            print("BgwIpType entry_add_with_ip_from_vpc_hit fail")
        else:
            print("BgwIpType entry_add_with_ip_from_vpc_hit ok")

    def entry_add_with_ip_from_vbr_hit(self, pri=0, vxlan_isvalid=0,
                                        vni=0, vni_mask=0,
                                        ipv4_dstAddr="0.0.0.0", ipv4_dstAddr_mask="0.0.0.0",
                                        protocol=0,protocol_mask=0,
                                        outer_l4_dport=0,outer_l4_dport_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",                                       
                                        ingress_port=0, ingress_port_mask=0, egress_id=0):
        self.bgw_ip_type_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        self.bgw_ip_type_table.info.key_field_annotation_add("meta.l3.lkp_dip", "ipv4")
        key_list = [self.bgw_ip_type_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.vxlan.$valid', vxlan_isvalid),
                                             gc.KeyTuple('hdr.vxlan.vni', vni, vni_mask),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', ipv4_dstAddr, ipv4_dstAddr_mask),
                                             gc.KeyTuple('hdr.ipv4.protocol', protocol, protocol_mask),
                                             gc.KeyTuple('meta.l3.lkp_outer_l4_dport', outer_l4_dport, outer_l4_dport_mask),
                                             gc.KeyTuple('meta.l3.lkp_dip', dip, dip_mask),
                                             gc.KeyTuple('ig_intr_md.ingress_port', ingress_port, ingress_port_mask)])]
        data_list = [self.bgw_ip_type_table.make_data([gc.DataTuple('egress_id', egress_id)],"ip_from_vbr_hit")]
        try:
            self.bgw_ip_type_table.entry_add(self.target, key_list, data_list)
        except:
            print("BgwIpType entry_add_with_ip_from_vbr_hit fail")
        else:
            print("BgwIpType entry_add_with_ip_from_vbr_hit ok")

    def entry_del(self, pri=0, vxlan_isvalid=0,
                                        vni=0, vni_mask=0,
                                        ipv4_dstAddr="0.0.0.0", ipv4_dstAddr_mask="0.0.0.0",
                                        protocol=0,protocol_mask=0,
                                        outer_l4_dport=0,outer_l4_dport_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",                                       
                                        ingress_port=0, ingress_port_mask=0):
        self.bgw_ip_type_table.info.key_field_annotation_add("hdr.ipv4.dstAddr", "ipv4")
        self.bgw_ip_type_table.info.key_field_annotation_add("meta.l3.lkp_dip", "ipv4")
        key_list = [self.bgw_ip_type_table.make_key([gc.KeyTuple('$MATCH_PRIORITY', pri),
                                             gc.KeyTuple('hdr.vxlan.$valid', vxlan_isvalid),
                                             gc.KeyTuple('hdr.vxlan.vni', vni, vni_mask),
                                             gc.KeyTuple('hdr.ipv4.dstAddr', ipv4_dstAddr, ipv4_dstAddr_mask),
                                             gc.KeyTuple('hdr.ipv4.protocol', protocol, protocol_mask),
                                             gc.KeyTuple('meta.l3.lkp_outer_l4_dport', outer_l4_dport, outer_l4_dport_mask),
                                             gc.KeyTuple('meta.l3.lkp_dip', dip, dip_mask),
                                             gc.KeyTuple('ig_intr_md.ingress_port', ingress_port, ingress_port_mask)])]
        try:
            self.bgw_ip_type_table.entry_del(self.target, key_list)
        except:
            print("BgwIpType entry_del fail")
        else:
            print("BgwIpType entry_del ok")

    def entry_get_all(self):
        talbe_list = []
        try:
            resp = self.bgw_ip_type_table.entry_get(
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
            print("BgwIpType entry_get_all fail")
        else:
            print("BgwIpType entry_get_all ok")

    def entry_del_all(self):
        try:
            self.bgw_ip_type_table.entry_del(
                self.target,
                None)
        except:
            print("BgwIpType entry_del_all fail")
        else:
            print("BgwIpType entry_del_all ok")

if __name__ == "__main__":
    bf = BgwIpType()
    """
    bf.setUp('bgw_switch','10.226.137.238')
    bf.entry_del_all()
    bf.entry_add_with_ip_from_vpc_hit(pri=0, vxlan_isvalid=1,
                                        vni=100, vni_mask=0,
                                        ipv4_dstAddr="198.10.10.1", ipv4_dstAddr_mask="255.255.255.0",
                                        protocol=0,protocol_mask=0,
                                        outer_l4_dport=0,outer_l4_dport_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",                                       
                                        ingress_port=0, ingress_port_mask=0, egress_id=1)
    bf.entry_add_with_ip_from_vbr_hit(pri=0, vxlan_isvalid=0,
                                        vni=200, vni_mask=0xFF,
                                        ipv4_dstAddr="198.10.10.1", ipv4_dstAddr_mask="255.255.255.0",
                                        protocol=0,protocol_mask=0,
                                        outer_l4_dport=0,outer_l4_dport_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",                                       
                                        ingress_port=0, ingress_port_mask=0, egress_id=1)
    bf.entry_add_with_ip_from_vbr_hit(pri=0, vxlan_isvalid=1,
                                        vni=300, vni_mask=0xFF,
                                        ipv4_dstAddr="198.10.10.1", ipv4_dstAddr_mask="255.255.255.0",
                                        protocol=0,protocol_mask=0,
                                        outer_l4_dport=0,outer_l4_dport_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",                                       
                                        ingress_port=0, ingress_port_mask=0, egress_id=1)
    bf.entry_del(pri=0, vxlan_isvalid=1,
                                        vni=300, vni_mask=0xFF,
                                        ipv4_dstAddr="198.10.10.1", ipv4_dstAddr_mask="255.255.255.0",
                                        protocol=0,protocol_mask=0,
                                        outer_l4_dport=0,outer_l4_dport_mask=0,
                                        dip="0.0.0.0", dip_mask="0.0.0.0",                                       
                                        ingress_port=0, ingress_port_mask=0)
    print(bf.entry_get_all())
    """
    bf.tearDown()