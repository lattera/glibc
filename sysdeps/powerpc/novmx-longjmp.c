/* Copyright (C) 1991,92,94,95,97,98,2000,2002,2003,2004
   Free Software Foundation, Inc.
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

/* Copy of sysdeps/generic/longjmp.c modified for backward compatibility
   with old non AltiVec/VMX longjmp.  */

#include <bits/wordsize.h>
#include <shlib-compat.h>
#if defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_4)
# include <stddef.h>
# include <novmxsetjmp.h>
# include <signal.h>


/* Set the signal mask to the one specified in ENV, and jump
   to the position specified in ENV, causing the setjmp
   call there to return VAL, or 1 if VAL is 0.  */
void
__novmx__libc_siglongjmp (__novmx__sigjmp_buf env, int val)
{
  /* Perform any cleanups needed by the frames being unwound.  */
  _longjmp_unwind (env, val);

  if (env[0].__mask_was_saved)
    /* Restore the saved signal mask.  */
    (void) __sigprocmask (SIG_SETMASK, &env[0].__saved_mask,
			  (sigset_t *) NULL);

  /* Call the machine-dependent function to restore machine state.  */
  __novmx__longjmp (env[0].__jmpbuf, val ?: 1);
}

strong_alias (__novmx__libc_siglongjmp, __novmx__libc_longjmp)
libc_hidden_def (__novmx__libc_longjmp)
weak_alias (__novmx__libc_siglongjmp, __novmx_longjmp)
weak_alias (__novmx__libc_siglongjmp, __novmxlongjmp)
weak_alias (__novmx__libc_siglongjmp, __novmxsiglongjmp)

# if __WORDSIZE == 64
symbol_version (__novmx_longjmp,_longjmp,GLIBC_2.3);
symbol_version (__novmxlongjmp,longjmp,GLIBC_2.3);
symbol_version (__novmxsiglongjmp,siglongjmp,GLIBC_2.3);
# else
symbol_version (__novmx_longjmp,_longjmp,GLIBC_2.0);
symbol_version (__novmxlongjmp,longjmp,GLIBC_2.0);
symbol_version (__novmxsiglongjmp,siglongjmp,GLIBC_2.0);
# endif
#endif /* defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_4))  */
