#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_asinl (long double x)
{
  fputs ("__ieee754_asinl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_asinl, __asinl_finite)

stub_warning (asinl)
