#!/usr/bin/env python2.7


vxlan_vni_num = 0xffffff

switch_password = "onl"

server_mac_address_list = [
'24:6e:96:06:cf:10'
]

server_ip_address_list = [
'192.168.189.16'
]

#Switch ip address
switch_mac_address_list = [
'00:90:fb:60:e2:8b'
]

#Switch management IP
switch_ip_address_list = [
'192.168.255.201'
]

#Swtich Underlay ip address
switch_underlay_ip_address_list = [
'192.168.189.201'
]

#Switch DR to VR ip address 
switch_dv_ip_address_list = [
'192.168.189.201'        
]


#Switch DR to VS ip address
switch_ds_ip_address_list = [
'11.0.0.2'
]

#Switch oif that connecte to the GW
swith_oif_port_list = [
'Ethernet0'        
]

#The IP of the Vxlan pakcage
vr_ip_address_list = [
  #the IP of az1/vr1 that connected to DR
  '192.168.190.101',
  #the IP of az1/vr2 that connected to DR
  '192.168.190.102',
  #the IP of az2/vr1 that connected to DR
  '192.168.190.111',
  #the IP of az2/vr2 that connected to DR
  '192.168.190.112',
  #the IP of az3/vr1 that connected to DR
  '192.168.190.121',
  #the IP of az3/vr2 that connected to DR
  '192.168.190.122',
  #the IP of az4/vr1 that connected to DR
  '192.168.190.131',
  #the IP of az4/vr2 that connected to DR
  '192.168.190.132',
  #the IP of az5/vr1 that connected to DR
  '192.168.190.141',
  #the IP of az5/vr2 that connected to DR
  '192.168.190.142'
]
