#include <math.h>

void
__sincosf (float x, float *s, float *c)
{
  *s = sinf (x);
  *c = cosf (x);
}
weak_alias (__sincosf, sincosf)
