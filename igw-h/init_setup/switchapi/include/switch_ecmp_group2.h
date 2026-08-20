/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _SWITCH_ECMP_GROUP2_H__
#define _SWITCH_ECMP_GROUP2_H__

#define PIPE0_SELECTORCID 1
#define PIPE1_SELECTORCID 2
#define PIPE2_SELECTORCID 3
#define PIPE3_SELECTORCID 4

#define PER_PIPE_PORT_NUMS  16

extern uint32_t pipe0_memberid_val[PER_PIPE_PORT_NUMS];
extern uint32_t pipe2_memberid_val[PER_PIPE_PORT_NUMS];

extern uint32_t pipe0_devport_val[PER_PIPE_PORT_NUMS];
extern uint32_t pipe2_devport_val[PER_PIPE_PORT_NUMS];

extern void ecmp_group02_init(void);

#endif

