/*
 * __isinf_nsl(x) returns != 0 if x is ±inf, else 0;
 * no branching!
 * slightly dodgy in relying on signed shift right copying sign bit
 */

#include <math.h>
#include <math_private.h>

int
__isinf_nsl (long double x)
{
  double xhi;
  int64_t hx, mask;

  xhi = ldbl_high (x);
  EXTRACT_WORDS64 (hx, xhi);

  mask = (hx & 0x7fffffffffffffffLL) ^ 0x7ff0000000000000LL;
  mask |= -mask;
  mask >>= 63;
  return ~mask;
}
