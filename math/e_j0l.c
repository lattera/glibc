#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <math_private.h>

long double
__ieee754_j0l (long double x)
{
  fputs ("__ieee754_j0l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (j0l)

long double
__ieee754_y0l (long double x)
{
  fputs ("__ieee754_y0l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (y0l)
#include <stub-tag.h>
