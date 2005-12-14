#include <math.h>
#include <stdio.h>
#include <errno.h>

long double
__ieee754_hypotl (long double x, long double y)
{
  fputs ("__ieee754_hypotl not implemented\n", stderr);
  __set_errno (ENOSYS);
  return 0.0;
}

stub_warning (__ieee754_hypotl)
#include <stub-tag.h>
