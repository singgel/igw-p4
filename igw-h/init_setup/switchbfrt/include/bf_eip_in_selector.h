/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __EIP_IN_SELECTOR_H__
#define __EIP_IN_SELECTOR_H__

#include "jd_bfrt.h"

#define EIP_IN_SELECTOR_GROUP_ID 1

typedef struct eipInSelector_Key {
  uint16_t selector_groupid;
} eipInSelectorKey;

extern void eip_in_selector_table_setup();
extern int eip_in_selector_entry_add(const eipInSelectorKey *key,
                                        uint16_t max_group_size,
                                        uint32_t *memberid_val,uint16_t memberid_num,
										bool *memberstatus_val,uint16_t memberstatus_num,
										const bool add);
#endif

