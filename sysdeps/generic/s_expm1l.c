#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__expm1l (long double x)
{
  fputs ("__expm1l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
libm_hidden_def (__expm1l)
weak_alias (__expm1l, expm1l)

stub_warning (expm1l)
#include <stub-tag.h>
