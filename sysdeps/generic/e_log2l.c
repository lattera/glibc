#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_log2l (long double x)
{
  fputs ("__ieee754_log2l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (log2l)
#include <stub-tag.h>
