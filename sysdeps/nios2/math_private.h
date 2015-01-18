#ifndef _MATH_PRIVATE_H

/* Suppress use of exceptions here to avoid build errors if the FE_*
   macros aren't defined. Only allow rounding modes implemented for Nios II.

   This does mean that some code will silently fail to report exceptions,
   set rounding mode as expected, etc., but it allows math code to compile
   that otherwise wouldn't (such as math/s_fma.c) and so is valuable.

   We intentionally ignore the "exception" arguments of functions that
   take an exception, since we can't even evaluate the argument
   without causing a build failure.  The extra level of statement
   expression wrapping avoids "statement with no effect" warnings.
   Since the callers don't check for errors anyway, we just claim
   success in every case.

   The overrides for libc_ functions must happen before we include
   the generic math_private.h, and the overrides for regular
   <fenv.h> functions must happen afterwards, to avoid clashing with
   the declarations of those functions.  */

#define libc_fesetround(rnd)                   ({ 0; })
#define libc_fetestexcept(exc)                 ({ 0; })
#define libc_feholdexcept_setround(env, exc)   ({ (void) (env); 0; })
#define libc_feupdateenv_test(env, exc)        ({ (void) (env); 0; })

#include_next <math_private.h>

#define feraiseexcept(excepts)                 ({ 0; })
#define __feraiseexcept(excepts)               ({ 0; })
#define feclearexcept(exc)                     ({ 0; })

#endif
