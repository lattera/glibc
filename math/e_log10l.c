#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_log10l (long double x)
{
  fputs ("__ieee754_log10l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
strong_alias (__ieee754_log10l, __log10l_finite)

stub_warning (log10l)
#include <stub-tag.h>
