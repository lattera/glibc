/* Disable use of exceptions and rounding modes for no-FPU ColdFire.
   Copyright (C) 2012-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef COLDFIRE_NOFPU_MATH_PRIVATE_H
#define COLDFIRE_NOFPU_MATH_PRIVATE_H 1

/* Suppress use of exceptions and rounding modes here to avoid build
   errors if the FE_* macros aren't defined.

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

/* Enable __finitel, __isinfl, and __isnanl for binary compatibility
   when built without long double support. */
#define LDBL_CLASSIFY_COMPAT 1

#include_next <math_private.h>

#define feraiseexcept(excepts)                 ({ 0; })
#define __feraiseexcept(excepts)               ({ 0; })
#define feclearexcept(exc)                     ({ 0; })

#endif
