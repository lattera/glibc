#include <errno.h>
#include <sys/mman.h>

int
posix_madvise (void *addr, size_t len, int advise)
{
  return madvise (addr, len, advise) ? errno : 0;
}
