/* Prototype declarations for math functions; helper file for <math.h>.
Copyright (C) 1996 Free Software Foundation, Inc.
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

/* NOTE: Because of the special way this file is used by <math.h>, this
   file must NOT be protected from multiple inclusion as header files
   usually are.

   This file provides prototype declarations for the math functions.
   Most functions are declared using the macro:

   __MATHCALL (NAME,[_r], (ARGS...));

   This means there is a function `NAME' returning `double' and a function
   `NAMEf' returning `float'.  Each place `_Mdouble_' appears in the
   prototype, that is actually `double' in the prototype for `NAME' and
   `float' in the prototype for `NAMEf'.  Reentrant variant functions are
   called `NAME_r' and `NAMEf_r'.

   Functions returning other types like `int' are declared using the macro:

   __MATHDECL (TYPE, NAME,[_r], (ARGS...));

   This is just like __MATHCALL but for a function returning `TYPE'
   instead of `_Mdouble_'.  In all of these cases, there is still
   both a `NAME' and a `NAMEf' that takes `float' arguments.  */

#ifndef _MATH_H
 #error "Never include mathcalls.h directly; include <math.h> instead."
#endif


/* Trigonometric functions.  */

/* Arc cosine of X.  */
__MATHCALL (acos,, (_Mdouble_ __x));
/* Arc sine of X.  */
__MATHCALL (asin,, (_Mdouble_ __x));
/* Arc tangent of X.  */
__MATHCALL (atan,, (_Mdouble_ __x));
/* Arc tangent of Y/X.  */
__MATHCALL (atan2,, (_Mdouble_ __y, _Mdouble_ __x));

/* Cosine of X.  */
__MATHCALL (cos,, (_Mdouble_ __x));
/* Sine of X.  */
__MATHCALL (sin,, (_Mdouble_ __x));
/* Tangent of X.  */
__MATHCALL (tan,, (_Mdouble_ __x));


/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
__MATHCALL (cosh,, (_Mdouble_ __x));
/* Hyperbolic sine of X.  */
__MATHCALL (sinh,, (_Mdouble_ __x));
/* Hyperbolic tangent of X.  */
__MATHCALL (tanh,, (_Mdouble_ __x));

#ifdef	__USE_MISC
/* Hyperbolic arc cosine of X.  */
__MATHCALL (acosh,, (_Mdouble_ __x));
/* Hyperbolic arc sine of X.  */
__MATHCALL (asinh,, (_Mdouble_ __x));
/* Hyperbolic arc tangent of X.  */
__MATHCALL (atanh,, (_Mdouble_ __x));
#endif

/* Exponential and logarithmic functions.  */

/* Exponentional function of X.  */
__MATHCALL (exp,, (_Mdouble_ __x));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
__MATHCALL (frexp,, (_Mdouble_ __value, int *__exp));

/* X times (two to the EXP power).  */
__MATHCALL (ldexp,, (_Mdouble_ __x, int __exp));

/* Natural logarithm of X.  */
__MATHCALL (log,, (_Mdouble_ __x));

/* Base-ten logarithm of X.  */
__MATHCALL (log10,, (_Mdouble_ __x));

#ifdef	__USE_MISC
/* Return exp(X) - 1.  */
__MATHCALL (expm1,, (_Mdouble_ __x));

/* Return log(1 + X).  */
__MATHCALL (log1p,, (_Mdouble_ __x));
#endif

/* Break VALUE into integral and fractional parts.  */
__MATHCALL (modf,, (_Mdouble_ __value, _Mdouble_ *__iptr));


/* Power functions.  */

/* Return X to the Y power.  */
__MATHCALL (pow,, (_Mdouble_ __x, _Mdouble_ __y));

/* Return the square root of X.  */
__MATHCALL (sqrt,, (_Mdouble_ __x));

#ifdef	__USE_MISC
/* Return the cube root of X.  */
__MATHCALL (cbrt,, (_Mdouble_ __x));
#endif


