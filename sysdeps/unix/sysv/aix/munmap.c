/* This is a system call.  We only have to provide the wrapper.  */
#include <sys/mman.h>

int
__munmap (void *addr, size_t len)
{
  return munmap (addr, len);
}
