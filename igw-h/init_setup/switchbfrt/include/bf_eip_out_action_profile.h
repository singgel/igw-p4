/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __EIP_OUT_ACTION_PROFILE_TABLE_H__
#define __EIP_OUT_ACTION_PROFILE_TABLE_H__

#include "jd_bfrt.h"

typedef struct eipOutActionProfile_Key {
  uint16_t memberid;
} eipOutActionProfileKey;

typedef struct eipOutActionPfofile_Data {
  uint32_t dl_ip;
} eipOutActionPfofileData;

extern void eip_out_action_profile_table_setup();
extern int  eip_out_action_profile_entry_add(const eipOutActionProfileKey *key,
                                         const eipOutActionPfofileData *data);
#endif

