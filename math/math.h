/* Copyright (C) 1991, 1992, 1993, 1995 Free Software Foundation, Inc.
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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

/*
 *	ANSI Standard: 4.5 MATHEMATICS	<math.h>
 */

#ifndef	_MATH_H

#define	_MATH_H	1
#include <features.h>

__BEGIN_DECLS

#define	__need_Emath
#include <errno.h>

/* Get machine-dependent HUGE_VAL value (returned on overflow).  */
#include <huge_val.h>

/* Get machine-dependent NAN value (returned for some domain errors).  */
#ifdef	 __USE_GNU
#include <nan.h>
#endif


/* Trigonometric functions.  */

/* Arc cosine of X.  */
extern double acos __P ((double __x)) __attribute__ ((__const__));
/* Arc sine of X.  */
extern double asin __P ((double __x)) __attribute__ ((__const__));
/* Arc tangent of X.  */
extern double atan __P ((double __x)) __attribute__ ((__const__));
/* Arc tangent of Y/X.  */
extern double atan2 __P ((double __y, double __x)) __attribute__ ((__const__));

/* Cosine of X.  */
extern double cos __P ((double __x)) __attribute__ ((__const__));
/* Sine of X.  */
extern double sin __P ((double __x)) __attribute__ ((__const__));
/* Tangent of X.  */
extern double tan __P ((double __x)) __attribute__ ((__const__));


/* Hyperbolic functions.  */

/* Hyperbolic cosine of X.  */
extern double cosh __P ((double __x)) __attribute__ ((__const__));
/* Hyperbolic sine of X.  */
extern double sinh __P ((double __x)) __attribute__ ((__const__));
/* Hyperbolic tangent of X.  */
extern double tanh __P ((double __x)) __attribute__ ((__const__));

#ifdef	__USE_MISC
/* Hyperbolic arc cosine of X.  */
extern double acosh __P ((double __x)) __attribute__ ((__const__));
/* Hyperbolic arc sine of X.  */
extern double asinh __P ((double __x)) __attribute__ ((__const__));
/* Hyperbolic arc tangent of X.  */
extern double atanh __P ((double __x)) __attribute__ ((__const__));
#endif

/* Exponential and logarithmic functions.  */

/* Exponentional function of X.  */
extern double exp __P ((double __x)) __attribute__ ((__const__));

/* Break VALUE into a normalized fraction and an integral power of 2.  */
extern double frexp __P ((double __value, int *__exp));

/* X times (two to the EXP power).  */
extern double ldexp __P ((double __x, int __exp)) __attribute__ ((__const__));

/* Natural logarithm of X.  */
extern double log __P ((double __x)) __attribute__ ((__const__));

/* Base-ten logarithm of X.  */
extern double log10 __P ((double __x)) __attribute__ ((__const__));

#ifdef	__USE_MISC
/* Return exp(X) - 1.  */
extern double __expm1 __P ((double __x)) __attribute__ ((__const__));
extern double expm1 __P ((double __x)) __attribute__ ((__const__));

/* Return log(1 + X).  */
extern double log1p __P ((double __x)) __attribute__ ((__const__));
#endif

/* Break VALUE into integral and fractional parts.  */
extern double modf __P ((double __value, double *__iptr));


/* Power functions.  */

/* Return X to the Y power.  */
extern double pow __P ((double __x, double __y)) __attribute__ ((__const__));

/* Return the square root of X.  */
extern double sqrt __P ((double __x)) __attribute__ ((__const__));

#ifdef	__USE_MISC
/* Return the cube root of X.  */
extern double cbrt __P ((double __x)) __attribute__ ((__const__));
#endif


/* Nearest integer, absolute value, and remainder functions.  */

/* Smallest integral value not less than X.  */
extern double ceil __P ((double __x)) __attribute__ ((__const__));

/* Absolute value of X.  */
extern double fabs __P ((double __x)) __attribute__ ((__const__));

/* Largest integer not greater than X.  */
extern double floor __P ((double __x)) __attribute__ ((__const__));

/* Floating-point modulo remainder of X/Y.  */
extern double fmod __P ((double __x, double __y)) __attribute__ ((__const__));


/* Return 0 if VALUE is finite or NaN, +1 if it
   is +Infinity, -1 if it is -Infinity.  */
extern int __isinf __P ((double __value)) __attribute__ ((__const__));

/* Return nonzero if VALUE is not a number.  */
extern int __isnan __P ((double __value)) __attribute__ ((__const__));

/* Return nonzero if VALUE is finite and not NaN.  */
extern int __finite __P ((double __value)) __attribute__ ((__const__));
#ifdef	__OPTIMIZE__
#define	__finite(value)	(!__isinf(value))
#endif

/* Deal with an infinite or NaN result.
   If ERROR is ERANGE, result is +Inf;
   if ERROR is - ERANGE, result is -Inf;
   otherwise result is NaN.
   This will set `errno' to either ERANGE or EDOM,
   and may return an infinity or NaN, or may do something else.  */
extern double __infnan __P ((int __error));

/* Return X with its signed changed to Y's.  */
extern double __copysign __P ((double __x, double __y))
     __attribute__ ((__const__));

/* Return X times (2 to the Nth power).  */
extern double __scalb __P ((double __x, int __n))
     __attribute__ ((__const__));

#ifdef	__OPTIMIZE__
#define	__scalb(x, n)	ldexp ((x), (n))
#endif

/* Return the remainder of X/Y.  */
extern double __drem __P ((double __x, double __y))
     __attribute__ ((__const__));

