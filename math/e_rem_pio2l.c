#include <math.h>
#include <stdio.h>
#include <errno.h>
#include <math_private.h>

int
__ieee754_rem_pio2l (long double x, long double *y)
{
  fputs ("__ieee754_rem_pio2l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0;
}

stub_warning (__ieee754_rem_pio2l)
