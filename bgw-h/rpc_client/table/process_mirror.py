#!/usr/bin/env python
# -*- coding:utf-8 -*-

import logging
import grpc
import time
import random
import sys

from jd_bfrt import *

RED = 3 
GREEN = 0 
YELLOW = 1 
MIRROR_PPS = 1000

dev_id = 0
CPU_MIRROR_PORT_4P = 320 
CPU_MIRROR_PORT_2P = 192 
SID_MIRROR_PORT = 0 

CPU_MIRROR_SESSION_ID = 10
SID_MIRROR_SESSION_ID = 20

class ProcessMirror(BfRuntimeBase):
    def create(self, pipenum):
        self.bfrt_info = self.interface.bfrt_info_get()
        self.mirror_table = self.bfrt_info.table_get("P02_Egress.mirror.mirror")
        self.mirror_drop_table = self.bfrt_info.table_get("P02_Egress.mirror.mirror_drop")
        self.target = gc.Target(device_id=0, pipe_id=0xffff)
        self.mirror_cfg_table = self.bfrt_info.table_get("$mirror.cfg")

    def setUp(self, p4_name, grpc_server = 'localhost', pipenum = 4):
        client_id = 0
        BfRuntimeBase.setUp(self, client_id, p4_name, grpc_server, True)
        self.create(pipenum)

    def entry_add_with_clone_to_cpu(self, mirror_flag=0,cir=0,pir=0,cbs=0,pbs=0):
        key_list = [self.mirror_table.make_key([gc.KeyTuple('meta.mirror.flag', mirror_flag)])]
        data_list = [self.mirror_table.make_data([gc.DataTuple('$METER_SPEC_CIR_PPS', cir),
                                                  gc.DataTuple('$METER_SPEC_PIR_PPS', pir),
                                                  gc.DataTuple('$METER_SPEC_CBS_PKTS', cbs),
                                                  gc.DataTuple('$METER_SPEC_PBS_PKTS', pbs)],"clone_to_cpu")]
        try:
            self.mirror_table.entry_add(self.target, key_list, data_list)
        except:
            print("ProcessMirror entry_add_with_clone_to_cpu fail")
        else:
            print("ProcessMirror entry_add_with_clone_to_cpu ok")

    def entry_add_with_drop_packet(self, color=0):
        key_list = [self.mirror_drop_table.make_key([gc.KeyTuple('hdr.bg_md.meter_packet_color', color)])]
        data_list = [self.mirror_drop_table.make_data([],"drop_packet")]
        try:
            self.mirror_drop_table.entry_add(self.target, key_list, data_list)
        except:
            print("ProcessMirror entry_add_with_drop_packet fail")
        else:
            print("ProcessMirror entry_add_with_drop_packet ok")

    def entry_del_all(self):
        try:
            self.mirror_table.entry_del(
                self.target,
                None)
            self.mirror_drop_table.entry_del(
                self.target,
                None)
        except:
            print("ProcessMirror entry_del_all fail")
        else:
            print("ProcessMirror entry_del_all ok")

    def cpu_mirror_session_create(self,sid=0, port=0):
        key_list = [self.mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])]
        data_list = [self.mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="BOTH"),
                                                  gc.DataTuple('$ucast_egress_port', port),
                                                  gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                  gc.DataTuple('$session_enable', bool_val=True),
                                                  gc.DataTuple('$max_pkt_len', 1600)],
                                                  "$normal")]
        try:
            self.mirror_cfg_table.entry_add(self.target, key_list, data_list)
        except:
            print("ProcessMirror cpu_mirror_session_create fail")
        else:
            print("ProcessMirror cpu_mirror_session_create ok")

    def sid_mirror_session_create(self,sid=0, port=0):
        key_list = [self.mirror_cfg_table.make_key([gc.KeyTuple('$sid', sid)])]
        data_list = [self.mirror_cfg_table.make_data([gc.DataTuple('$direction', str_val="BOTH"),
                                                  gc.DataTuple('$ucast_egress_port', port),
                                                  gc.DataTuple('$ucast_egress_port_valid', bool_val=True),
                                                  gc.DataTuple('$session_enable', bool_val=True),
                                                  gc.DataTuple('$max_pkt_len', 1600)],
                                                  "$normal")]
        try:
            self.mirror_cfg_table.entry_add(self.target, key_list, data_list)
        except:
            print("ProcessMirror sid_mirror_session_create fail")
        else:
            print("ProcessMirror sid_mirror_session_create ok")

if __name__ == "__main__":
    bf = ProcessMirror()
    bf.setUp('bgw_switch','127.0.0.1')
    bf.cpu_mirror_session_create(CPU_MIRROR_SESSION_ID, MIRROR_PORT)
    """
    bf.entry_del_all()
    bf.entry_add_with_clone_to_cpu(mirror_flag=0,cir=100,pir=10,cbs=10,pbs=10)
    bf.entry_add_with_drop_packet(color=2)
    """
    bf.tearDown()