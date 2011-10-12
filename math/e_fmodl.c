#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_fmodl (long double x, long double y)
{
  fputs ("__ieee754_fmodl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_fmodl, __fmodl_finite)

stub_warning (fmodl)
#include <stub-tag.h>
