/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __BF_MIRROR_CFG_H__
#define __BF_MIRROR_CFG_H__

#include "jd_bfrt.h"

typedef struct normalData_s {
	bool 		session_enable;
	const char *direction;
  	uint64_t 	ucast_egress_port;
	bool 		ucast_egress_port_valid;
  	uint64_t 	max_pkt_len;
} normalData;

extern void mirror_cfg_table_setup();
extern int entry_add_with_normal(uint16_t sid, normalData *data);

#endif

