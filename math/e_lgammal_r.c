#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <math_private.h>

long double
__ieee754_lgammal_r (long double x, int *signgamp)
{
  *signgamp = 0;
  fputs ("__ieee754_lgammal_r not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (lgammal)
stub_warning (lgammal_r)
#include <stub-tag.h>
