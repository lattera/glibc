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
/* This is used when defining the functions themselves.  Define them with
   __ names, and with `static inline' instead of `extern inline' so the
   bodies will always be used, never an external function call.  */
#define	__m81_u(x)	__CONCAT(__,x)
#define __m81_inline	static __inline
#else
#define	__m81_u(x)	x
#define __m81_inline	extern __inline
#define	__MATH_INLINES	1
#endif

/* Define a const math function.  */
#define __m81_defun(rettype, func, args)				      \
  __m81_inline rettype							      \
  __m81_u(func) args __attribute__((__const__));			      \
  __m81_inline rettype							      \
  __m81_u(func) args

#define	__inline_mathop(func, op)					      \
  __m81_defun (double, func, (double __mathop_x))			      \
  {									      \
    double __result;							      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }

#define __inline_mathopf(func, op)					      \
  __m81_defun (float, func, (float __mathop_x))				      \
  {									      \
    float __result;							      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }

#define __inline_mathopl(func, op)					      \
  __m81_defun (long double, func, (long double __mathop_x))		      \
  {									      \
    long double __result;						      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }
  
/* ieee style elementary functions */
__inline_mathop(__ieee754_acos, acos)
__inline_mathop(__ieee754_asin, asin)
__inline_mathop(__ieee754_cosh, cosh)
__inline_mathop(__ieee754_sinh, sinh)
__inline_mathop(__ieee754_exp, etox)
__inline_mathop(__ieee754_log10, log10)
__inline_mathop(__ieee754_log, logn)
__inline_mathop(__ieee754_sqrt, sqrt)
__inline_mathop(__ieee754_atanh, atanh)

/* ieee style elementary float functions */
__inline_mathopf(__ieee754_acosf, acos)
__inline_mathopf(__ieee754_asinf, asin)
__inline_mathopf(__ieee754_coshf, cosh)
__inline_mathopf(__ieee754_sinhf, sinh)
__inline_mathopf(__ieee754_expf, etox)
__inline_mathopf(__ieee754_log10f, log10)
__inline_mathopf(__ieee754_logf, logn)
__inline_mathopf(__ieee754_sqrtf, sqrt)
__inline_mathopf(__ieee754_atanhf, atan)

/* ieee style elementary long double functions */
__inline_mathopl(__ieee754_acosl, acos)
__inline_mathopl(__ieee754_asinl, asin)
__inline_mathopl(__ieee754_coshl, cosh)
__inline_mathopl(__ieee754_sinhl, sinh)
__inline_mathopl(__ieee754_expl, etox)
__inline_mathopl(__ieee754_log10l, log10)
__inline_mathopl(__ieee754_logl, logn)
__inline_mathopl(__ieee754_sqrtl, sqrt)
__inline_mathopl(__ieee754_atanhl, atan)

__inline_mathop(__atan, atan)
__inline_mathop(__cos, cos)
__inline_mathop(__sin, sin)
__inline_mathop(__tan, tan)
__inline_mathop(__tanh, tanh)
__inline_mathop(__fabs, abs)
__inline_mathop(__sqrt, sqrt)

__inline_mathop(__rint, int)
__inline_mathop(__expm1, etoxm1)
__inline_mathop(__log1p, lognp1)
__inline_mathop(__logb, log2)
__inline_mathop(__significand, getman)

__inline_mathopf(__atanf, atan)
__inline_mathopf(__cosf, cos)
__inline_mathopf(__sinf, sin)
__inline_mathopf(__tanf, tan)
__inline_mathopf(__tanhf, tanh)
__inline_mathopf(__fabsf, abs)
__inline_mathopf(__sqrtf, sqrt)

__inline_mathopf(__rintf, int)
__inline_mathopf(__expm1f, etoxm1)
__inline_mathopf(__log1pf, lognp1)
__inline_mathopf(__logbf, log2)
__inline_mathopf(__significandf, getman)

__inline_mathopl(__atanl, atan)
__inline_mathopl(__cosl, cos)
__inline_mathopl(__sinl, sin)
__inline_mathopl(__tanl, tan)
__inline_mathopl(__tanhl, tanh)
__inline_mathopl(__fabsl, abs)
__inline_mathopl(__sqrtl, sqrt)

__inline_mathopl(__rintl, int)
__inline_mathopl(__expm1l, etoxm1)
__inline_mathopl(__log1pl, lognp1)
__inline_mathopl(__logbl, log2)
__inline_mathopl(__significandl, getman)

