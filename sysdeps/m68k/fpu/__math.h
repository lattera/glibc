/* Definitions of inline math functions implemented by the m68881/2.
   Copyright (C) 1991, 92, 93, 94, 96, 97 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifdef	__GNUC__

#include <sys/cdefs.h>

#ifdef	__LIBC_M81_MATH_INLINES
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
  __m81_inline rettype __attribute__((__const__))			      \
  __m81_u(func) args

/* Define the three variants of a math function that has a direct
   implementation in the m68k fpu.  FUNC is the name for C (which will be
   suffixed with f and l for the float and long double version, resp).  OP
   is the name of the fpu operation (without leading f).  */

#ifdef __USE_MISC
#define	__inline_mathop(func, op)			\
  __inline_mathop1(double, func, op)			\
  __inline_mathop1(float, __CONCAT(func,f), op)		\
  __inline_mathop1(long double, __CONCAT(func,l), op)
#else
#define	__inline_mathop(func, op)			\
  __inline_mathop1(double, func, op)
#endif

#define __inline_mathop1(float_type,func, op)				      \
  __m81_defun (float_type, func, (float_type __mathop_x))		      \
  {									      \
    float_type __result;						      \
    __asm("f" __STRING(op) "%.x %1, %0" : "=f" (__result) : "f" (__mathop_x));\
    return __result;							      \
  }

#ifdef __LIBC_M81_MATH_INLINES
/* ieee style elementary functions */
/* These are internal to the implementation of libm.  */
__inline_mathop(__ieee754_acos, acos)
__inline_mathop(__ieee754_asin, asin)
__inline_mathop(__ieee754_cosh, cosh)
__inline_mathop(__ieee754_sinh, sinh)
__inline_mathop(__ieee754_exp, etox)
__inline_mathop(__ieee754_log10, log10)
__inline_mathop(__ieee754_log, logn)
__inline_mathop(__ieee754_sqrt, sqrt)
__inline_mathop(__ieee754_atanh, atanh)
#endif

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

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

__inline_mathop(atan, atan)
__inline_mathop(cos, cos)
__inline_mathop(sin, sin)
__inline_mathop(tan, tan)
__inline_mathop(tanh, tanh)
__inline_mathop(fabs, abs)
__inline_mathop(sqrt, sqrt)

#if defined(__USE_MISC) || defined(__USE_XOPEN_EXTENDED)
__inline_mathop(rint, int)
__inline_mathop(expm1, etoxm1)
__inline_mathop(log1p, lognp1)
__inline_mathop(logb, log2)
#endif

#ifdef __USE_MISC
__inline_mathop(significand, getman)
#endif

#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */

/* This macro contains the definition for the rest of the inline
   functions, using __FLOAT_TYPE as the domain type and __S as the suffix
   for the function names.  */

