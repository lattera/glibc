/* This is a system call.  We only have to provide the wrapper.  */
#include <sys/mman.h>
#include <sys/types.h>

void *
__mmap (void *addr, size_t len, int prot, int flags, int fd, off_t offset)
{
  return mmap (addr, len, prot, flags, fd, offset);
}
