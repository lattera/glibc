/* This is a system call.  We only have to provide the wrapper.  */
#include <sys/socket.h>

#undef __connect

int
__connect (int fd, __CONST_SOCKADDR_ARG addr, socklen_t len)
{
  return connect (fd, addr, len);
}
INTDEF(__connect)
