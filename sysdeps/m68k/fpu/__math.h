/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifdef	__GNUC__

#include <sys/cdefs.h>

#ifdef	__NO_MATH_INLINES
#define	__m81_u(x)	__CONCAT(__,x)
#else
#define	__m81_u(x)	x
#define	__MATH_INLINES	1
#endif

#define	__inline_mathop2(func, op)					      \
  extern __inline __CONSTVALUE double					      \
  __m81_u(func)(double __mathop_x)					      \
  {									      \
    double __result;							      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }
#define	__inline_mathop(op)		__inline_mathop2(op, op)

__inline_mathop(acos)
__inline_mathop(asin)
__inline_mathop(atan)
__inline_mathop(cos)
__inline_mathop(sin)
__inline_mathop(tan)
__inline_mathop(cosh)
__inline_mathop(sinh)
__inline_mathop(tanh)
__inline_mathop2(exp, etox)
__inline_mathop2(fabs, abs)
__inline_mathop(log10)
__inline_mathop2(log, logn)
__inline_mathop2(floor, intrz)
__inline_mathop(sqrt)

__inline_mathop2(__rint, int)
__inline_mathop2(__expm1, etoxm1)

#ifdef	__USE_MISC
__inline_mathop2(rint, int)
__inline_mathop2(expm1, etoxm1)
__inline_mathop2(log1p, lognp1)
__inline_mathop(atanh)
#endif

extern __inline __CONSTVALUE double
__m81_u(__drem)(double __x, double __y)
{
  double __result;
  __asm("frem%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

extern __inline __CONSTVALUE double
__m81_u(ldexp)(double __x, int __e)
{
  double __result;
  double __double_e = (double) __e;
  __asm("fscale%.x %1, %0" : "=f" (__result) : "f" (__double_e), "0" (__x));
  return __result;
}

extern __inline __CONSTVALUE double
__m81_u(fmod)(double __x, double __y)
{
  double __result;
  __asm("fmod%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

extern __inline double
__m81_u(frexp)(double __value, int *__expptr)
{
  double __mantissa, __exponent;
  __asm("fgetexp%.x %1, %0" : "=f" (__exponent) : "f" (__value));
  __asm("fgetman%.x %1, %0" : "=f" (__mantissa) : "f" (__value));
  *__expptr = (int) __exponent;
  return __mantissa;
}

extern __inline __CONSTVALUE double
__m81_u(pow)(double __x, double __y)
{
  double __result;
  if (__y == 0.0 || __x == 1.0)
    __result = 1.0;
  else if (__y == 1.0)
    __result = __x;
  else if (__y == 2.0)
    __result = __x * __x;
  else if (__x == 10.0)
    __asm("ftentox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else if (__x == 2.0)
    __asm("ftwotox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else
    __result = __m81_u(exp)(__y * __m81_u(log)(__x));
  return __result;
}

extern __inline __CONSTVALUE double
__m81_u(ceil)(double __x)
{
  double __result;
  unsigned long int __ctrl_reg;
  __asm("fmove%.l fpcr, %0" : "=g" (__ctrl_reg));
  /* Set rounding towards positive infinity.  */
  __asm("fmove%.l %0, fpcr" : /* No outputs.  */ : "g" (__ctrl_reg | 0x30));
  /* Convert X to an integer, using +Inf rounding.  */
  __asm("fint%.x %1, %0" : "=f" (__result) : "f" (__x));
  /* Restore the previous rounding mode.  */
  __asm("fmove%.l %0, fpcr" : /* No outputs.  */ : "g" (__ctrl_reg));
  return __result;
}

extern __inline double
__m81_u(modf)(double __value, double *__iptr)
{
  double __modf_int = __m81_u(floor)(__value);
  *__iptr = __modf_int;
  return __value - __modf_int;
}

extern __inline __CONSTVALUE int
__m81_u(__isinf)(double __value)
{
  /* There is no branch-condition for infinity,
     so we must extract and examine the condition codes manually.  */
  unsigned long int __fpsr;
  __asm("ftst%.x %1\n"
	"fmove%.l fpsr, %0" : "=g" (__fpsr) : "f" (__value));
  return (__fpsr & (2 << (3 * 8))) ? (__value < 0 ? -1 : 1) : 0;
}

extern __inline __CONSTVALUE int
__m81_u(__isnan)(double __value)
{
  char __result;
  __asm("ftst%.x %1\n"
	"fsun %0" : "=g" (__result) : "f" (__value));
  return __result;
}

#endif	/* GCC.  */
