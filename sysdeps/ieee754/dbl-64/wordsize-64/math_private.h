#ifndef _MATH_PRIVATE_H_

#include_next <math_private.h>
#include <stdint.h>

#ifndef __isnan
extern __always_inline int
__isnan (double d)
{
  uint64_t di;
  EXTRACT_WORDS64 (di, d);
  return (di & 0x7fffffffffffffffull) > 0x7ff0000000000000ull;
}
#endif

#ifndef __isinf_ns
extern __always_inline int
__isinf_ns (double d)
{
  uint64_t di;
  EXTRACT_WORDS64 (di, d);
  return (di & 0x7fffffffffffffffull) == 0x7ff0000000000000ull;
}
#endif

#ifndef __finite
extern __always_inline int
__finite (double d)
{
  uint64_t di;
  EXTRACT_WORDS64 (di, d);
  return (di & 0x7fffffffffffffffull) < 0x7ff0000000000000ull;
}
#endif

#endif /* _MATH_PRIVATE_H_ */
