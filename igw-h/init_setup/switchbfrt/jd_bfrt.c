/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifdef __cplusplus
extern "C" {
#endif
#include <bf_rt/bf_rt_init.h>
#include <bf_rt/bf_rt_session.h>
#include <bf_rt/bf_rt_common.h>
#include <bf_rt/bf_rt_table_key.h>
#include <bf_rt/bf_rt_table_data.h>
#include <bf_rt/bf_rt_table.h>
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

#include "jd_bfrt.h"

jd_bf_rt_t jd_bfrt;

// This function does the initial setUp of getting bfrtInfo object associated
// with the P4 program from which all other required objects are obtained
void bf_rt_setup(const char *p4_name) {
  const bf_rt_info_hdl *bfrtInfo = NULL;

  jd_bfrt.dev_tgt.dev_id = 0;
  jd_bfrt.dev_tgt.pipe_id = ALL_PIPES;

  // Get bfrtInfo object from dev_id and p4 program name
  bf_status_t bf_status = bf_rt_info_get(jd_bfrt.dev_tgt.dev_id, p4_name, &bfrtInfo);
  assert(bf_status == BF_SUCCESS);
  jd_bfrt.bfrtInfo = bfrtInfo;

  // Create a session object
  bf_status = bf_rt_session_create(&(jd_bfrt.session));
  assert(bf_status == BF_SUCCESS);

}

// This function clears up any allocated mem during setUp()
void bf_rt_teardown() {
  bf_status_t bf_status;
  bf_status = bf_rt_session_destroy(jd_bfrt.session);
  assert(bf_status == BF_SUCCESS);
}
