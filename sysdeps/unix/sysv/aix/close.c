/* This is a system call.  We only have to provide the wrapper.  */
#include <unistd.h>

int
__close (int fd)
{
  return close (fd);
}
