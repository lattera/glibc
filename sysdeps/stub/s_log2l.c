#include <math.h>
#include <stdio.h>

long double
__log2l (long double x)
{
  fputs ("__log2l not implemented\n", stderr);
  return 0.0;
}
weak_alias (__log2l, log2l)

stub_warning (log2l)
