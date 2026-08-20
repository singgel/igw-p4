/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __JD_BFRT_H__
#define __JD_BFRT_H__

#ifdef __cplusplus
extern "C" {
#endif
#include <bf_rt/bf_rt_init.h>
#include <bf_rt/bf_rt_session.h>
#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_table_key.h>
#include <bf_rt/bf_rt_table_data.h>
#include <bf_rt/bf_rt_table.h>
#include <bf_rt/bf_rt_table_attributes.h>
#if defined (USE_SDE_9_7)
	#include <bf_rt/bf_rt_info.h>
#endif
#include <getopt.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include <bfutils/clish/thread.h>
#include <bf_switchd/bf_switchd.h>
#include <bfsys/bf_sal/bf_sys_intf.h>
#ifdef __cplusplus
}
#endif

#define ALL_PIPES 0xffff

typedef struct jd_bf_rt {
	const bf_rt_info_hdl *bfrtInfo;
	bf_rt_session_hdl *session;
	bf_rt_target_t dev_tgt;
} jd_bf_rt_t;

extern jd_bf_rt_t jd_bfrt;

extern void bf_rt_setup(const char *p4_name);
extern void bf_rt_teardown();

#endif
