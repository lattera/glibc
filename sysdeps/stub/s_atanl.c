#include <math.h>
#include <stdio.h>

long double
__atanl (long double x)
{
  fputs ("__atanl not implemented\n", stderr);
  return 0.0;
}
weak_alias (__atanl, atanl)

stub_warning (atanl)
