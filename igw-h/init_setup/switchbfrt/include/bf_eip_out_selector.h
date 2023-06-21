/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __EIP_OUT_SELECTOR_H__
#define __EIP_OUT_SELECTOR_H__

#include "jd_bfrt.h"

#define EIP_OUT_SELECTOR_GROUP_ID 1

typedef struct eipOutSelector_Key {
  uint16_t selector_groupid;
} eipOutSelectorKey;

extern void eip_out_selector_table_setup();
extern int eip_out_selector_entry_add(const eipOutSelectorKey *key,
                                        uint16_t max_group_size,
                                        uint32_t *memberid_val,uint16_t memberid_num,
										bool *memberstatus_val,uint16_t memberstatus_num,
										const bool add);
#endif