/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
__MATHCALL (ceil,, (_Mdouble_ __x));

/* Absolute value of X.  */
__MATHCALL (fabs,, (_Mdouble_ __x));

/* Largest integer not greater than X.  */
__MATHCALL (floor,, (_Mdouble_ __x));

/* Floating-point modulo remainder of X/Y.  */
__MATHCALL (fmod,, (_Mdouble_ __x, _Mdouble_ __y));


#ifdef __USE_MISC

/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
__MATHDECL (int, isinf,, (_Mdouble_ __value));

/* Return nonzero if VALUE is not a number.  */
__MATHDECL (int, isnan,, (_Mdouble_ __value));

/* Return nonzero if VALUE is finite and not NaN.  */
__MATHDECL (int, finite,, (_Mdouble_ __value));

/* Deal with an infinite or NaN result.
   If ERROR is ERANGE, result is +Inf;
   if ERROR is - ERANGE, result is -Inf;
   otherwise result is NaN.
   This will set `errno' to either ERANGE or EDOM,
   and may return an infinity or NaN, or may do something else.  */
__MATHCALL (infnan,, (int __error));

/* Return X with its signed changed to Y's.  */
__MATHCALL (copysign,, (_Mdouble_ __x, _Mdouble_ __y));

/* Return X times (2 to the Nth power).  */
__MATHCALL (scalb,, (_Mdouble_ __x, _Mdouble_ __n));

/* Return X times (2 to the Nth power).  */
__MATHCALL (scalbn,, (_Mdouble_ __x, int __n));

/* Return the remainder of X/Y.  */
__MATHCALL (drem,, (_Mdouble_ __x, _Mdouble_ __y));

/* Return the base 2 signed integral exponent of X.  */
__MATHCALL (logb,, (_Mdouble_ __x));

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
__MATHCALL (rint,, (_Mdouble_ __x));

/* Return `sqrt(X*X + Y*Y)'.  */
__MATHCALL (hypot,, (_Mdouble_ __x, _Mdouble_ __y));

struct __MATH_PRECNAME(__cabs_complex,)
{
  _Mdouble_ x, y;
};

/* Return `sqrt(X*X + Y*Y)'.  */
__MATHCALL (cabs,, (struct __MATH_PRECNAME(__cabs_complex,)));


/* Return X + epsilon if X < Y, X - epsilon if X > Y.  */
__MATHCALL (nextafter,, (_Mdouble_ __x, _Mdouble_ __y));

/* Return the remainder of integer divison X / Y with infinite precision.  */
__MATHCALL (remainder,, (_Mdouble_ __x, _Mdouble_ __y));

/* Return the binary exponent of X, which must be nonzero.  */
__MATHDECL (int, ilogb,, (_Mdouble_ __x));

/* Return the fractional part of X after dividing out `ilogb (X)'.  */
__MATHCALL (significand,, (_Mdouble_ __x));



/* Error, gamma, and Bessel functions.  */
__MATHCALL (erf,, (_Mdouble_));
__MATHCALL (erfc,, (_Mdouble_));
__MATHCALL (gamma,, (_Mdouble_));
__MATHCALL (j0,, (_Mdouble_));
__MATHCALL (j1,, (_Mdouble_));
__MATHCALL (jn,, (int, _Mdouble_));
__MATHCALL (lgamma,, (_Mdouble_));
__MATHCALL (y0,, (_Mdouble_));
__MATHCALL (y1,, (_Mdouble_));
__MATHCALL (yn,, (int, _Mdouble_));

/* This variable is used by `gamma' and `lgamma'.  */
extern int signgam;

#ifdef __USE_REENTRANT

/* Reentrant versions of gamma and lgamma.  Those functions use the global
   variable `signgam'.  The reentrant versions instead take a pointer and
   store the value through it.  */
__MATHCALL (gamma,_r, (_Mdouble_, int *));
__MATHCALL (lgamma,_r, (_Mdouble_, int *));
#endif

#endif /* Use misc.  */
