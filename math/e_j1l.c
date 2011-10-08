#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <math_private.h>

long double
__ieee754_j1l (long double x)
{
  fputs ("__ieee754_j1l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (j1l)

long double
__ieee754_y1l (long double x)
{
  fputs ("__ieee754_y1l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (y1l)
#include <stub-tag.h>
