#include <inet/netinet/in.h>

extern const struct in6_addr in6addr_any_internal attribute_hidden;

/* Bind socket to a privileged IP port.  */
extern int bindresvport_internal (int __sockfd,
				  struct sockaddr_in *__sock_in) attribute_hidden;
