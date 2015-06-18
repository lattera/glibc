/* Definitions for float vector tests with vector length 16.
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

#define FLOAT float
#define FUNC(function) function ## f
#define TEST_MSG "testing float vector math (without inline functions)\n"
#define MATHCONST(x) x
#define CHOOSE(Clongdouble,Cdouble,Cfloat,Cinlinelongdouble,Cinlinedouble,Cinlinefloat) Cfloat
#define PRINTF_EXPR "e"
#define PRINTF_XEXPR "a"
#define PRINTF_NEXPR "f"
#define TEST_FLOAT 1
#define TEST_MATHVEC 1

#ifndef __NO_MATH_INLINES
# define __NO_MATH_INLINES
#endif

#define EXCEPTION_TESTS_float 0
#define ROUNDING_TESTS_float(MODE) ((MODE) == FE_TONEAREST)

#define CNCT(x, y) x ## y
#define CONCAT(a, b) CNCT (a, b)

#define VEC_SUFF _vlen16
#define WRAPPER_NAME(function) CONCAT (function, VEC_SUFF)

#define FUNC_TEST(function) function ## f ## _VEC_SUFF

#define WRAPPER_DECL(func) extern FLOAT func (FLOAT x);
#define WRAPPER_DECL_ff(func) extern FLOAT func (FLOAT x, FLOAT y);
#define WRAPPER_DECL_fFF(function) extern void function (FLOAT, FLOAT *, FLOAT *);

// Wrapper from scalar to vector function with vector length 16.
#define VECTOR_WRAPPER(scalar_func, vector_func) \
extern VEC_TYPE vector_func (VEC_TYPE);		\
FLOAT scalar_func (FLOAT x)			\
{						\
  int i;					\
  VEC_TYPE mx;					\
  INIT_VEC_LOOP (mx, x, 16);			\
  VEC_TYPE mr = vector_func (mx);		\
  TEST_VEC_LOOP (mr, 16);			\
  return ((FLOAT) mr[0]);			\
}

// Wrapper from scalar 2 argument function to vector one.
#define VECTOR_WRAPPER_ff(scalar_func, vector_func) 	\
extern VEC_TYPE vector_func (VEC_TYPE, VEC_TYPE);	\
FLOAT scalar_func (FLOAT x, FLOAT y)		\
{						\
  int i;					\
  VEC_TYPE mx, my;				\
  INIT_VEC_LOOP (mx, x, 16);			\
  INIT_VEC_LOOP (my, y, 16);			\
  VEC_TYPE mr = vector_func (mx, my);		\
  TEST_VEC_LOOP (mr, 16);			\
  return ((FLOAT) mr[0]);			\
}

// Wrapper from scalar 3 argument function to vector one.
#define VECTOR_WRAPPER_fFF(scalar_func, vector_func) 	\
extern void vector_func (VEC_TYPE, VEC_TYPE *, VEC_TYPE *);	\
void scalar_func (FLOAT x, FLOAT * r, FLOAT * r1)		\
{						\
  int i;					\
  VEC_TYPE mx, mr, mr1;				\
  INIT_VEC_LOOP (mx, x, 16);			\
  vector_func (mx, &mr, &mr1);			\
  TEST_VEC_LOOP (mr, 16);			\
  TEST_VEC_LOOP (mr1, 16);			\
  *r = (FLOAT) mr[0];				\
  *r1 = (FLOAT) mr1[0];				\
  return;					\
}