__m81_defun (double, __ieee754_remainder, (double __x, double __y))
{
  double __result;
  __asm("frem%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

__m81_defun (double, __ldexp, (double __x, int __e))
{
  double __result;
  double __double_e = (double) __e;
  __asm("fscale%.x %1, %0" : "=f" (__result) : "f" (__double_e), "0" (__x));
  return __result;
}

__m81_defun (double, __ieee754_fmod, (double __x, double __y))
{
  double __result;
  __asm("fmod%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

__m81_inline double
__m81_u(__frexp)(double __value, int *__expptr)
{
  double __mantissa, __exponent;
  __asm("fgetexp%.x %1, %0" : "=f" (__exponent) : "f" (__value));
  __asm("fgetman%.x %1, %0" : "=f" (__mantissa) : "f" (__value));
  *__expptr = (int) __exponent;
  return __mantissa;
}

__m81_defun (double, __floor, (double __x))
{
  double __result;
  unsigned long int __ctrl_reg;
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));
  /* Set rounding towards negative infinity.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */ 
		      : "dmi" ((__ctrl_reg & ~0x10) | 0x20));
  /* Convert X to an integer, using -Inf rounding.  */
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));
  /* Restore the previous rounding mode.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg));
  return __result;
}

__m81_defun (double, __ieee754_pow, (double __x, double __y))
{
  double __result;
  if (__x == 0.0)
    {
      if (__y <= 0.0)
	__result = 0.0 / 0.0;
      else
	__result = 0.0;
    }
  else if (__y == 0.0 || __x == 1.0)
    __result = 1.0;
  else if (__y == 1.0)
    __result = __x;
  else if (__y == 2.0)
    __result = __x * __x;
  else if (__x == 10.0)
    __asm("ftentox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else if (__x == 2.0)
    __asm("ftwotox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else if (__x < 0.0)
    {
      double __temp = __m81_u (__rint) (__y);
      if (__y == __temp)
	{
	  int i = (int) __y;
	  __result = __m81_u(__ieee754_exp)(__y * __m81_u(__ieee754_log)(-__x));
	  if (i & 1)
	    __result = -__result;
	}
      else
	__result = 0.0 / 0.0;
    }
  else
    __result = __m81_u(__ieee754_exp)(__y * __m81_u(__ieee754_log)(__x));
  return __result;
}

__m81_defun (double, __ceil, (double __x))
{
  double __result;
  unsigned long int __ctrl_reg;
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));
  /* Set rounding towards positive infinity.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg | 0x30));
  /* Convert X to an integer, using +Inf rounding.  */
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));
  /* Restore the previous rounding mode.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg));
  return __result;
}

__m81_inline double
__m81_u(__modf)(double __value, double *__iptr)
{
  double __modf_int;
  __asm ("fintrz%.x %1, %0" : "=f" (__modf_int) : "f" (__value));
  *__iptr = __modf_int;
  return __value - __modf_int;
}

__m81_defun (int, __isinf, (double __value))
{
  /* There is no branch-condition for infinity,
     so we must extract and examine the condition codes manually.  */
  unsigned long int __fpsr;
  __asm("ftst%.x %1\n"
	"fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));
  return (__fpsr & (2 << 24)) ? (__fpsr & (8 << 24) ? -1 : 1) : 0;
}

__m81_defun (int, __isnan, (double __value))
{
  char __result;
  __asm("ftst%.x %1\n"
	"fsun %0" : "=dm" (__result) : "f" (__value));
  return __result;
}

__m81_defun (int, __finite, (double __value))
{
  /* There is no branch-condition for infinity, so we must extract and
     examine the condition codes manually.  */
  unsigned long int __fpsr;
  __asm ("ftst%.x %1\n"
	 "fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));
  return (__fpsr & (3 << 24)) == 0;
}

__m81_defun (int, __ilogb, (double __x))
{
  double __result;
  __asm("fgetexp%.x %1, %0" : "=f" (__result) : "f" (__x));
  return (int) __result;
}

__m81_defun (double, __ieee754_scalb, (double __x, double __n))
{
  double __result;
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__n), "0" (__x));
  return __result;
}

__m81_defun (double, __scalbn, (double __x, int __n))
{
  double __result;
  double __double_n = (double) __n;
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__double_n), "0" (__x));
  return __result;
}

