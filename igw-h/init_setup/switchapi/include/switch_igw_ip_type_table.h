/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef _SWITCH_IGW_IP_TYPE_TABLE_H__
#define _SWITCH_IGW_IP_TYPE_TABLE_H__

#define EGR_PIPELINE_0  1
#define EGR_PIPELINE_1  2
#define EGR_PIPELINE_2  3
#define EGR_PIPELINE_3  4

#define VXLAN_TYPE_STD  1
#define VXLAN_TYPE_JD   2

#define  TOF_EIP_IN  1
#define  TOF_EIP_OUT 2
#define  TOF_AZ_IN   3
#define  TOF_AZ_OUT  4

extern void igw_ip_type_table_init(void);

#endif

