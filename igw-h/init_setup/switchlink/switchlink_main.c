/**********************************************************************
* 
* Copyright (c) 2022 jd.com, Inc. All Rights Reserved
* @author       liweiwei118@jd.com
* 
***********************************************************************/

#define _GNU_SOURCE

#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/sysinfo.h>
#include <unistd.h>
#include <sys/select.h>
#include <semaphore.h>
#include <errno.h>
#include "utils.h"

#include "switchlink.h"
#include "switchlink_neigh.h"
#include "switchlink_route.h"

static pthread_t netlink_thread;
static int g_nl_fd = -1;
sem_t netlink_sem;

static void process_netlink_message(struct nlmsghdr *nlmsg) {
  switch (nlmsg->nlmsg_type) {
    case RTM_NEWNEIGH:
    case RTM_DELNEIGH:		
      	process_neigh_msg(nlmsg, nlmsg->nlmsg_type);
      	break;
	case RTM_NEWROUTE:
    case RTM_DELROUTE:		
      	process_route_msg(nlmsg, nlmsg->nlmsg_type);
      	break;
    default:
      break;
  }
}

static void process_netlink_loop() {
	int len, err;
	int res;
	char logmsg[256];
	int msg_count;
	ssize_t status;
	struct nlmsghdr *hdr;
	struct sockaddr_nl nladdr;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char buf[8192];
	struct pollfd fds[1];

	iov.iov_base = buf;	
	fds[0].events = POLLIN;
	fds[0].fd = g_nl_fd;

	while (1) {
		res = poll(fds, 1, NETL_POLL_TIMEOUT);
		if (res < 0 && errno != EINTR) {
			continue;
		}	
		
		if (fds[0].revents & POLLIN) {
			iov.iov_len = sizeof(buf);
			status = recvmsg(g_nl_fd, &msg, 0);
			if (status < 0) {
				continue;
			}
			
			if (status == 0) {
				goto abort;
			}
			
			if (msg.msg_namelen != sizeof(nladdr)) {
				continue;
			}

			if (iov.iov_len < ((size_t) status) || (msg.msg_flags & MSG_TRUNC)) {
				continue;
			}

			msg_count = 0;
			for (hdr = (struct nlmsghdr *) buf;
				 (size_t) status >= sizeof(*hdr);) {
				len = hdr->nlmsg_len;
				process_netlink_message(hdr);
				msg_count++;
				status -= NLMSG_ALIGN(len);
				hdr = (struct nlmsghdr *) ((char *) hdr + NLMSG_ALIGN(len));
			}
				 
			if (status) {
				continue;
			}
		}
	
	}

abort:
	return ;
}

static void switchlink_sock_init() {	
	int fd;
	int rcvbuf = 1024 * 1024 * 1024;
	socklen_t addr_len;	
	struct sockaddr_nl local;
	unsigned subscriptions = 0;

	subscriptions |= nl_mgrp(RTNLGRP_NEIGH);
	subscriptions |= nl_mgrp(RTNLGRP_IPV4_ROUTE);
	
	fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	if (fd < 0) {
		perror("Cannot open netlink socket");
		return ;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &rcvbuf, sizeof(rcvbuf)) < 0) {
		perror("Cannot set RCVBUF");		
		goto err;
	}
	
	memset(&local, 0, sizeof(local));
	local.nl_family = AF_NETLINK;
	local.nl_groups = subscriptions;

	if (bind(fd, (struct sockaddr *) &(local), sizeof(local)) < 0) {
		perror("Cannot bind netlink socket");		
		goto err;
	}
	
	addr_len = sizeof(local);
	if (getsockname(fd, (struct sockaddr *) &local, &addr_len) < 0) {
		perror("Cannot getsockname");
		goto err;
	}
	
	
   if (addr_len != sizeof(local)) {
		perror("Wrong address length");
		goto err;
	}

	if (local.nl_family != AF_NETLINK) {
		perror("Wrong address family");
		goto err;
	}

	g_nl_fd = fd;
	return;
	
err:	
	close(fd);
	return ;
}

static void *switchlink_main(void *args) {
	neigh_system_init();
	route_system_init();
	sem_post(&netlink_sem);
	
	while (1) {		
		g_nl_fd = -1;
		switchlink_sock_init();
		if (g_nl_fd >= 0) {
			process_netlink_loop();				
			close(g_nl_fd);				
		} else {
			usleep(1000);
		}
	}
	
  	return NULL;
}

int switchlink_init() {
	if (sem_init(&netlink_sem, 0, 0) < 0) 		
		SETUP_PANIC("netlink_sem intitialization failed\n");
		
  	if (pthread_create(&netlink_thread, NULL, switchlink_main, NULL) !=0 )
		SETUP_PANIC("switchlink_main thread create fail!\n");
}

