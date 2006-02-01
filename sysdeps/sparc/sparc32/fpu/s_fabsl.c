#include <math.h>
#include <math_ldbl_opt.h>

long double __fabsl (long double x)
{
  return __builtin_fabsl (x);
}
long_double_symbol (libm, __fabsl, fabsl);