/* Return the base 2 signed integral exponent of X.  */
extern double __logb __P ((double __x)) __attribute__ ((__const__));

#ifdef	__USE_MISC

/* Return the integer nearest X in the direction of the
   prevailing rounding mode.  */
extern double __rint __P ((double __x)) __attribute__ ((__const__));
extern double rint __P ((double __x)) __attribute__ ((__const__));

/* Return `sqrt(X*X + Y*Y)'.  */
extern double hypot __P ((double __x, double __y)) __attribute__ ((__const__));

struct __cabs_complex
{
  double __x, __y;
};

/* Return `sqrt(X*X + Y*Y)'.  */
extern double cabs __P ((struct __cabs_complex)) __attribute__ ((__const__));

extern int isinf __P ((double __value)) __attribute__ ((__const__));
extern int isnan __P ((double __value)) __attribute__ ((__const__));
extern int finite __P ((double __value)) __attribute__ ((__const__));
extern double infnan __P ((int __error)) __attribute__ ((__const__));
extern double copysign __P ((double __x, double __y))
     __attribute__ ((__const__));
extern double scalb __P ((double __x, int __n)) __attribute__ ((__const__));
extern double drem __P ((double __x, double __y)) __attribute__ ((__const__));
extern double logb __P ((double __x)) __attribute__ ((__const__));

#ifdef	__OPTIMIZE__
#define	isinf(value)	__isinf(value)
#define	isnan(value)	__isnan(value)
#define	infnan(error)	__infnan(error)
#define	finite(value)	__finite(value)
#define	copysign(x, y)	__copysign((x), (y))
#define	scalb(x, n)	__scalb((x), (n))
#define	drem(x, y)	__drem((x), (y))
#define	logb(x)		__logb(x)
#endif /* Optimizing.  */

#endif /* Use misc.  */


#if 0
/* The "Future Library Directions" section of the
   ANSI Standard reserves these as `float' and
   `long double' versions of the above functions.  */

extern float acosf __P ((float __x)) __attribute__ ((__const__));
extern float asinf __P ((float __x)) __attribute__ ((__const__));
extern float atanf __P ((float __x)) __attribute__ ((__const__));
extern float atan2f __P ((float __y, float __x)) __attribute__ ((__const__));
extern float cosf __P ((float __x)) __attribute__ ((__const__));
extern float sinf __P ((float __x)) __attribute__ ((__const__));
extern float tanf __P ((float __x)) __attribute__ ((__const__));
extern float coshf __P ((float __x)) __attribute__ ((__const__));
extern float sinhf __P ((float __x)) __attribute__ ((__const__));
extern float tanhf __P ((float __x)) __attribute__ ((__const__));
extern float expf __P ((float __x)) __attribute__ ((__const__));
extern float frexpf __P ((float __value, int *__exp));
extern float ldexpf __P ((float __x, int __exp)) __attribute__ ((__const__));
extern float logf __P ((float __x)) __attribute__ ((__const__));
extern float log10f __P ((float __x)) __attribute__ ((__const__));
extern float modff __P ((float __value, float *__iptr));
extern float powf __P ((float __x, float __y)) __attribute__ ((__const__));
extern float sqrtf __P ((float __x)) __attribute__ ((__const__));
extern float ceilf __P ((float __x)) __attribute__ ((__const__));
extern float fabsf __P ((float __x)) __attribute__ ((__const__));
extern float floorf __P ((float __x)) __attribute__ ((__const__));
extern float fmodf __P ((float __x, float __y)) __attribute__ ((__const__));

extern __long_double_t acosl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t asinl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t atanl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t atan2l __P ((__long_double_t __y, __long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t cosl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t sinl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t tanl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t coshl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t sinhl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t tanhl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t expl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t frexpl __P ((__long_double_t __value, int *__exp));
extern __long_double_t ldexpl __P ((__long_double_t __x, int __exp))
     __attribute__ ((__const__));
extern __long_double_t logl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t log10l __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t modfl __P ((__long_double_t __value,
				   __long_double_t *__ip));
extern __long_double_t powl __P ((__long_double_t __x, __long_double_t __y))
     __attribute__ ((__const__));
extern __long_double_t sqrtl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t ceill __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t fabsl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t floorl __P ((__long_double_t __x))
     __attribute__ ((__const__));
extern __long_double_t fmodl __P ((__long_double_t __x, __long_double_t __y))
     __attribute__ ((__const__));
#endif /* 0 */

/* Get machine-dependent inline versions (if there are any).  */
#include <__math.h>

__END_DECLS


#ifdef __USE_BSD
/* Some useful constants.  */
#define	M_E		2.7182818284590452354	/* e */
#define	M_LOG2E		1.4426950408889634074	/* log 2e */
#define	M_LOG10E	0.43429448190325182765	/* log 10e */
#define	M_LN2		0.69314718055994530942	/* log e2 */
#define	M_LN10		2.30258509299404568402	/* log e10 */
#define	M_PI		3.14159265358979323846	/* pi */
#define	M_PI_2		1.57079632679489661923	/* pi/2 */
#define	M_PI_4		0.78539816339744830962	/* pi/4 */
#define	M_1_PI		0.31830988618379067154	/* 1/pi */
#define	M_2_PI		0.63661977236758134308	/* 2/pi */
#define	M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
#define	M_SQRT2		1.41421356237309504880	/* sqrt(2) */
#define	M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif


#endif /* math.h  */
