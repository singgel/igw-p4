#ifndef _INIT_SETUP_UTILS_H_
#define _INIT_SETUP_UTILS_H_

#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <syslog.h>

#define CONST_UINT8_MAX 255

#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(x) ((uint8_t*)(x))[0],((uint8_t*)(x))[1],((uint8_t*)(x))[2],((uint8_t*)(x))[3],((uint8_t*)(x))[4],((uint8_t*)(x))[5]

static inline int ether_aton(const char *a, uint8_t mac_addr[6])
{
	int i;
	char *end;
	unsigned long o[6];

	i = 0;
	do {
		errno = 0;
		o[i] = strtoul(a, &end, 16);
		if (errno != 0 || end == a || (end[0] != ':' && end[0] != 0))
			return -1;
		a = end + 1;
	} while (++i != sizeof(o) / sizeof(o[0]) && end[0] != 0);

	/* Support the format XX:XX:XX:XX:XX:XX */
	if (i == 6) {
		while (i-- != 0) {
			if (o[i] > CONST_UINT8_MAX)
				return -1;
			mac_addr[i] = (uint8_t)o[i];
		}
	}

	return 0;
}

#define CONST_MAX_MASK_LEN 32

static inline uint32_t get_mask_by_prefix_len(int prefix_len)
{
	uint32_t mask = 0;
	int i = 1;
	for (; i <= prefix_len; ++i) {
		mask |= 1<<(CONST_MAX_MASK_LEN - i);
	}
	return mask;
}

static inline uint32_t ip_atoi(const char *str)
{
	uint32_t ipaddr = 0;

	if(inet_pton(AF_INET, str, &ipaddr) != 1) {
		return 0;
	}
	return ntohl(ipaddr);
}

static inline int ip6_atoi(const char *str, struct in6_addr *ip6_addr)
{

	if (inet_pton(AF_INET6, str, ip6_addr) != 1) {
		return -1;
	}
	return 0;
}

static inline void ipv6_addr_set(struct in6_addr *addr,
                                 uint32_t w1, uint32_t w2,
                                 uint32_t w3, uint32_t w4)
{
        addr->s6_addr32[0] = w1;
        addr->s6_addr32[1] = w2;
        addr->s6_addr32[2] = w3;
        addr->s6_addr32[3] = w4;
}

static inline void ipv6_addr_solict_mult_set(struct in6_addr *addr,
                                                    struct in6_addr *solicited)
{
        ipv6_addr_set(solicited,
                      htonl(0xFF020000), 0,
                      htonl(0x1),
                      htonl(0xFF000000) | addr->s6_addr32[3]);
}

static inline void ip6_get_linklocal(uint8_t *mult_eth, struct in6_addr *ll_addr)
{
	ll_addr->s6_addr32[0] = htonl(0xFE800000);
	ll_addr->s6_addr32[1] = htonl(0x0);
	ll_addr->s6_addr[8] = mult_eth[0] ^ 0x02;
	ll_addr->s6_addr[9] = mult_eth[1];
	ll_addr->s6_addr[10] = mult_eth[2];
	ll_addr->s6_addr[11] = 0xFF;
	ll_addr->s6_addr[12] = 0xFE;
	ll_addr->s6_addr[13] = mult_eth[3];
	ll_addr->s6_addr[14] = mult_eth[4];
	ll_addr->s6_addr[15] = mult_eth[5];
}

static inline void reverse_mac(char *rmac, char *mac) {
	rmac[0] = mac[5];	
	rmac[1] = mac[4];
	rmac[2] = mac[3];
	rmac[3] = mac[2];
	rmac[4] = mac[1];
	rmac[5] = mac[0];
}

#define SETUP_PANIC(msg...)\
	do{\
		fprintf(stderr, "PANIC: %s: %d: ", __FILE__, __LINE__);\
		fprintf(stderr, msg);\
		abort(); \
	}while(0)

#define SETUP_LOG(msg...)                                              \
	do{ 																	\
		syslog(6, msg); 													\
	}while(0)

static inline int is_multicast_ether_addr(const uint8_t *addr)
{
	return 0x01 & addr[0];
}

static inline int is_zero_ether_addr(const uint8_t *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

static inline int is_valid_ether_addr(const uint8_t *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
		* explicitly check for it here. */
	return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

#endif
