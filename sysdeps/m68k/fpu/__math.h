/* Copyright (C) 1991, 92, 93, 94, 96 Free Software Foundation, Inc.
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

#ifdef	__NO_M81_MATH_INLINES
/* This is used when defining the functions themselves.  Define them with
   __ names, and with `static inline' instead of `extern inline' so the
   bodies will always be used, never an external function call.  */
#define	__m81_u(x)	__CONCAT(__,x)
#define __m81_inline	static __inline
#else
#define	__m81_u(x)	x
#define __m81_inline	extern __inline
#define	__M81_MATH_INLINES	1
#endif

/* Define a const math function.  */
#define __m81_defun(rettype, func, args)				      \
  __m81_inline rettype							      \
  __m81_u(func) args __attribute__((__const__));			      \
  __m81_inline rettype							      \
  __m81_u(func) args

/* Define the three variants of a math function that has a direct
   implementation in the m68k fpu.  FUNC is the name for C (which will be
   suffixed with f and l for the float and long double version, resp).  OP
   is the name of the fpu operation (without leading f).  */
#define	__inline_mathop(func, op)					      \
  __m81_defun (double, func, (double __mathop_x))			      \
  {									      \
    double __result;							      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }									      \
  __m81_defun (float, func##f, (float __mathop_x))			      \
  {									      \
    float __result;							      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }									      \
  __m81_defun (long double, func##l, (long double __mathop_x))		      \
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

/* This macro contains the definition for the rest of the inline
   functions, using __FLOAT_TYPE as the domain type and __S as the suffix
   for the function names.  */

#define __inline_functions(__float_type, __s)				     \
__m81_defun (__float_type,						     \
	     __ieee754_remainder##__s, (__float_type __x, __float_type __y)) \
{									     \
  __float_type __result;						     \
  __asm("frem%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));	     \
  return __result;							     \
}									     \
									     \
__m81_defun (__float_type,						     \
	     __ieee754_fmod##__s, (__float_type __x, __float_type __y))	     \
{									     \
  __float_type __result;						     \
  __asm("fmod%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));	     \
  return __result;							     \
}									     \
									     \
__m81_defun (__float_type,						     \
	     __ieee754_atan2##__s, (__float_type __y, __float_type __x))     \
{									     \
  __float_type __pi, __pi_2;						     \
									     \
  __asm ("fmovecr%.x %#0, %0" : "=f" (__pi));				     \
  __asm ("fscale%.w %#-1, %0" : "=f" (__pi_2) : "0" (__pi));		     \
  if (__x > 0)								     \
    {									     \
      if (__y > 0)							     \
	{								     \
	  if (__x > __y)						     \
	    return __m81_u(__atan##__s) (__y / __x);			     \
	  else								     \
	    return __pi_2 - __m81_u(__atan##__s) (__x / __y);		     \
	}								     \
      else								     \
	{								     \
	  if (__x > -__y)						     \
	    return __m81_u(__atan##__s) (__y / __x);			     \
	  else								     \
	    return -__pi_2 - __m81_u(__atan##__s) (__x / __y);		     \
	}								     \
    }									     \
  else									     \
    {									     \
      if (__y > 0)							     \
	{								     \
	  if (-__x < __y)						     \
	    return __pi + __m81_u(__atan##__s) (__y / __x);		     \
	  else								     \
	    return __pi_2 - __m81_u(__atan##__s) (__x / __y);		     \
	}								     \
      else								     \
	{								     \
	  if (-__x > -__y)						     \
	    return -__pi + __m81_u(__atan##__s) (__y / __x);		     \
	  else								     \
	    return -__pi_2 - __m81_u(__atan##__s) (__x / __y);		     \
	}								     \
    }									     \
}									     \
									     \
__m81_inline __float_type						     \
__m81_u(__frexp##__s)(__float_type __value, int *__expptr)		     \
{									     \
  __float_type __mantissa, __exponent;					     \
  int __iexponent;							     \
  if (__value == 0.0)							     \
    {									     \
      *__expptr = 0;							     \
      return __value;							     \
    }									     \
  __asm("fgetexp%.x %1, %0" : "=f" (__exponent) : "f" (__value));	     \
  __iexponent = (int) __exponent + 1;					     \
  *__expptr = __iexponent;						     \
  __asm("fscale%.l %2, %0" : "=f" (__mantissa)				     \
	: "0" (__value), "dmi" (-__iexponent));				     \
  return __mantissa;							     \
}									     \
									     \
__m81_defun (__float_type, __floor##__s, (__float_type __x))		     \
{									     \
  __float_type __result;						     \
  unsigned long int __ctrl_reg;						     \
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));		     \
  /* Set rounding towards negative infinity.  */			     \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */ 		     \
		      : "dmi" ((__ctrl_reg & ~0x10) | 0x20));		     \
  /* Convert X to an integer, using -Inf rounding.  */			     \
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));	     \
  /* Restore the previous rounding mode.  */				     \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		     \
		      : "dmi" (__ctrl_reg));				     \
  return __result;							     \
}									     \
									     \
__m81_defun (__float_type,						     \
	     __ieee754_pow##__s, (__float_type __x, __float_type __y))	     \
{									     \
  __float_type __result;						     \
  if (__x == 0.0)							     \
    {									     \
      if (__y <= 0.0)							     \
	__result = 0.0 / 0.0;						     \
      else								     \
	__result = 0.0;							     \
    }									     \
  else if (__y == 0.0 || __x == 1.0)					     \
    __result = 1.0;							     \
  else if (__y == 1.0)							     \
    __result = __x;							     \
  else if (__y == 2.0)							     \
    __result = __x * __x;						     \
  else if (__x == 10.0)							     \
    __asm("ftentox%.x %1, %0" : "=f" (__result) : "f" (__y));		     \
  else if (__x == 2.0)							     \
    __asm("ftwotox%.x %1, %0" : "=f" (__result) : "f" (__y));		     \
  else if (__x < 0.0)							     \
    {									     \
      __float_type __temp = __m81_u (__rint##__s) (__y);		     \
      if (__y == __temp)						     \
	{								     \
	  int __i = (int) __y;						     \
	  __result = (__m81_u(__ieee754_exp##__s)			     \
		      (__y * __m81_u(__ieee754_log##__s) (-__x)));	     \
	  if (__i & 1)							     \
	    __result = -__result;					     \
	}								     \
      else								     \
	__result = 0.0 / 0.0;						     \
    }									     \
  else									     \
    __result = (__m81_u(__ieee754_exp##__s)				     \
		(__y * __m81_u(__ieee754_log##__s) (__x)));		     \
  return __result;							     \
}									     \
									     \
__m81_defun (__float_type, __ceil##__s, (__float_type __x))		     \
{									     \
  __float_type __result;						     \
  unsigned long int __ctrl_reg;						     \
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));		     \
  /* Set rounding towards positive infinity.  */			     \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		     \
		      : "dmi" (__ctrl_reg | 0x30));			     \
  /* Convert X to an integer, using +Inf rounding.  */			     \
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));	     \
  /* Restore the previous rounding mode.  */				     \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		     \
		      : "dmi" (__ctrl_reg));				     \
  return __result;							     \
}									     \
									     \
__m81_inline __float_type						     \
__m81_u(__modf##__s)(__float_type __value, __float_type *__iptr)	     \
{									     \
  __float_type __modf_int;						     \
  __asm ("fintrz%.x %1, %0" : "=f" (__modf_int) : "f" (__value));	     \
  *__iptr = __modf_int;							     \
  return __value - __modf_int;						     \
}									     \
									     \
__m81_defun (int, __isinf##__s, (__float_type __value))			     \
{									     \
  /* There is no branch-condition for infinity,				     \
     so we must extract and examine the condition codes manually.  */	     \
  unsigned long int __fpsr;						     \
  __asm("ftst%.x %1\n"							     \
	"fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	     \
  return (__fpsr & (2 << 24)) ? (__fpsr & (8 << 24) ? -1 : 1) : 0;	     \
}									     \
									     \
__m81_defun (int, __isnan##__s, (__float_type __value))			     \
{									     \
  char __result;							     \
  __asm("ftst%.x %1\n"							     \
	"fsun %0" : "=dm" (__result) : "f" (__value));			     \
  return __result;							     \
}									     \
									     \
__m81_defun (int, __finite##__s, (__float_type __value))		     \
{									     \
  /* There is no branch-condition for infinity, so we must extract and	     \
     examine the condition codes manually.  */				     \
  unsigned long int __fpsr;						     \
  __asm ("ftst%.x %1\n"							     \
	 "fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	     \
  return (__fpsr & (3 << 24)) == 0;					     \
}									     \
									     \
__m81_defun (int, __ilogb##__s, (__float_type __x))			     \
{									     \
  __float_type __result;						     \
  if (__x == 0.0)							     \
    return 0x80000001;							     \
  __asm("fgetexp%.x %1, %0" : "=f" (__result) : "f" (__x));		     \
  return (int) __result;						     \
}									     \
									     \
__m81_defun (__float_type,						     \
	     __ieee754_scalb##__s, (__float_type __x, __float_type __n))     \
{									     \
  __float_type __result;						     \
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__n), "0" (__x));	     \
  return __result;							     \
}									     \
									     \
__m81_defun (__float_type, __scalbn##__s, (__float_type __x, int __n))	     \
{									     \
  __float_type __result;						     \
  __asm ("fscale%.l %1, %0" : "=f" (__result) : "dmi" (__n), "0" (__x));     \
  return __result;							     \
}

/* This defines the three variants of the inline functions.  */
__inline_functions (double, )
__inline_functions (float, f)
__inline_functions (long double, l)
#undef __inline_functions

#endif	/* GCC.  */