__m81_defun (float, __ieee754_remainderf, (float __x, float __y))
{
  float __result;
  __asm("frem%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

__m81_defun (float, __ldexpf, (float __x, int __e))
{
  float __result;
  float __float_e = (float) __e;
  __asm("fscale%.x %1, %0" : "=f" (__result) : "f" (__float_e), "0" (__x));
  return __result;
}

__m81_defun (float, __ieee754_fmodf, (float __x, float __y))
{
  float __result;
  __asm("fmod%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

__m81_inline float
__m81_u(__frexpf)(float __value, int *__expptr)
{
  float __mantissa, __exponent;
  __asm("fgetexp%.x %1, %0" : "=f" (__exponent) : "f" (__value));
  __asm("fgetman%.x %1, %0" : "=f" (__mantissa) : "f" (__value));
  *__expptr = (int) __exponent;
  return __mantissa;
}

__m81_defun (float, __floorf, (float __x))
{
  float __result;
  unsigned long int __ctrl_reg;
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));
  /* Set rounding towards negative infinity.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */ 
		      : "dmi" ((__ctrl_reg & ~0x10) | 0x20));
  /* Convert X to an integer, using -Inf rounding.  */
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));
  /* Restore the previous rounding mode.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg));
  return __result;
}

__m81_defun (float, __ieee754_powf, (float __x, float __y))
{
  float __result;
  if (__x == 0.0f)
    {
      if (__y <= 0.0f)
	__result = 0.0f / 0.0f;
      else
	__result = 0.0f;
    }
  else if (__y == 0.0f || __x == 1.0f)
    __result = 1.0;
  else if (__y == 1.0f)
    __result = __x;
  else if (__y == 2.0f)
    __result = __x * __x;
  else if (__x == 10.0f)
    __asm("ftentox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else if (__x == 2.0f)
    __asm("ftwotox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else if (__x < 0.0f)
    {
      float __temp = __m81_u(__rintf)(__y);
      if (__y == __temp)
	{
	  int i = (int) __y;
	  __result = __m81_u(__ieee754_expf)(__y * __m81_u(__ieee754_logf)(-__x));
	  if (i & 1)
	    __result = -__result;
	}
      else
	__result = 0.0f / 0.0f;
    }
  else
    __result = __m81_u(__ieee754_expf)(__y * __m81_u(__ieee754_logf)(__x));
  return __result;
}

__m81_defun (float, __ceilf, (float __x))
{
  float __result;
  unsigned long int __ctrl_reg;
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));
  /* Set rounding towards positive infinity.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg | 0x30));
  /* Convert X to an integer, using +Inf rounding.  */
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));
  /* Restore the previous rounding mode.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg));
  return __result;
}

__m81_inline float
__m81_u(__modff)(float __value, float *__iptr)
{
  float __modf_int;
  __asm ("fintrz%.x %1, %0" : "=f" (__modf_int) : "f" (__value));
  *__iptr = __modf_int;
  return __value - __modf_int;
}

__m81_defun (int, __isinff, (float __value))
{
  /* There is no branch-condition for infinity,
     so we must extract and examine the condition codes manually.  */
  unsigned long int __fpsr;
  __asm("ftst%.x %1\n"
	"fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));
  return (__fpsr & (2 << 24)) ? (__fpsr & (8 << 24) ? -1 : 1) : 0;
}

__m81_defun (int, __isnanf, (float __value))
{
  char __result;
  __asm("ftst%.x %1\n"
	"fsun %0" : "=dm" (__result) : "f" (__value));
  return __result;
}

__m81_defun (int, __finitef, (float __value))
{
  /* There is no branch-condition for infinity, so we must extract and
     examine the condition codes manually.  */
  unsigned long int __fpsr;
  __asm ("ftst%.x %1\n"
	 "fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));
  return (__fpsr & (3 << 24)) == 0;
}

__m81_defun (int, __ilogbf, (float __x))
{
  float __result;
  __asm("fgetexp%.x %1, %0" : "=f" (__result) : "f" (__x));
  return (int) __result;
}

__m81_defun (float, __ieee754_scalbf, (float __x, float __n))
{
  float __result;
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__n), "0" (__x));
  return __result;
}

__m81_defun (float, __scalbnf, (float __x, int __n))
{
  float __result;
  float __float_n = (float) __n;
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__float_n), "0" (__x));
  return __result;
}

__m81_defun (long double, __ieee754_remainderl, (long double __x,
						 long double __y))
{
  long double __result;
  __asm ("frem%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

__m81_defun (long double, __ldexpl, (long double __x, int __e))
{
  long double __result;
  long double __float_e = (long double) __e;
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__float_e), "0" (__x));
  return __result;
}

