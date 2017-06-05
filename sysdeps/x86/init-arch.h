/* This file is part of the GNU C Library.
   Copyright (C) 2008-2017 Free Software Foundation, Inc.

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

#ifdef  __ASSEMBLER__
# include <cpu-features.h>
#else
# include <ldsodefs.h>
#endif

/* These macros are used to implement ifunc selection in C.  To implement
   an ifunc function, foo, which returns the address of __foo_sse2 or
   __foo_avx2:

   #define foo __redirect_foo
   #define __foo __redirect___foo
   #include <foo.h>
   #undef foo
   #undef __foo
   #define SYMBOL_NAME foo
   #include <init-arch.h>

   extern __typeof (REDIRECT_NAME) OPTIMIZE (sse2) attribute_hidden;
   extern __typeof (REDIRECT_NAME) OPTIMIZE (avx2) attribute_hidden;

   static inline void *
   foo_selector (void)
   {
     if (use AVX2)
      return OPTIMIZE (avx2);

     return OPTIMIZE (sse2);
   }

   libc_ifunc_redirected (__redirect_foo, foo, foo_selector ());

*/

#define PASTER1(x,y)	x##_##y
#define EVALUATOR1(x,y)	PASTER1 (x,y)
#define PASTER2(x,y)	__##x##_##y
#define EVALUATOR2(x,y)	PASTER2 (x,y)

/* Basically set '__redirect_<symbol>' to use as type definition,
   '__<symbol>_<variant>' as the optimized implementation and
   '<symbol>_ifunc_selector' as the IFUNC selector.  */
#define REDIRECT_NAME	EVALUATOR1 (__redirect, SYMBOL_NAME)
#define OPTIMIZE(name)	EVALUATOR2 (SYMBOL_NAME, name)
#define IFUNC_SELECTOR	EVALUATOR1 (SYMBOL_NAME, ifunc_selector)

#ifndef __x86_64__
/* Due to the reordering and the other nifty extensions in i686, it is
   not really good to use heavily i586 optimized code on an i686.  It's
   better to use i486 code if it isn't an i586.  */
# if MINIMUM_ISA == 686
#  define USE_I586 0
#  define USE_I686 1
# else
#  define USE_I586 (HAS_ARCH_FEATURE (I586) && !HAS_ARCH_FEATURE (I686))
#  define USE_I686 HAS_ARCH_FEATURE (I686)
# endif
#endif
