/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __EIP_IN_SELECT_IP_H__
#define __EIP_IN_SELECT_IP_H__

#include "jd_bfrt.h"

extern void eip_in_select_redirect_ip_table_setup();
extern int eip_in_select_redirect_ip_entry_add(uint16_t vxlan_valid,
                                      uint16_t selector_group_id);
#endif

