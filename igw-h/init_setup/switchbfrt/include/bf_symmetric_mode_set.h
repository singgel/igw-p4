/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _BF_SYMMETRIC_SET_H__
#define _BF_SYMMETRIC_SET_H__

#include "jd_bfrt.h"

extern void symmetric_table_setup(const char *table_name, 
	const bf_rt_table_hdl **bf_rt_table_hdl_ret,
	bf_rt_table_attributes_hdl **tbl_attr_hdl_ret);

extern void symmetric_mode_set(const bool symmetric_mode,
	const bf_rt_table_hdl *bf_rt_table_hdl,
	bf_rt_table_attributes_hdl *tbl_attr_hdl);
#endif
