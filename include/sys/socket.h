#ifndef _SYS_SOCKET_H
#include <socket/sys/socket.h>

/* Now define the internal interfaces.  */
extern int __socket (int __domain, int __type, int __protocol);

/* Create two new sockets, of type TYPE in domain DOMAIN and using
   protocol PROTOCOL, which are connected to each other, and put file
   descriptors for them in FDS[0] and FDS[1].  If PROTOCOL is zero,
   one will be chosen automatically.  Returns 0 on success, -1 for errors.  */
extern int __socketpair (int __domain, int __type, int __protocol,
			 int __fds[2]);

/* Return a socket of any type.  The socket can be used in subsequent
   ioctl calls to talk to the kernel.  */
extern int __opensock (void) internal_function;

#endif
