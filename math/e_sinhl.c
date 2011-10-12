#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_sinhl (long double x)
{
  fputs ("__ieee754_sinhl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_sinhl, __sinhl_finite)

stub_warning (__ieee754_sinhl)
#include <stub-tag.h>
