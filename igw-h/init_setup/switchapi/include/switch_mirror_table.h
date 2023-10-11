/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef _SWITCH_MIRROR_TABLE_H__
#define _SWITCH_MIRROR_TABLE_H__

#define MIRROR_RED 		3 
#define MIRROR_GREEN 	0 
#define MIRROR_YELLOW   1 
#define MIRROR_PPS 5000

#define  CPU_MIRROR_PORT_4P  320 
#define  CPU_MIRROR_PORT_2P 192 
#define  SID_MIRROR_PORT 0 

#define  CPU_MIRROR_SESSION_ID   10
#define  SID_MIRROR_SESSION_ID   20

extern void mirror_table_init(void);

#endif

