#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_expl (long double x)
{
  fputs ("__ieee754_expl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (expl)
#include <stub-tag.h>
