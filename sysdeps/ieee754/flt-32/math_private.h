#ifndef _MATH_PRIVATE_H_

#include_next <math_private.h>

#ifndef __isnanf
extern __always_inline int
__isnanf (float d)
{
  u_int32_t di;
  GET_FLOAT_WORD (di, d);
  return (di & 0x7fffffff) > 0x7f800000;
}
#endif

#ifndef __isinf_nsf
extern __always_inline int
__isinf_nsf (float d)
{
  u_int32_t di;
  GET_FLOAT_WORD (di, d);
  return (di & 0x7fffffff) == 0x7f800000;
}
#endif

#ifndef __finitef
extern __always_inline int
__finitef (float d)
{
  u_int32_t di;
  GET_FLOAT_WORD (di, d);
  return (di & 0x7fffffff) < 0x7f800000;
}
#endif

#endif /* _MATH_PRIVATE_H_ */
