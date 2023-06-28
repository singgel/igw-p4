/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _SWITCH_ACL_TABLE_H__
#define _SWITCH_ACL_TABLE_H__

#define ACL_RED 3

#define ACL_HIGHEST_PRI 0
#define ACL_MIRROR_PRI 	10
#define ACL_SYSTEM_PRI 	20
#define ACL_USER_PRI 	30

extern void acl_table_init(void);

#endif

