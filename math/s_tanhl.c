#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__tanhl(long double x)
{
  fputs ("__tanhl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

weak_alias (__tanhl, tanhl)
stub_warning (tanhl)
