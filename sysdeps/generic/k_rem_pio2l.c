#include <math.h>
#include <math_private.h>
#include <stdio.h>
#include <errno.h>

int
__kernel_rem_pio2l (long double *x, long double *y, int e0, int nx, int prec,
		    const int *ipio2)
{
  fputs ("__kernel_rem_pio2l not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (__kernel_rem_pio2l)
#include <stub-tag.h>
