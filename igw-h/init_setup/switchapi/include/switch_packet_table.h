/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _SWITCH_PACKET_TABLE_H__
#define _SWITCH_PACKET_TABLE_H__

#define HIGHEST_PRI  (0)
#define MIDDLE_PRI 	(10)
#define LOWEST_PRI 	(20)

#define ETHERTYPE_ARP (0x0806)
#define ETHERTYPE_IPV4 (0x0800)
#define ETHERTYPE_LLDP (0x88CC)
#define ETHERTYPE_IPV6 (0x86DD)


extern void process_protocol_packet_table_init(void);
#endif
