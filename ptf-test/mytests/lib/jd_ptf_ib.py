#!/usr/bin/env python2.7

import socket
import ptf.packet as scapy
import ptf.dataplane as dataplane
from ptf.testutils import *
from ptf.mask import Mask
from lib.logger import Logger

def verify_dr_packet_port(test, device_number=0, port_number=None, timeout=-1, exp_pkt=None, vr_ip_list=[]):
    result = dp_poll(test, device_number, port_number, timeout, exp_pkt)
    if isinstance(result, test.dataplane.PollSuccess):
        receive_ip = result.packet[0x1e:0x22]
        vr_ip_list_str = []
        for ip in vr_ip_list:
            vr_ip_list_str.append(str(socket.inet_aton(ip)))
        if receive_ip not in vr_ip_list_str: 
            test.fail("The VR IP address was not in the expected VR IP list.\n %s"
                                    % (result.format()))
        else:
            for i in range(0, len(vr_ip_list_str)):
                if receive_ip == vr_ip_list_str[i]:
                    return vr_ip_list[i]
    else:
        test.fail("Expected packet was not received on device %d, port %r.\n%s"
                                    % (device_number, port_number, result.format()))


def verify_vr_nlb_packet_port(test, device_number=0, port_number=None, timeout=-1, exp_pkt=None, mac_verify_list=[]):
    exp_pkt.set_do_not_care(0x4a, 12)
    result = dp_poll(test, device_number, port_number, timeout, exp_pkt)
    if isinstance(result, test.dataplane.PollSuccess):
        src_mac = result.packet[0x38:0x3e]
        dst_mac = result.packet[0x32:0x38]
        mac_tuple = (src_mac, dst_mac)
        if mac_tuple not in mac_verify_list: 
            test.fail("The MAC address pair was not in the expected mac tuple list.\n %s"
                                    % (result.format()))
    else:
        test.fail("Expected packet was not received on device %d, port %r.\n%s"
                                    % (device_number, port_number, result.format()))
