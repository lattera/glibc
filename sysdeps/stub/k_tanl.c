#include <math.h>
#include <stdio.h>

long double
__kernel_tanl (long double x, long double y, int iy)
{
  fputs ("__kernel_tanl not implemented\n", stderr);
  return 0.0;
}

stub_warning (__kernel_tanl)
