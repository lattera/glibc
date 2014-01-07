/* Copyright (C) 2013 Free Software Foundation, Inc.
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
   <http://www.gnu.org/licenses/>.

   Versioned copy of sysdeps/generic/longjmp.c modified for extended
   jmpbuf.  */

#include <shlib-compat.h>
#include <stddef.h>
#include <signal.h>
#include "v1-setjmp.h"

#if !defined NOT_INT_libc && defined SHARED
# if SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_19)

void
__v1__libc_siglongjmp (__v1__sigjmp_buf env, int val)
{
  /* Perform any cleanups needed by the frames being unwound.  */
  _longjmp_unwind (env, val);

  if (env[0].__mask_was_saved)
    /* Restore the saved signal mask.  */
    (void) __sigprocmask (SIG_SETMASK, &env[0].__saved_mask,
			  (sigset_t *) NULL);

  /* Call the machine-dependent function to restore machine state.  */
  __v1__longjmp (env[0].__jmpbuf, val ?: 1);
}

#  ifndef __v1__longjmp
strong_alias (__v1__libc_siglongjmp, __v1__libc_longjmp)
libc_hidden_def (__v1__libc_longjmp)
weak_alias (__v1__libc_siglongjmp, __v1_longjmp)
weak_alias (__v1__libc_siglongjmp, __v1longjmp)
weak_alias (__v1__libc_siglongjmp, __v1siglongjmp)

compat_symbol (libc, __v1_longjmp, _longjmp, GLIBC_2_0);
compat_symbol (libc, __v1longjmp, longjmp, GLIBC_2_0);
compat_symbol (libc, __v1siglongjmp, siglongjmp, GLIBC_2_0);

#  endif /* ifndef __v1__longjmp */
# endif /* SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_19) */
#endif /* if !defined NOT_INT_libc && defined SHARED */
