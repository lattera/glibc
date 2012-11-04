#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <math_private.h>

long double
__kernel_sinl (long double x, long double y, int iy)
{
  fputs ("__kernel_sinl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (__kernel_sinl)
