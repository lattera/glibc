#include <math.h>
#include <stdio.h>

double
__exp2 (double x)
{
  fputs ("__exp2 not implemented\n", stderr);
  return 0.0;
}
weak_alias (__exp2, exp2)

stub_warning (exp2)