#ifdef __LIBC_M81_MATH_INLINES
/* Internally used functions.  */
#define __internal_inline_functions(float_type, s)			     \
__m81_defun (float_type, __CONCAT(__ieee754_remainder,s),		     \
	     (float_type __x, float_type __y))				     \
{									     \
  float_type __result;							     \
  __asm("frem%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));	     \
  return __result;							     \
}									     \
									     \
__m81_defun (float_type, __CONCAT(__ieee754_fmod,s),			     \
	     (float_type __x, float_type __y))				     \
{									     \
  float_type __result;							     \
  __asm("fmod%.x %1, %0" : "=f" (__result) : "f" (__y), "0" (__x));	     \
  return __result;							     \
}									     \
									     \
__m81_defun (float_type, __CONCAT(__ieee754_atan2,s),			     \
	     (float_type __y, float_type __x))				     \
{									     \
  float_type __pi, __pi_2;						     \
									     \
  __asm ("fmovecr%.x %#0, %0" : "=f" (__pi));				     \
  __asm ("fscale%.w %#-1, %0" : "=f" (__pi_2) : "0" (__pi));		     \
  if (__x > 0)								     \
    {									     \
      if (__y > 0)							     \
	{								     \
	  if (__x > __y)						     \
	    return __m81_u(__CONCAT(__atan,s)) (__y / __x);		     \
	  else								     \
	    return __pi_2 - __m81_u(__CONCAT(__atan,s)) (__x / __y);	     \
	}								     \
      else								     \
	{								     \
	  if (__x > -__y)						     \
	    return __m81_u(__CONCAT(__atan,s)) (__y / __x);		     \
	  else								     \
	    return -__pi_2 - __m81_u(__CONCAT(__atan,s)) (__x / __y);	     \
	}								     \
    }									     \
  else									     \
    {									     \
      if (__y > 0)							     \
	{								     \
	  if (-__x < __y)						     \
	    return __pi + __m81_u(__CONCAT(__atan,s)) (__y / __x);	     \
	  else								     \
	    return __pi_2 - __m81_u(__CONCAT(__atan,s)) (__x / __y);	     \
	}								     \
      else								     \
	{								     \
	  if (-__x > -__y)						     \
	    return -__pi + __m81_u(__CONCAT(__atan,s)) (__y / __x);	     \
	  else								     \
	    return -__pi_2 - __m81_u(__CONCAT(__atan,s)) (__x / __y);	     \
	}								     \
    }									     \
}									     \
									     \
__m81_defun (float_type, __CONCAT(__ieee754_pow,s),			     \
	     (float_type __x, float_type __y))				     \
{									     \
  float_type __result;							     \
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
      float_type __temp = __m81_u (__CONCAT(__rint,s)) (__y);		     \
      if (__y == __temp)						     \
	{								     \
	  int __i = (int) __y;						     \
	  __result = (__m81_u(__CONCAT(__ieee754_exp,s))		     \
		      (__y * __m81_u(__CONCAT(__ieee754_log,s)) (-__x)));    \
	  if (__i & 1)							     \
	    __result = -__result;					     \
	}								     \
      else								     \
	__result = 0.0 / 0.0;						     \
    }									     \
  else									     \
    __result = (__m81_u(__CONCAT(__ieee754_exp,s))			     \
		(__y * __m81_u(__CONCAT(__ieee754_log,s)) (__x)));	     \
  return __result;							     \
}									     \
									     \
__m81_defun (float_type, __CONCAT(__ieee754_scalb,s),			     \
	     (float_type __x, float_type __n))				     \
{									     \
  float_type __result;							     \
  __asm ("fscale%.x %1, %0" : "=f" (__result) : "f" (__n), "0" (__x));	     \
  return __result;							     \
}

__internal_inline_functions (double,)
__internal_inline_functions (float,f)
__internal_inline_functions (long double,l)
#undef __internal_inline_functions

#endif /* __LIBC_M81_MATH_INLINES */

/* The rest of the functions are available to the user.  */

#define __inline_functions(float_type, s)				     \
__m81_inline float_type							     \
__m81_u(__CONCAT(__frexp,s))(float_type __value, int *__expptr)		     \
{									     \
  float_type __mantissa, __exponent;					     \
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
__m81_defun (float_type, __CONCAT(__floor,s), (float_type __x))		     \
{									     \
  float_type __result;							     \
  unsigned long int __ctrl_reg;						     \
  __asm __volatile__ ("fmove%.l %!, %0" : "=dm" (__ctrl_reg));		     \
  /* Set rounding towards negative infinity.  */			     \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		     \
		      : "dmi" ((__ctrl_reg & ~0x10) | 0x20));		     \
  /* Convert X to an integer, using -Inf rounding.  */			     \
  __asm __volatile__ ("fint%.x %1, %0" : "=f" (__result) : "f" (__x));	     \
  /* Restore the previous rounding mode.  */				     \
  __asm __volatile__ ("fmove%.l %0, %!" : /* No outputs.  */		     \
		      : "dmi" (__ctrl_reg));				     \
  return __result;							     \
}									     \
									     \
