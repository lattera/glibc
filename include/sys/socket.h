#ifndef _SYS_SOCKET_H
#include <socket/sys/socket.h>

/* Now define the internal interfaces.  */
extern int __socket (int __domain, int __type, int __protocol) __THROW;

/* Return a socket of any type.  The socket can be used in subsequent
   ioctl calls to talk to the kernel.  */
extern int __opensock (void) internal_function;

#endif
