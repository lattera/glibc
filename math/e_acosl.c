#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_acosl (long double x)
{
  fputs ("__ieee754_acosl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_acosl, __acosl_finite)

stub_warning (acosl)
