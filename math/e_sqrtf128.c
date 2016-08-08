#include <math.h>
#include <stdio.h>
#include <errno.h>

_Float128
__ieee754_sqrtf128 (_Float128 x)
{
  fputs ("__ieee754_sqrtf128 not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_sqrtf128, __sqrtf128_finite)

stub_warning (sqrtf128)
