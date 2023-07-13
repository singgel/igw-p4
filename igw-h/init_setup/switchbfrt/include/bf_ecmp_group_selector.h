/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __ECMP_GROUP_SELECTOR_TABLE_H__
#define __ECMP_GROUP_SELECTOR_TABLE_H__

#include "jd_bfrt.h"

#define ECMP_GROUP_SELECTOR_GROUP_ID 1

typedef struct ecmpGroupSelector_Key {
  uint16_t selector_groupid;
} ecmpGroupSelectorKey;

extern void ecmp_group_selector_table_setup();
extern int ecmp_group_selector_entry_add(ecmpGroupSelectorKey *key,
                                        uint16_t max_group_size,
                                        uint32_t *memberid_val,uint16_t memberid_num,
										bool *memberstatus_val,uint16_t memberstatus_num
										,const bool add);
#endif

