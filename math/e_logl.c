#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_logl (long double x)
{
  fputs ("__ieee754_logl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (logl)
#include <stub-tag.h>
