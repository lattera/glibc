#ifndef _IFADDRS_H
#include <inet/ifaddrs.h>
#include <stdbool.h>

libc_hidden_proto (getifaddrs)
libc_hidden_proto (freeifaddrs)

struct in6addrinfo
{
  enum {
    in6ai_deprecated = 1,
    in6ai_temporary = 2
  } flags;
  uint32_t addr[4];
};

extern void __check_pf (bool *seen_ipv4, bool *seen_ipv6,
			struct in6addrinfo **in6ai, size_t *in6ailen)
  attribute_hidden;

#endif	/* ifaddrs.h */
