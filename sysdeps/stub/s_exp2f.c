#include <math.h>
#include <stdio.h>

float
__exp2f (float x)
{
  fputs ("__exp2f not implemented\n", stderr);
  return 0.0;
}
weak_alias (__exp2f, exp2f)

stub_warning (exp2)
