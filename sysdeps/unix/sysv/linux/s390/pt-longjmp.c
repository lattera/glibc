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

   This is a copy of pthread/pt-longjmp.c made for extending the
   jmpbuf structure on System z.  */

#include <setjmp.h>
#include <stdlib.h>
#include <bits/wordsize.h>
#include "pthreadP.h"
#include  <shlib-compat.h>
#if defined SHARED && SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_19)

/* The __v1 version prototypes are declared in v1-setjmp.h which
   cannot be included together with setjmp.h.  So we put the
   prototypes here manually.  */
extern void __v1__libc_siglongjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));
extern void __v1__libc_longjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));

void __v1_siglongjmp (sigjmp_buf env, int val)
{
  __v1__libc_siglongjmp (env, val);
}

void __v1_longjmp (jmp_buf env, int val)
{
  __v1__libc_longjmp (env, val);
}

compat_symbol (libpthread, __v1_longjmp, longjmp, GLIBC_2_0);
compat_symbol (libpthread, __v1_siglongjmp, siglongjmp, GLIBC_2_0);
#endif /* defined SHARED && SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_19))  */

void
__v2_longjmp (jmp_buf env, int val)
{
  __libc_longjmp (env, val);
}

void
__v2_siglongjmp (jmp_buf env, int val)
{
  __libc_siglongjmp (env, val);
}

versioned_symbol (libpthread, __v2_longjmp, longjmp, GLIBC_2_19);
versioned_symbol (libpthread, __v2_siglongjmp, siglongjmp, GLIBC_2_19);
