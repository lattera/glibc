#include <inet/arpa/inet.h>

extern int __inet_aton (const char *__cp, struct in_addr *__inp);
libc_hidden_proto (__inet_aton)

libc_hidden_proto (inet_aton)
libc_hidden_proto (inet_ntop)
libc_hidden_proto (inet_pton)
libc_hidden_proto (inet_makeaddr)
libc_hidden_proto (inet_netof)
