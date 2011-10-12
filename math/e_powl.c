#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_powl (long double x, long double y)
{
  fputs ("__ieee754_powl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_powl, __powl_finite)

stub_warning (powl)
#include <stub-tag.h>
