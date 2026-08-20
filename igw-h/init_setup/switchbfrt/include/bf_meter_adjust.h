/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __BF_METER_ADJUST_H__
#define __BF_METER_ADJUST_H__

#include "jd_bfrt.h"

extern void eip_in_meter_adjust_setup();
extern void eip_in_meter_byte_count_adjust_set(const int byte_count_adj);
extern void eip6_in_meter_adjust_setup();
extern void eip6_in_meter_byte_count_adjust_set(const int byte_count_adj);
extern void eip_in_share_meter_adjust_setup();
extern void eip_in_share_meter_byte_count_adjust_set(const int byte_count_adj);
extern void eip_out_meter_adjust_setup();
extern void eip_out_meter_byte_count_adjust_set(const int byte_count_adj);
extern void eip6_out_meter_adjust_setup();
extern void eip6_out_meter_byte_count_adjust_set(const int byte_count_adj);
extern void eip_out_share_meter_adjust_setup();
extern void eip_out_share_meter_byte_count_adjust_set(const int byte_count_adj);

#endif

