#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__asinhl(long double x)
{
  fputs ("__asinhl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

weak_alias (__asinhl, asinhl)
stub_warning (asinhl)
