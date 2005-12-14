#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_gammal_r (long double x, int *signgamp)
{
  *signgamp = 0;
  fputs ("__ieee754_gammal_r not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (__ieee754_gammal_r)
#include <stub-tag.h>
