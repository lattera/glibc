/* Copyright (C) 1991, 1992, 1994, 1997, 2001 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <setjmp.h>

/* Save the current program position in ENV and return 0.  */
int
#if defined BSD_SETJMP
# undef setjmp
# define savemask 1
setjmp (jmp_buf env)
#elif defined BSD__SETJMP
# undef _setjmp
# define savemask 0
_setjmp (jmp_buf env)
#else
__sigsetjmp (jmp_buf env, int savemask)
#endif
{
  /* Save data registers D1 through D7.  */
  asm volatile ("movem%.l %/d1-%/d7, %0"
		: : "m" (env[0].__jmpbuf[0].__dregs[0]));

  /* Save return address in place of register A0.  */
  env[0].__jmpbuf[0].__aregs[0] = ((void **) &env)[-1];

  /* Save address registers A1 through A5.  */
  asm volatile ("movem%.l %/a1-%/a5, %0"
		: : "m" (env[0].__jmpbuf[0].__aregs[1]));

  /* Save caller's FP, not our own.  */
  env[0].__jmpbuf[0].__fp = ((void **) &env)[-2];

  /* Save caller's SP, not our own.  */
  env[0].__jmpbuf[0].__sp = (void *) &env;

#if defined __HAVE_68881__ || defined __HAVE_FPU__
  /* Save floating-point (68881) registers FP0 through FP7.  */
  asm volatile ("fmovem%.x %/fp0-%/fp7, %0"
		: : "m" (env[0].__jmpbuf[0].__fpregs[0]));
#endif

  /* Save the signal mask if requested.  */
  return __sigjmp_save (env, savemask);
}
#if !defined BSD_SETJMP && !defined BSD__SETJMP
hidden_def (__sigsetjmp)
#endif
