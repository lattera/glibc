#include <math.h>
#include <stdio.h>

long double
__erfl (long double x)
{
  fputs ("__erfl not implemented\n", stderr);
  return 0.0;
}
weak_alias (__erfl, erfl)

stub_warning (erfl)

long double
__erfcl (long double x)
{
  fputs ("__erfcl not implemented\n", stderr);
  return 0.0;
}
weak_alias (__erfcl, erfcl)

stub_warning (erfcl)
