/* This is a system call.  We only have to provide the wrapper.  */
#include <unistd.h>

int
__readlink (const char *path, char *buf, size_t len)
{
  return readlink (path, buf, len);
}
