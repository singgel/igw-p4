
import ptf.packet as scapy
import ptf.dataplane as dataplane
import gw_base_test
from ptf.testutils import *
from ptf.mask import Mask
from topo import *

@group("full")
@group("basic")
class ExpectReply(gw_base_test.DataplaneTest):
    def setUp(self):
        super(ExpectReply, self).setUp()
        #do some work here
        pass

    def tearDown(self):
        super(ExpectReply, self).tearDown()
        #do some work here
        pass

    def runTest(self):
        pkt = simple_tcp_packet(eth_dst=switch_mac_address_list[0],
                                eth_src=server_mac_address_list[0],
                                ip_dst='200.64.0.1',
                                ip_src='10.10.50.1',
                                ip_id=101,
                                ip_ttl=64, 
                                with_tcp_chksum=True)
        exp_pkt = simple_vxlan_packet(eth_dst=server_mac_address_list[0],
                                eth_src=switch_mac_address_list[0],
                                ip_id=0,
                                ip_dst=server_ip_address_list[0],
                                ip_src=switch_ip_address_list[0],
                                ip_ttl=64,
                                ip_ihl=5,
                                ip_flags=0x2,
                                vxlan_vni=vxlan_vni_num,
                                with_udp_chksum=False,
                                inner_frame=pkt)
        m = Mask(exp_pkt)
        m.set_do_not_care_scapy(IP, 'ihl')
        m.set_do_not_care_scapy(IP, 'len')
        m.set_do_not_care_scapy(IP, 'chksum')
        m.set_do_not_care_scapy(UDP, 'sport')
        m.set_do_not_care_scapy(UDP, 'len')
        m.set_do_not_care_scapy(UDP, 'chksum')
        try:
            send_packet(self, 0, exp_pkt)
            #verify_packet(self, m, (0,0))
            verify_no_packet(self, m, (0,0),timeout=50)
        finally:
            print "do some clean up"
