#include <math.h>
#include <stdio.h>

long double
__kernel_sinl (long double x, long double y)
{
  fputs ("__kernel_sinl not implemented\n", stderr);
  return 0.0;
}

stub_warning (__kernel_sinl)
