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

/* Put the address of the peer connected to socket FD into *ADDR
   (which is *LEN bytes long), and its actual length into *LEN.  */
extern int __getpeername (int __fd, __SOCKADDR_ARG __addr, socklen_t *__len);

/* Send N bytes of BUF to socket FD.  Returns the number sent or -1.  */
extern ssize_t __send (int __fd, __const void *__buf, size_t __n, int __flags);

/* Open a connection on socket FD to peer at ADDR (which LEN bytes long).
   For connectionless socket types, just set the default address to send to
   and the only address from which to accept transmissions.
   Return 0 on success, -1 for errors.  */
extern int __connect (int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len);
extern int __connect_internal (int __fd, __CONST_SOCKADDR_ARG __addr,
			       socklen_t __len) attribute_hidden;

/* Return the length of a `sockaddr' structure.  */
#ifdef _HAVE_SA_LEN
# define SA_LEN(_x)      (_x)->sa_len
#else
extern int __libc_sa_len (sa_family_t __af);
extern int __libc_sa_len_internal (sa_family_t __af) attribute_hidden;
# ifndef NOT_IN_libc
#  define SA_LEN(_x)      INTUSE(__libc_sa_len)((_x)->sa_family)
# else
#  define SA_LEN(_x)      __libc_sa_len((_x)->sa_family)
# endif
#endif


#ifndef NOT_IN_libc
# define __connect(fd, addr, len) INTUSE(__connect) (fd, addr, len)
#endif

#endif
