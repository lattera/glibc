#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <math_private.h>

long double
__kernel_cosl (long double x, long double y)
{
  fputs ("__kernel_cosl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (__kernel_cosl)
