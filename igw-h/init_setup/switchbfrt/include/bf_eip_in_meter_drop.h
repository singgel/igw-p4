/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __BF_EIP_IN_METER_DROP_H__
#define __BF_EIP_IN_METER_DROP_H__

#include "jd_bfrt.h"

extern void eip_in_meter_drop_table_setup();
extern int entry_add_with_eip_in_drop_packet(uint8_t color);

#endif

