/* Common utilities for testing strtod and its derivatives.
   This file is part of the GNU C Library.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.

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

#ifndef _TST_STRTOD_H
#define _TST_STRTOD_H

#define FSTRLENMAX 128

#include <bits/floatn.h>

#define F128 __f128 ()

/* Test strfromf128 and strtof128 on all platforms that provide them,
   whether or not the type _Float128 is ABI-distinct from long double.  */
#if __HAVE_FLOAT128
# define IF_FLOAT128(x) x
#else
# define IF_FLOAT128(x)
#endif

/* Provide an extra parameter expansion for mfunc.  */
#define MMFUNC(mmfunc, ...) mmfunc (__VA_ARGS__)

/* Splat n variants of the same test for the various strtod functions.  */
#define GEN_TEST_STRTOD_FOREACH(mfunc, ...)				      \
  mfunc (  f,       float, strfromf, f, f, ##__VA_ARGS__)		      \
  mfunc (  d,      double, strfromd,  ,  , ##__VA_ARGS__)		      \
  mfunc ( ld, long double, strfroml, L, l, ##__VA_ARGS__)		      \
  IF_FLOAT128 (MMFUNC							      \
   (mfunc, f128, _Float128, strfromf128, F128, f128, ##__VA_ARGS__))
/* The arguments to the generated macros are:
   FSUF - Function suffix
   FTYPE - float type
   FTOSTR - float to string func
   LSUF - Literal suffix
   CSUF - C standardish suffix for many of the math functions
*/



#define STRTOD_TEST_FOREACH(mfunc, ...)			\
({							\
   int result = 0;					\
   result |= mfunc ## f  (__VA_ARGS__);			\
   result |= mfunc ## d  (__VA_ARGS__);			\
   result |= mfunc ## ld (__VA_ARGS__);			\
   IF_FLOAT128 (result |= mfunc ## f128 (__VA_ARGS__));	\
   result;						\
})


#endif
