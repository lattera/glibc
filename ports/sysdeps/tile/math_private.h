#ifndef _MATH_PRIVATE_H

#include_next <math_private.h>

/* We have no exception support, so feraiseexcept() must be a no-op.
   And since we don't define FE_INVALID, FE_DIVBYZERO, etc., we
   must ignore the argument of feraiseexcept() as well.  we return
   "1" to indicate we failed to raise an exception, though none of
   the callers in glibc actually care.  The extra level of statement
   expression wrapping avoids "statement with no effect" warnings.  */
#define feraiseexcept(excepts) ({ 1; })

#endif
