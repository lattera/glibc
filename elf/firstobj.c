#include <errno.h>

int
foo (void)
{
  errno = 0;
  return 0;
}
