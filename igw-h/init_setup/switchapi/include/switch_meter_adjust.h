/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _SWITCH_METER_ADJUST_H__
#define _SWITCH_METER_ADJUST_H__

#define EIP_IN_METER_ADJUST_BYTE_COUNT 			(-(4 + 50 + 15 + 14))
#define EIP6_IN_METER_ADJUST_BYTE_COUNT 		(-(4 + 50 + 15 + 14))
#define EIP_IN_SHARE_METER_ADJUST_BYTE_COUNT 	(-(4 + 50 + 15 + 14))

#define EIP_OUT_METER_ADJUST_BYTE_COUNT 		(-(4 + 50 + 14 + 15 + 14))
#define EIP6_OUT_METER_ADJUST_BYTE_COUNT 		(-(4 + 50 + 14 + 15 + 14))
#define EIP_OUT_SHARE_METER_ADJUST_BYTE_COUNT 	(-(4 + 50 + 14 + 15 + 14))

extern void meter_adjust_init(void);

#endif

