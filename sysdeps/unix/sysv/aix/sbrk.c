/* This is a system call.  We only have to provide the wrapper.  */
#include <unistd.h>

void *
__sbrk (ptrdiff_t delta)
{
  return sbrk (delta);
}
