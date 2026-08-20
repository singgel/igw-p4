/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __EIP_IN_ACTION_PROFILE_TABLE_H__
#define __EIP_IN_ACTION_PROFILE_TABLE_H__

#include "jd_bfrt.h"

typedef struct eipInActionProfile_Key {
  uint16_t memberid;
} eipInActionProfileKey;

typedef struct eipInActionPfofile_Data {
  uint32_t dl_ip;
} eipInActionPfofileData;

extern void eip_in_action_profile_table_setup();
extern int  eip_in_action_profile_entry_add(const eipInActionProfileKey *key,
                                         const eipInActionPfofileData *data);
#endif
