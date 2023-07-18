/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "bf_symmetric_mode_set.h"


void symmetric_table_setup(const char *table_name, 
	const bf_rt_table_hdl **bf_rt_table_hdl_ret,
	bf_rt_table_attributes_hdl **tbl_attr_hdl_ret) 
{
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;
	
 	bf_status = bf_rt_table_from_name_get(jd_bf_p->bfrtInfo, 
		table_name, bf_rt_table_hdl_ret);
  	assert(bf_status == BF_SUCCESS);

  	bf_status = bf_rt_table_entry_scope_attributes_allocate(
					*bf_rt_table_hdl_ret, 
					tbl_attr_hdl_ret);
  	assert(bf_status == BF_SUCCESS);
}

void symmetric_mode_set(const bool symmetric_mode,
	const bf_rt_table_hdl *bf_rt_table_hdl,
	bf_rt_table_attributes_hdl *tbl_attr_hdl) {
	bf_status_t bf_status;
	jd_bf_rt_t *jd_bf_p = &jd_bfrt;

	bf_status = bf_rt_attributes_entry_scope_symmetric_mode_set(
		tbl_attr_hdl, symmetric_mode);
  	assert(bf_status == BF_SUCCESS);

	 bf_status = bf_rt_table_attributes_set(
	 			bf_rt_table_hdl, 
				jd_bf_p->session, 
    			&jd_bf_p->dev_tgt, 
	#ifdef BFRT_GENERIC_FLAGS
							   0,
	#endif
				tbl_attr_hdl);
  	assert(bf_status == BF_SUCCESS);

	bf_status = bf_rt_table_attributes_deallocate(tbl_attr_hdl);
  	assert(bf_status == BF_SUCCESS);
}

