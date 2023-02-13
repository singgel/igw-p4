# Regression Test cases based on PTF
More info about PTF, see [PTF][1] on GitHub 


----------

## Test Beds Topo

![](http://cf.jd.com/download/attachments/102549564/image2018-3-26%2018%3A44%3A19.png?version=1&modificationDate=1522061054000&api=v2)

## How to RUN the tese cases by directory
`sudo ptf --test-dir tests --platform-dir platforms --relax -P remote`

NOTE: we use "--relax" to check only our expected packet, ignore other packet, such as ICMP


## How to RUN one group test cases
You can choose to run only the tests belonging to a given group using a command like this one:

    sudo ptf --test-dir tests/ **<name of group>**


## How to RUN one specific test case
Currently there is no command parameter to support one test case name, but there are some ways to do this
- You can add your test case into one new file, and use ptf "-f" parameter.
- You can use group decorator to add a new group for your test case, and then use "group name" in the ptf.


## How to update the configuraiton according to your test beds env
1. update the dict "**remote_port_map**" in platforms/remote.py to match your test bed enviroment.
2. update all the list in **"tests/topo.py"**, it is about the MAC & IP of your switch and server.

## How to set up driver node
1. install scapy
	
	yum -y install tcpdump graphviz ImageMagick
	
	download https://github.com/secdev/scapy
	
	cd scapy-master
	
	python setup.py install

2. install pip
	
	yum install epel-release
	
	yum install python-pip

3. install pypcap 
  	
	yum -y install libpcap-devel.x86_64
  	
	pip install pypcap

4. install PTF
	
	pip install six==1.16.0
 	
	git clone https://github.com/p4lang/ptf
	
	cd ptf; python setup.py install

5. install paramiko

	yum install python-paramiko

## How to add test case
1. create a new file under test directory
2. import the necessary library from ptf
3. create a new derived class from gw_base_test.DataplaneTest
4. implement the function of "**runTest**".
5. if there are some pre-settings works, please implement the "**setUp**" function
6. if there are some clean-up works, please implement the "**tearDown**" function 


## How to disable one tese case
PTF provides a convenient **disabled** decorator for tests.
You can add "@disabled" before the class name.


## How to group the test cases
PTF suppor to group the test cases, and suppor to run only one specific
test cases group. If you want to add one test case in to group '**a**', then
you need to add "**@group('a')**" before the class name. 

###***Note: If one test cases need to be added into more than one groups, please add all the group names.***


  [1]: https://github.com/p4lang/ptf/blob/master/README.md
