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

#endif /* ALPHA_MATH_PRIVATE_H */
