/* FIXME */
#include <math.h>
#include <math_ldbl_opt.h>

long int __lrintl (long double d)
{
  return llrintl (d);
}
long_double_symbol (libm, __lrintl, lrintl);
