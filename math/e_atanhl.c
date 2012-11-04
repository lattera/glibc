#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_atanhl (long double x)
{
  fputs ("__ieee754_atanhl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_atanhl, __atanhl_finite)

stub_warning (__ieee754_atanhl)
