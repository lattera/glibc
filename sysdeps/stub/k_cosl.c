#include <math.h>
#include <stdio.h>

long double
__kernel_cosl (long double x, long double y)
{
  fputs ("__kernel_cosl not implemented\n", stderr);
  return 0.0;
}

stub_warning (__kernel_cosl)
