#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__exp2l (long double x)
{
  fputs ("__exp2l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}
weak_alias (__exp2l, exp2l)

stub_warning (exp2l)
