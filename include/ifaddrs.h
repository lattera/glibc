#ifndef _IFADDRS_H
#include <inet/ifaddrs.h>
#include <stdbool.h>

libc_hidden_proto (getifaddrs)
libc_hidden_proto (freeifaddrs)

extern void __check_pf (bool *seen_ipv4, bool *seen_ipv6) attribute_hidden;

#endif	/* ifaddrs.h */