__m81_defun (long double, __ieee754_fmodl, (long double __x, long double __y))
{
  long double __result;
  __asm("fmod%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));
  return __result;
}

__m81_inline long double
__m81_u(__frexpl)(long double __value, int *__expptr)
{
  long double __mantissa, __exponent;
  __asm("fgetexp%.x %1, %0" : "=f" (__exponent) : "f" (__value));
  __asm("fgetman%.x %1, %0" : "=f" (__mantissa) : "f" (__value));
  *__expptr = (int) __exponent;
  return __mantissa;
}

__m81_defun (long double, __floorl, (long double __x))
{
  long double __result;
  unsigned long int __ctrl_reg;
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));
  /* Set rounding towards negative infinity.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */ 
		      : "dmi" ((__ctrl_reg & ~0x10) | 0x20));
  /* Convert X to an integer, using -Inf rounding.  */
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));
  /* Restore the previous rounding mode.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg));
  return __result;
}

__m81_defun (long double, __ieee754_powl, (long double __x, long double __y))
{
  long double __result;
  if (__x == 0.0l)
    {
      if (__y <= 0.0l)
	__result = 0.0l / 0.0l;
      else
	__result = 0.0l;
    }
  else if (__y == 0.0l || __x == 1.0l)
    __result = 1.0;
  else if (__y == 1.0l)
    __result = __x;
  else if (__y == 2.0l)
    __result = __x * __x;
  else if (__x == 10.0l)
    __asm("ftentox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else if (__x == 2.0l)
    __asm("ftwotox%.x %1, %0" : "=f" (__result) : "f" (__y));
  else if (__x < 0.0l)
    {
      long double __temp = __m81_u(__rintl)(__y);
      if (__y == __temp)
	{
	  int i = (int) __y;
	  __result
	    = __m81_u(__ieee754_expl)(__y * __m81_u(__ieee754_logl)(-__x));
	  if (i & 1)
	    __result = -__result;
	}
      else
	__result = 0.0l / 0.0l;
    }
  else
    __result = __m81_u(__ieee754_expl)(__y * __m81_u(__ieee754_logl)(__x));
  return __result;
}

__m81_defun (long double, __ceill, (long double __x))
{
  long double __result;
  unsigned long int __ctrl_reg;
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));
  /* Set rounding towards positive infinity.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg | 0x30));
  /* Convert X to an integer, using +Inf rounding.  */
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));
  /* Restore the previous rounding mode.  */
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */
		      : "dmi" (__ctrl_reg));
  return __result;
}

__m81_inline long double
__m81_u(__modfl)(long double __value, long double *__iptr)
{
  long double __modf_int;
  __asm ("fintrz%.x %1, %0" : "=f" (__modf_int) : "f" (__value));
  *__iptr = __modf_int;
  return __value - __modf_int;
}

__m81_defun (int, __isinfl, (long double __value))
{
  /* There is no branch-condition for infinity,
     so we must extract and examine the condition codes manually.  */
  unsigned long int __fpsr;
  __asm("ftst%.x %1\n"
	"fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));
  return (__fpsr & (2 << 24)) ? (__fpsr & (8 << 24) ? -1 : 1) : 0;
}

__m81_defun (int, __isnanl, (long double __value))
{
  char __result;
  __asm("ftst%.x %1\n"
	"fsun %0" : "=dm" (__result) : "f" (__value));
  return __result;
}

__m81_defun (int, __finitel, (long double __value))
{
  /* There is no branch-condition for infinity, so we must extract and
     examine the condition codes manually.  */
  unsigned long int __fpsr;
  __asm ("ftst%.x %1\n"
	 "fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));
  return (__fpsr & (3 << 24)) == 0;
}

__m81_defun (int, __ilogbl, (long double __x))
{
  long double __result;
  __asm("fgetexp%.x %1, %0" : "=f" (__result) : "f" (__x));
  return (int) __result;
}

__m81_defun (long double, __ieee754_scalbl, (long double __x, long double __n))
{
  long double __result;
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__n), "0" (__x));
  return __result;
}

__m81_defun (long double, __scalbnl, (long double __x, int __n))
{
  long double __result;
  long double __float_n = (long double) __n;
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__float_n), "0" (__x));
  return __result;
}

#endif	/* GCC.  */
