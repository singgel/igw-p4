#!/usr/bin/env python
# -*- coding:utf-8 -*-
import sys
from config import *

from pal_rpc import pal
from pal_rpc.ttypes import *

from thrift import Thrift
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol
from thrift.protocol import TMultiplexedProtocol

class PortMgnt():
    def __init__(self):
        self.transport = None
        self.client = None

    def setUp(self, sw_ip = 'localhost'):
        try:
            self.transport = TSocket.TSocket(sw_ip, 9090)
            self.transport = TTransport.TBufferedTransport(self.transport)
            bprotocol = TBinaryProtocol.TBinaryProtocol(self.transport)
            pal_protocol = TMultiplexedProtocol.TMultiplexedProtocol(bprotocol, "pal")
            self.client = pal.Client(pal_protocol)
            self.transport.open()
        except Thrift.TException as tx:
            print('%s' % tx)

    def tearDown(self):
        self.transport.close()

    def portdev_get(self, portnum, device = BF_DEV):
        port_dev = self.client.pal_port_get_first(device)
        for i in range(portnum - 1):
            port_dev = self.client.pal_port_get_next(device, port_dev)
        return port_dev

    def portup_exact(self, portnum, ps, device = BF_DEV, fec = BF_FEC_NONE):
        port_dev = self.portdev_get(portnum)
        self.client.pal_port_add(device, port_dev, ps, fec)
        self.client.pal_port_enable(device, port_dev)

    def portenb_exact(self, portnum, device = BF_DEV):
        port_dev = self.portdev_get(portnum)
        self.client.pal_port_enable(device, port_dev)

    def portdis_exact(self, portnum, device = BF_DEV):
        port_dev = self.portdev_get(portnum)
        self.client.pal_port_dis(device, port_dev)

    def portdel_exact(self, portnum, device = BF_DEV):
        port_dev = self.portdev_get(portnum)
        self.client.pal_port_del(device, port_dev)

    def portup_all_access_port(self, device = BF_DEV, ps = BF_25G, fec = BF_FEC_NONE):
        port_dev = self.client.pal_port_get_first(device)
        for i in range(BF_ACC_PORT_NUM):
            self.client.pal_port_add(device, port_dev, ps, fec)
            self.client.pal_port_enable(device, port_dev)
            port_dev = self.client.pal_port_get_next(device, port_dev)

    def portdel_all_access_port(self, device=BF_DEV):
        port_dev = self.client.pal_port_get_first(device)
        for i in range(BF_ACC_PORT_NUM):
            self.client.pal_port_del(device, port_dev)
            port_dev = self.client.pal_port_get_next(device, port_dev)

    def portup_all_uplink_port(self, device=BF_DEV, ps = BF_100G, fec = BF_FEC_NONE):
        for qsfp in range(BF_UPL_PORT_QSFP_BEGIN, BF_UPL_PORT_QSFP_BEGIN + BF_UPL_PORT_NUM):
            port_dev = self.client.pal_port_front_panel_port_to_dev_port_get(device, front_port=qsfp, front_chnl=0)
            self.client.pal_port_add(device, port_dev, ps, fec)
            self.client.pal_port_enable(device, port_dev)

    def portdel_all_uplink_port(self, device=BF_DEV):
        for qsfp in range(BF_UPL_PORT_QSFP_BEGIN, BF_UPL_PORT_QSFP_BEGIN+BF_UPL_PORT_NUM):
            port_dev = self.client.pal_port_front_panel_port_to_dev_port_get(device, front_port=qsfp, front_chnl=0)
            client.pal_port_del(device, port_dev)

    def port_valid_get(self, portnum, device=BF_DEV):
        '''If port is ADDed, return True, else retun False'''
        port_dev = self.portdev_get(portnum)
        port_is_valid = self.client.pal_port_is_valid(device, port_dev)
        return port_is_valid

    def port_status_get(self, portnum, device=BF_DEV):
        '''If port is UP, return 1, else retun 0'''
        port_num = (portnum-1) *4 + 1
        port_dev = self.portdev_get(port_num)
        if self.client.pal_port_is_valid(device, port_dev):
            port_status = self.client.pal_port_oper_status_get(device, port_dev)
            return port_status
        else:
            return None

    def port_all_stats_get(self, portnum, device=BF_DEV):
        port_dev = self.portdev_get(portnum)
        return self.client.pal_port_all_stats_get(device, port_dev)

    def port_all_stats_get_with_ts(self, portnum, device=BF_DEV):
        port_dev = self.portdev_get(portnum)
        return self.client.pal_port_all_stats_get_with_ts(device, port_dev)

    def port_this_stat_get(self, portnum, ctr_type, device=BF_DEV):
        port_num = (portnum-1) *4 + 1
        port_dev = self.portdev_get(port_num)
        return self.client.pal_port_this_stat_get(device, port_dev, ctr_type)
    
    def port_stats_poll_intvl_get(self, device=BF_DEV):
        return self.client.pal_port_stats_poll_intvl_get(device)

    def port_stats_poll_intvl_set(self, poll_intvl_ms, device=BF_DEV):
        self.client.pal_port_stats_poll_intvl_set(device, poll_intvl_ms)

    def get_ports_list(self, device=BF_DEV):
        ports = []
        ports_num = self.client.pal_max_ports_get(device)
        port_dev = self.client.pal_port_get_first(device)
        ports.append(port_dev)
        for i in range(ports_num - 1):
            port_dev = self.client.pal_port_get_next(device, port_dev)
            ports.append(port_dev)
        return ports

    def print_port_stats(self, portnum):
        print('port: ',portnum,'FramesReceivedOK:', self.port_this_stat_get(portnum, BF_MAC_FramesReceivedOK))
        print('port: ',portnum,'FramesReceivedwithUnicastAddresses:', self.port_this_stat_get(portnum, BF_MAC_FramesReceivedwithUnicastAddresses))
        print('port: ',portnum,'FramesReceivedwithMulticastAddresses:', self.port_this_stat_get(portnum, BF_MAC_FramesReceivedwithMulticastAddresses))
        print('port: ',portnum,'FramesReceivedwithBroadcastAddresses:', self.port_this_stat_get(portnum, BF_MAC_FramesReceivedwithBroadcastAddresses))
        print('port: ',portnum,'FramesTransmittedOK:', self.port_this_stat_get(portnum, BF_MAC_FramesTransmittedOK))
        print('port: ',portnum,'FramesTransmittedUnicast:', self.port_this_stat_get(portnum, BF_MAC_FramesTransmittedUnicast))
        print('port: ',portnum,'FramesTransmittedMulticast:', self.port_this_stat_get(portnum, BF_MAC_FramesTransmittedMulticast))
        print('port: ',portnum,'FramesTransmittedBroadcast:', self.port_this_stat_get(portnum, BF_MAC_FramesTransmittedBroadcast))

#export PYTHONPATH=/root/sde/bf-sde-9.1.0/install/lib/python2.7/site-packages/tofino
if __name__ == "__main__":
    pm = PortMgnt()
    pm.setUp('127.0.0.1')
    #pm.print_port_stats(17)
    #pm.print_port_stats(18)
    print('port: ',17,'up/down:',pm.port_status_get(17))
    print('port: ',18,'up/down:',pm.port_status_get(18))
    print('port: ',19,'up/down:',pm.port_status_get(19))
    print('port: ',20,'up/down:',pm.port_status_get(20))
    pm.tearDown()