__m81_defun (float_type, __CONCAT(__ceil,s), (float_type __x))		     \
{									     \
  float_type __result;							     \
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
__m81_inline float_type							     \
__m81_u(__CONCAT(__modf,s))(float_type __value, float_type *__iptr)	     \
{									     \
  float_type __modf_int;						     \
  __asm ("fintrz%.x %1, %0" : "=f" (__modf_int) : "f" (__value));	     \
  *__iptr = __modf_int;							     \
  return __value - __modf_int;						     \
}									     \
									     \
__m81_defun (int, __CONCAT(__isinf,s), (float_type __value))		     \
{									     \
  /* There is no branch-condition for infinity,				     \
     so we must extract and examine the condition codes manually.  */	     \
  unsigned long int __fpsr;						     \
  __asm("ftst%.x %1\n"							     \
	"fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	     \
  return (__fpsr & (2 << 24)) ? (__fpsr & (8 << 24) ? -1 : 1) : 0;	     \
}									     \
									     \
__m81_defun (int, __CONCAT(__isnan,s), (float_type __value))		     \
{									     \
  char __result;							     \
  __asm("ftst%.x %1\n"							     \
	"fsun %0" : "=dm" (__result) : "f" (__value));			     \
  return __result;							     \
}									     \
									     \
__m81_defun (int, __CONCAT(__finite,s), (float_type __value))		     \
{									     \
  /* There is no branch-condition for infinity, so we must extract and	     \
     examine the condition codes manually.  */				     \
  unsigned long int __fpsr;						     \
  __asm ("ftst%.x %1\n"							     \
	 "fmove%.l %/fpsr, %0" : "=dm" (__fpsr) : "f" (__value));	     \
  return (__fpsr & (3 << 24)) == 0;					     \
}									     \
									     \
__m81_defun (int, __CONCAT(__ilogb,s), (float_type __x))		     \
{									     \
  float_type __result;							     \
  if (__x == 0.0)							     \
    return 0x80000001;							     \
  __asm("fgetexp%.x %1, %0" : "=f" (__result) : "f" (__x));		     \
  return (int) __result;						     \
}									     \
									     \
__m81_defun (float_type, __CONCAT(__scalbn,s), (float_type __x, int __n))    \
{									     \
  float_type __result;							     \
  __asm ("fscale%.l %1, %0" : "=f" (__result) : "dmi" (__n), "0" (__x));     \
  return __result;							     \
}

/* This defines the three variants of the inline functions.  */
__inline_functions (double,)
__inline_functions (float,f)
__inline_functions (long double,l)
#undef __inline_functions

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

/* Define inline versions of the user visible functions.  */

#define __inline_forward_c(rettype, name, args1, args2)	\
extern __inline rettype __attribute__((__const__))	\
name args1						\
{							\
  return __CONCAT(__,name) args2;			\
}

#define __inline_forward(rettype, name, args1, args2)	\
extern __inline rettype name args1			\
{							\
  return __CONCAT(__,name) args2;			\
}

__inline_forward(double,frexp, (double __value, int *__expptr),
		 (__value, __expptr))
__inline_forward_c(double,floor, (double __x), (__x))
__inline_forward_c(double,ceil, (double __x), (__x))
__inline_forward(double,modf, (double __value, double *__iptr),
		 (__value, __iptr))
#ifdef __USE_MISC
__inline_forward_c(int,isinf, (double __value), (__value))
__inline_forward_c(int,finite, (double __value), (__value))
__inline_forward_c(double,scalbn, (double __x, int __n), (__x, __n))
#endif
#if defined __USE_MISC || defined __USE_XOPEN
__inline_forward_c(int,isnan, (double __value), (__value))
__inline_forward_c(int,ilogb, (double __value), (__value))
#endif

#ifdef __USE_MISC

__inline_forward(float,frexpf, (float __value, int *__expptr),
		 (__value, __expptr))
__inline_forward_c(float,floorf, (float __x), (__x))
__inline_forward_c(float,ceilf, (float __x), (__x))
__inline_forward(float,modff, (float __value, float *__iptr),
		 (__value, __iptr))
__inline_forward_c(int,isinff, (float __value), (__value))
__inline_forward_c(int,finitef, (float __value), (__value))
__inline_forward_c(float,scalbnf, (float __x, int __n), (__x, __n))
__inline_forward_c(int,isnanf, (float __value), (__value))
__inline_forward_c(int,ilogbf, (float __value), (__value))

__inline_forward(long double,frexpl, (long double __value, int *__expptr),
		 (__value, __expptr))
__inline_forward_c(long double,floorl, (long double __x), (__x))
__inline_forward_c(long double,ceill, (long double __x), (__x))
__inline_forward(long double,modfl,
		 (long double __value, long double *__iptr),
		 (__value, __iptr))
__inline_forward_c(int,isinfl, (long double __value), (__value))
__inline_forward_c(int,finitel, (long double __value), (__value))
__inline_forward_c(long double,scalbnl, (long double __x, int __n),
		   (__x, __n))
__inline_forward_c(int,isnanl, (long double __value), (__value))
__inline_forward_c(int,ilogbl, (long double __value), (__value))

#endif /* __USE_MISC */

#undef __inline_forward
#undef __inline_forward_c

#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */

#endif	/* GCC.  */
