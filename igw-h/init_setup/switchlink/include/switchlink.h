/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       hekuangsheng@163.com
* 
***********************************************************************/

#ifndef __SWITCHLINK_H__
#define __SWITCHLINK_H__

#include <stdlib.h>
#include <netlink/netlink.h>
#include <arpa/inet.h>
#include <syslog.h>
#include <stdint.h>

typedef struct switchlink_ip_addr_ {
  uint32_t ip;
  uint8_t  prefix_len;
} switchlink_ip_addr_t;

#define NETL_POLL_TIMEOUT 1000

static inline __u32 nl_mgrp(__u32 group)
{
	return group ? (1 << (group - 1)) : 0;
}

static inline __u32 rta_getattr_u32(struct rtattr *rta)
{
	return *(__u32 *) RTA_DATA(rta);
}
	  
static inline __u16 rta_getattr_u16(struct rtattr *rta)
{
	return *(__u16 *) RTA_DATA(rta);
}
	  
static inline __u8 rta_getattr_u8(struct rtattr *rta)
{
	return *(__u8 *) RTA_DATA(rta);
}
	  
static inline char *rta_getattr_str(struct rtattr *rta)
{
	return (char *) RTA_DATA(rta);
}

static inline int parse_rtattr_flags(struct rtattr *tb[], int max,
									struct rtattr *rta, int len,
									unsigned short flags)
{
	unsigned short type;
	  
	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		type = rta->rta_type & ~flags;
		if ((type <= max) && (!tb[type]))
			tb[type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);

	return 0;
}

#define parse_rtattr_nested(tb, max, rta) \
        (parse_rtattr_flags((tb), (max), RTA_DATA(rta), RTA_PAYLOAD(rta), 0))

#endif
