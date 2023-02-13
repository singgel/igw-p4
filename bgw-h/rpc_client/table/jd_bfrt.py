#!/usr/bin/env python
# -*- coding:utf-8 -*-

import time
import grpc
import bfrt_grpc.bfruntime_pb2_grpc as bfruntime_pb2_grpc
import bfrt_grpc.bfruntime_pb2 as bfruntime_pb2
import bfrt_grpc.client as gc

import logging
import threading
import json
import sys
import random
import math

import google.rpc.status_pb2 as status_pb2
import google.rpc.code_pb2 as code_pb2

class BfRuntimeBase():
    def __init__(self):
        self.device_id = 0
        self.client_id = 0
        self._swports = []
        self.bfrt_info = None
        self.p4_name = ""

    def tearDown(self):
        if sys.version_info < (3, 0):
            self.interface._tear_down_stream()
        else:
            if not self.interface.is_independent:
                self.interface.tear_down_stream()

    def setUp(self, client_id = 0, p4_name = None, grpc_addr = None,is_master = False,
            notifications = None, perform_bind=True):
        """@brief Set up connection to gRPC server and bind
        @param client_id Client ID
        @param p4_name Name of P4 program. If none is given,
        then the test performs a bfrt_info_get() and binds to the first
        P4 that comes as part of the bfrt_info_get()
        @param is_master If this client needs to be the master. Default = False
        @param notifications A Notifications object.
        If you need to disable any notifications, then do the below as example,
        gc.Notifications(enable_learn=False)
        else default value is sent as below
            enable_learn = True
            enable_idletimeout = True
            enable_port_status_change = True
        @param perform_bind Set this to false if binding is not required
        """
        self.bfrt_info = None
        
        if grpc_addr is None or grpc_addr is 'localhost':
            grpc_addr = 'localhost:50052'
        else:
            grpc_addr = grpc_addr + ":50052"
        
        if sys.version_info < (3, 0):
            self.interface = gc.ClientInterface(grpc_addr, client_id=client_id,
                device_id=0, is_master=is_master, notifications=notifications)
        else:
            self.interface = gc.ClientInterface(grpc_addr, client_id=client_id,
                device_id=0, notifications=notifications, perform_subscribe=True)
        # If p4_name wasn't specified, then perform a bfrt_info_get and set p4_name
        # to it
        if not p4_name:
            self.bfrt_info = self.interface.bfrt_info_get()
            p4_name = self.bfrt_info.p4_name_get()

        # Set forwarding pipeline config (For the time being we are just
        # associating a client with a p4). Currently the grpc server supports
        # only one client to be in-charge of one p4.
        if perform_bind:
            self.interface.bind_pipeline_config(p4_name)

if __name__ == "__main__":
    bf = BfRuntimeBase()
    client_id = 0
    p4_name = "switch"
    bf.setUp(client_id, p4_name)
    bf.tearDown()
    print("hello")
