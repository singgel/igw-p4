/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __SWITCH_DEVICE_H__
#define __SWITCH_DEVICE_H__

#define P4_NAME "igw_switch"

#include "switch_device_int.h"

extern int g_cpu_pcie_port;
int switch_device_create(void);

#endif
