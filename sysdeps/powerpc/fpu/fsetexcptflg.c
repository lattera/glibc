/* Set floating-point environment exception handling.
   Copyright (C) 1997,99,2000,01 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv_libc.h>
#include <bp-sym.h>

int
__fesetexceptflag (const fexcept_t *flagp, int excepts)
{
  fenv_union_t u;
  fexcept_t flag;

  /* Get the current state.  */
  u.fenv = fegetenv_register ();

  /* Ignore exceptions not listed in 'excepts'.  */
  flag = *flagp & excepts;

  /* Replace the exception status */
  u.l[1] = ((u.l[1] & ~(FPSCR_STICKY_BITS & excepts))
	    | (flag & FPSCR_STICKY_BITS)
	    | (flag >> ((31 - FPSCR_VX) - (31 - FPSCR_VXSOFT))
	       & FE_INVALID_SOFTWARE));

  /* Store the new status word (along with the rest of the environment).
     This may cause floating-point exceptions if the restored state
     requests it.  */
  fesetenv_register (u.fenv);

  /* Deal with FE_INVALID_SOFTWARE not being implemented on some chips.  */
  if (flag & FE_INVALID)
    feraiseexcept(FE_INVALID);

  /* Success.  */
  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__fesetexceptflag, __old_fesetexceptflag)
compat_symbol (libm, BP_SYM (__old_fesetexceptflag), BP_SYM (fesetexceptflag), GLIBC_2_1);
#endif

versioned_symbol (libm, BP_SYM (__fesetexceptflag), BP_SYM (fesetexceptflag), GLIBC_2_2);
