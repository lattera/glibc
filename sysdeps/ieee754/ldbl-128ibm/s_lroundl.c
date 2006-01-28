/* FIXME */
#include <math.h>
#include <math_ldbl_opt.h>

long int __lroundl (long double d)
{
  return llroundl (d);
}
long_double_symbol (libm, __lroundl, lroundl);
