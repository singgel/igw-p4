/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#ifndef __SWITCHLINK_ROUTE_H__
#define __SWITCHLINK_ROUTE_H__

#include <stdint.h>
#include "init_setup_list.h"

extern void process_route_msg(struct nlmsghdr *nlmsg, int type);
extern void route_system_init(void);

#endif /* __SWITCHLINK_INT_H__ */

