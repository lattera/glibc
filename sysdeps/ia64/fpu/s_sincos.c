#include <math.h>

void
__sincos (double x, double *s, double *c)
{
  *s = sin (x);
  *c = cos (x);
}
weak_alias (__sincos, sincos)
