/* Definitions for double vector tests with vector length 8.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#define FLOAT double
#define FUNC(function) function
#define TEST_MSG "testing double vector math (without inline functions)\n"
#define MATHCONST(x) x
#define CHOOSE(Clongdouble,Cdouble,Cfloat,Cinlinelongdouble,Cinlinedouble,Cinlinefloat) Cdouble
#define PRINTF_EXPR "e"
#define PRINTF_XEXPR "a"
#define PRINTF_NEXPR "f"
#define TEST_DOUBLE 1
#define TEST_MATHVEC 1

#ifndef __NO_MATH_INLINES
# define __NO_MATH_INLINES
#endif

#define EXCEPTION_TESTS_double 0
#define ROUNDING_TESTS_double(MODE) ((MODE) == FE_TONEAREST)

#define CNCT(x, y) x ## y
#define CONCAT(a, b) CNCT (a, b)

#define VEC_SUFF _vlen8
#define WRAPPER_NAME(function) CONCAT (function, VEC_SUFF)

#define FUNC_TEST(function) function ## _VEC_SUFF

#define WRAPPER_DECL(function) extern FLOAT function (FLOAT);
#define WRAPPER_DECL_ff(function) extern FLOAT function (FLOAT, FLOAT);

// Wrapper from scalar to vector function with vector length 8.
#define VECTOR_WRAPPER(scalar_func, vector_func) \
extern VEC_TYPE vector_func (VEC_TYPE);		\
FLOAT scalar_func (FLOAT x)			\
{						\
  int i;					\
  VEC_TYPE mx;					\
  INIT_VEC_LOOP (mx, x, 8);			\
  VEC_TYPE mr = vector_func (mx);		\
  TEST_VEC_LOOP (8);				\
}

// Wrapper from scalar 2 argument function to vector one.
#define VECTOR_WRAPPER_ff(scalar_func, vector_func) 	\
extern VEC_TYPE vector_func (VEC_TYPE, VEC_TYPE);	\
FLOAT scalar_func (FLOAT x, FLOAT y)		\
{						\
  int i;					\
  VEC_TYPE mx, my;				\
  INIT_VEC_LOOP (mx, x, 8);			\
  INIT_VEC_LOOP (my, y, 8);			\
  VEC_TYPE mr = vector_func (mx, my);		\
  TEST_VEC_LOOP (8);				\
}
