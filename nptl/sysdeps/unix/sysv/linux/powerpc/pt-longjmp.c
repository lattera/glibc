/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <setjmp.h>
#include <stdlib.h>
#include <bits/wordsize.h>
#include "pthreadP.h"
#include  <shlib-compat.h>
#if defined SHARED && SHLIB_COMPAT (libpthread, GLIBC_2_0, GLIBC_2_3_4)

/* These functions are not declared anywhere since they shouldn't be
   used at another place but here.  */
extern void __novmx__libc_siglongjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));
extern void __novmx__libc_longjmp (sigjmp_buf env, int val)
     __attribute__ ((noreturn));


void __novmx_siglongjmp (sigjmp_buf env, int val)
{
  __novmx__libc_siglongjmp (env, val);
}

void __novmx_longjmp (jmp_buf env, int val)
{
  __novmx__libc_longjmp (env, val);
}

# if __WORDSIZE == 64
symbol_version (__novmx_longjmp,longjmp,GLIBC_2.3);
symbol_version (__novmx_siglongjmp,siglongjmp,GLIBC_2.3);
# else
symbol_version (__novmx_longjmp,longjmp,GLIBC_2.0);
symbol_version (__novmx_siglongjmp,siglongjmp,GLIBC_2.0);
# endif
#endif /* defined SHARED && SHLIB_COMPAT (libc, GLIBC_2_0, GLIBC_2_3_4))  */ 

void
__vmx_longjmp (jmp_buf env, int val)
{
  __libc_longjmp (env, val);
}

void
__vmx_siglongjmp (jmp_buf env, int val)
{
  __libc_siglongjmp (env, val);
}

versioned_symbol (libc, __vmx_longjmp, longjmp, GLIBC_2_3_4);
versioned_symbol (libc, __vmx_siglongjmp, siglongjmp, GLIBC_2_3_4);
