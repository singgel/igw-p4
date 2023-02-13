#!/bin/bash

#1. install initsetup bgw_start_up
chmod +x ./initsetup
chmod +x ./bgw_start_up
sed -i 's/\r$//g' initsetup
sed -i 's/\r$//g' bgw_start_up
cp ./initsetup /usr/local/bin/
cp ./bgw_start_up /usr/local/bin/

#2. install dummy.ko
cp ./dummy.ko /lib/modules/4.14.151-OpenNetworkLinux/kernel/drivers
depmod -a
modprobe dummy

#3. install lib
cp ./libjansson.so.4.9.0 /usr/local/lib/libjansson.so.4
cp ./libnl-3.so.200.22.0 /usr/local/lib/libnl-3.so.200
cp ./libnl-route-3.so.200.22.0 /usr/local/lib/libnl-route-3.so.200
cp ./libprofiler.so.0 /usr/local/lib/libprofiler.so.0
cp ./libunwind-x86_64.so.8.0.1 /usr/local/lib/libunwind.so.8
cp ./libthrift-0.13.0.so /usr/local/lib/

#4. install lldp
dpkg -i libconfig9_1.5-0.3_amd64.deb
dpkg -i libnl-3-200_3.2.27-2_amd64.deb
dpkg -i lldpad_0.9.46-3.1_amd64.deb

#5. install python lib
tar -zxvf thrift.tar.gz -C /usr/local/lib/python3.5/dist-packages/
tar -zxvf grpc3.5.tar.gz -C /usr/local/lib/python3.5/dist-packages/
tar -zxvf enum.tar.gz -C /usr/local/lib/python3.5/dist-packages/
tar -zxvf google.tar.gz  -C /usr/local/lib/python3.5/dist-packages/
cp protobuf-3.6.1-py3.5-nspkg.pth /usr/local/lib/python3.5/dist-packages/
cp six.py /usr/local/lib/python3.5/dist-packages/

#6. install supervisord
dpkg -i python-meld3_1.0.2-2_all.deb
dpkg -i supervisor_3.3.1-1+deb9u1_all.deb

#7. install sonic-plugins
chmod +x eeprom
chmod +x fancontrol
chmod +x ps_info
chmod +x sfputil
cp ./eeprom /usr/local/bin/
cp ./fancontrol /usr/local/bin/
cp ./ps_info /usr/local/bin/
cp ./sfputil /usr/local/bin/
cp eeprom.py /root/sde/bf-sde-9.7.3/install/lib/python3.5/dist-packages/
cp fancontrol.py /root/sde/bf-sde-9.7.3/install/lib/python3.5/dist-packages/
cp ps_info.py /root/sde/bf-sde-9.7.3/install/lib/python3.5/dist-packages/
cp sfputil.py /root/sde/bf-sde-9.7.3/install/lib/python3.5/dist-packages/

#8. install ops tool
sed -i 's/\r$//g' hbgw_env_check
sed -i 's/\r$//g' network_vip
sed -i 's/\r$//g' no_network_vip
sed -i 's/\r$//g' network_mgt_ip
sed -i 's/\r$//g' no_network_mgt_ip
chmod +x hbgw_env_check
chmod +x network_vip
chmod +x no_network_vip
chmod +x network_mgt_ip
chmod +x no_network_mgt_ip
cp ./hbgw_env_check /usr/local/bin/
cp ./network_vip /usr/local/bin/
cp ./no_network_vip /usr/local/bin/
cp ./network_mgt_ip /usr/local/bin/
cp ./no_network_mgt_ip /usr/local/bin/

#9 install curl
dpkg -i libcurl3_7.52.1-5+deb9u16_amd64.deb
dpkg -i curl_7.52.1-5+deb9u16_amd64.deb
