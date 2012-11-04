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
strong_alias (__ieee754_lgammal_r, __lgammal_r_finite)

stub_warning (lgammal)
stub_warning (lgammal_r)
