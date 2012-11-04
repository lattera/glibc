#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_atan2l (long double x, long double y)
{
  fputs ("__ieee754_atan2l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_atan2l, __atan2l_finite)

stub_warning (atan2l)
