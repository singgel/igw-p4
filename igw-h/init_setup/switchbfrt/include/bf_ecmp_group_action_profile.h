/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __ECMP_GROUP_ACTION_PROFILE_TABLE_H__
#define __ECMP_GROUP_ACTION_PROFILE_TABLE_H__

#include "jd_bfrt.h"

typedef struct ecmpGroupActionProfile_Key {
  uint16_t memberid;
} ecmpGroupActionProfileKey;

typedef struct ecmpGroupActionPfofile_Data {
  uint16_t egress_port;
} ecmpGroupActionPfofileData;

extern void ecmp_group_action_profile_table_setup();
extern int  ecmp_group_action_profile_entry_add(ecmpGroupActionProfileKey *key,
                                         ecmpGroupActionPfofileData *data);
#endif

