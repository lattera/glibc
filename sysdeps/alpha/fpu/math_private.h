#ifndef ALPHA_MATH_PRIVATE_H
#define ALPHA_MATH_PRIVATE_H 1

/* In bits/mathinline.h we define __isnan et al.
   In sysdeps/alpha/fpu/s_isnan.c we move the identifier out of the way
   via macro hackery.  In both cases, tell math/math_private.h that
   we have a local copy of the function.  */

#ifndef __isnan
# define __isnan  __isnan
#endif
#ifndef __isnanf
# define __isnanf __isnanf
#endif

#include_next <math_private.h>

#ifdef __alpha_fix__
extern __always_inline double
__ieee754_sqrt (double d)
{
  double ret;
# ifdef _IEEE_FP_INEXACT
  asm ("sqrtt/suid %1,%0" : "=f"(ret) : "f"(d));
# else
  asm ("sqrtt/sud %1,%0" : "=f"(ret) : "f"(d));
# endif
  return ret;
}

extern __always_inline float
__ieee754_sqrtf (float d)
{
  float ret;
# ifdef _IEEE_FP_INEXACT
  asm ("sqrts/suid %1,%0" : "=f"(ret) : "f"(d));
# else
  asm ("sqrts/sud %1,%0" : "=f"(ret) : "f"(d));
# endif
  return ret;
}
#endif /* FIX */

#endif /* ALPHA_MATH_PRIVATE_H */
