#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_lgammal_r (long double x, int *signgamp)
{
  fputs ("__ieee754_lgammal_r not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (__ieee754_lgammal_r)
