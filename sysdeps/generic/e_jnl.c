#include <math.h>
#include <stdio.h>
#include <errno.h>
#include "math_private.h"

long double
__ieee754_jnl (int n, long double x)
{
  fputs ("__ieee754_jnl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (jnl)

long double
__ieee754_ynl (int n, long double x)
{
  fputs ("__ieee754_ynl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (ynl)
#include <stub-tag.h>
