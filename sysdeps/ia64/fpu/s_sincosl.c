#include <math.h>

void
__sincosl (long double x, long double *s, long double *c)
{
  *s = sinl (x);
  *c = cosl (x);
}
weak_alias (__sincosl, sincosl)
