/* Declarations for math functions.
   Copyright (C) 1991, 92, 93, 95, 96, 97 Free Software Foundation, Inc.
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

/*
 *	ISO C Standard: 4.5 MATHEMATICS	<math.h>
 */

#ifndef	_MATH_H

#define	_MATH_H	1
#include <features.h>

__BEGIN_DECLS

/* Get machine-dependent HUGE_VAL value (returned on overflow).
   On all IEEE754 machines, this is +Infinity.  */
#include <huge_val.h>

/* Get machine-dependent NAN value (returned for some domain errors).  */
#ifdef	 __USE_GNU
#include <nan.h>
#endif


/* The file <mathcalls.h> contains the prototypes for all the actual
   math functions.  These macros are used for those prototypes, so
   we can easily declare each function as both `name' and `__name',
   and can declare the float versions `namef' and `__namef'.  */

#define __MATHCALL(function,suffix, args)	\
  __MATHDECL (_Mdouble_,function,suffix, args)
#define __MATHDECL(type, function,suffix, args) \
  __MATHDECL_1(type, function,suffix, args); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args)
#define __MATHCALLX(function,suffix, args, attrib)	\
  __MATHDECLX (_Mdouble_,function,suffix, args, attrib)
#define __MATHDECLX(type, function,suffix, args, attrib) \
  __MATHDECL_1(type, function,suffix, args) __attribute__ (attrib); \
  __MATHDECL_1(type, __CONCAT(__,function),suffix, args) __attribute__ (attrib)
#define __MATHDECL_1(type, function,suffix, args) \
  extern type __MATH_PRECNAME(function,suffix) args

#define _Mdouble_ 		double
#define __MATH_PRECNAME(name,r)	__CONCAT(name,r)
#include <mathcalls.h>
#undef	_Mdouble_
#undef	__MATH_PRECNAME

#if defined __USE_MISC || defined __USE_ISOC9X


/* Include the file of declarations again, this time using `float'
   instead of `double' and appending f to each function name.  */

#ifndef _Mfloat_
#define _Mfloat_		float
#endif
#define _Mdouble_ 		_Mfloat_
#ifdef __STDC__
#define __MATH_PRECNAME(name,r)	name##f##r
#else
#define __MATH_PRECNAME(name,r) name/**/f/**/r
#endif
#include <mathcalls.h>
#undef	_Mdouble_
#undef	__MATH_PRECNAME

#if __STDC__ - 0 || __GNUC__ - 0
/* Include the file of declarations again, this time using `long double'
   instead of `double' and appending l to each function name.  */

#ifndef _Mlong_double_
#define _Mlong_double_		long double
#endif
#define _Mdouble_ 		_Mlong_double_
#ifdef __STDC__
#define __MATH_PRECNAME(name,r)	name##l##r
#else
#define __MATH_PRECNAME(name,r) name/**/l/**/r
#endif
#include <mathcalls.h>
#undef	_Mdouble_
#undef	__MATH_PRECNAME

#endif /* __STDC__ || __GNUC__ */

#endif	/* Use misc or ISO C 9X.  */
#undef	__MATHDECL_1
#undef	__MATHDECL
#undef	__MATHCALL


#if defined __USE_MISC || defined __USE_XOPEN || defined __USE_ISOC9X
/* This variable is used by `gamma' and `lgamma'.  */
extern int signgam;
#endif


/* ISO C 9X defines some generic macros which work on any data type.  */
#if __USE_ISOC9X

/* Get the architecture specific values describing the floating-point
   evaluation.  The following symbols will get defined:

     float_t	floating-point type at least as wide as `float' used
		to evaluate `float' expressions
     double_t	floating-point type at least as wide as `double' used
		to evaluate `double' expressions

     FLT_EVAL_METHOD
		Defined to
		  0	if `float_t' is `float' and `double_t' is `double'
		  1	if `float_t' and `double_t' are `double'
		  2	if `float_t' and `double_t' are `long double'
		  else	`float_t' and `double_t' are unspecified

     INFINITY	representation of the infinity value of type `float_t'
*/
#include <mathbits.h>

/* All floating-point numbers can be put in one of these categories.  */
enum
  {
    FP_NAN,
#define FP_NAN FP_NAN
    FP_INFINITE,
#define FP_INFINITE FP_INFINITE
    FP_ZERO,
#define FP_ZERO FP_ZERO
    FP_SUBNORMAL,
#define FP_SUBNORMAL FP_SUBNORMAL
    FP_NORMAL
#define FP_NORMAL FP_NORMAL
  };

/* Return number of classification appropriate for X.  */
#define fpclassify(x) \
     (sizeof (x) == sizeof (float) ?					      \
        __fpclassifyf (x)						      \
      : sizeof (x) == sizeof (double) ?					      \
        __fpclassify (x) : __fpclassifyl (x))

/* Return nonzero value if sign of X is negative.  */
#define signbit(x) \
     (sizeof (x) == sizeof (float) ?					      \
        __signbitf (x)							      \
      : sizeof (x) == sizeof (double) ?					      \
        __signbit (x) : __signbitl (x))

/* Return nonzero value if X is not +-Inf or NaN.  */
#define isfinite(x) \
     (sizeof (x) == sizeof (float) ?					      \
        __finitef (x)							      \
      : sizeof (x) == sizeof (double) ?					      \
        __finite (x) : __finitel (x))

/* Return nonzero value if X is neither zero, subnormal, Inf, nor NaN.  */
#define isnormal(x) (fpclassify (x) == FP_NORMAL)

/* Return nonzero value if X is a NaN.  We could use `fpclassify' but
   we already have this functions `__isnan' and it is faster.  */
#define isnan(x) \
     (sizeof (x) == sizeof (float) ?					      \
        __isnanf (x)							      \
      : sizeof (x) == sizeof (double) ?					      \
        __isnan (x) : __isnanl (x))


/* Conversion functions.  */

/* Round X to nearest integral value according to current rounding
   direction.  */
extern long int rinttol __P ((long double __x));
extern long long int rinttoll __P ((long double __x));

/* Round X to nearest integral value, rounding halfway cases away from
   zero.  */
extern long int roundtol __P ((long double __x));
extern long long int roundtoll __P ((long double __x));


/* Comparison macros.  */

/* Return nonzero value if X is greater than Y.  */
#define isgreater(x, y) (!isunordered ((x), (y)) && (x) > (y))

/* Return nonzero value if X is greater than or equal to Y.  */
#define isgreaterequal(x, y) (!isunordered ((x), (y)) && (x) >= (y))

/* Return nonzero value if X is less than Y.  */
#define isless(x, y) (!isunordered ((x), (y)) && (x) < (y))

/* Return nonzero value if X is less than or equal to Y.  */
#define islessequal(x, y) (!isunordered ((x), (y)) && (x) <= (y))

/* Return nonzero value if either X is less than Y or Y is less than X.  */
#define islessgreater(x, y) \
     (!isunordered ((x), (y)) && ((x) < (y) || (y) < (x)))

/* Return nonzero value if arguments are unordered.  */
#define isunordered(x, y) \
     (fpclassify (x) == FP_NAN || fpclassify (y) == FP_NAN)

#endif /* Use ISO C 9X.  */

#ifdef	__USE_MISC
/* Support for various different standard error handling behaviors.  */

typedef enum { _IEEE_ = -1, _SVID_, _XOPEN_, _POSIX_ } _LIB_VERSION_TYPE;

/* This variable can be changed at run-time to any of the values above to
   affect floating point error handling behavior (it may also be necessary
   to change the hardware FPU exception settings).  */
extern _LIB_VERSION_TYPE _LIB_VERSION;
#endif


#ifdef __USE_SVID
/* In SVID error handling, `matherr' is called with this description
   of the exceptional condition.

   We have a problem when using C++ since `exception' is reserved in
   C++.  */
#ifdef __cplusplus
struct __exception
#else
struct exception
#endif
  {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
  };

#ifdef __cplusplus
extern int __matherr __P ((struct __exception *));
extern int matherr __P ((struct __exception *));
#else
extern int __matherr __P ((struct exception *));
extern int matherr __P ((struct exception *));
#endif

#define X_TLOSS		1.41484755040568800000e+16

/* Types of exceptions in the `type' field.  */
#define	DOMAIN		1
#define	SING		2
#define	OVERFLOW	3
#define	UNDERFLOW	4
#define	TLOSS		5
#define	PLOSS		6

/* SVID mode specifies returning this large value instead of infinity.  */
#define HUGE		FLT_MAX
#include <float.h>		/* Defines FLT_MAX.  */

#else	/* !SVID */

#ifdef __USE_XOPEN
/* X/Open wants another strange constant.  */
#define MAXFLOAT	FLT_MAX
#include <float.h>
#endif

#endif	/* SVID */


#ifdef __USE_BSD

/* Some useful constants.  */
#define	M_E		_Mldbl(2.7182818284590452354)	/* e */
#define	M_LOG2E		_Mldbl(1.4426950408889634074)	/* log 2e */
#define	M_LOG10E	_Mldbl(0.43429448190325182765)	/* log 10e */
#define	M_LN2		_Mldbl(0.69314718055994530942)	/* log e2 */
#define	M_LN10		_Mldbl(2.30258509299404568402)	/* log e10 */
#define	M_PI		_Mldbl(3.14159265358979323846)	/* pi */
#define	M_PI_2		_Mldbl(1.57079632679489661923)	/* pi/2 */
#define	M_PI_4		_Mldbl(0.78539816339744830962)	/* pi/4 */
#define	M_1_PI		_Mldbl(0.31830988618379067154)	/* 1/pi */
#define	M_2_PI		_Mldbl(0.63661977236758134308)	/* 2/pi */
#define	M_2_SQRTPI	_Mldbl(1.12837916709551257390)	/* 2/sqrt(pi) */
#define	M_SQRT2		_Mldbl(1.41421356237309504880)	/* sqrt(2) */
#define	M_SQRT1_2	_Mldbl(0.70710678118654752440)	/* 1/sqrt(2) */

/* Our constants might specify more precision than `double' can represent.
   Use `long double' constants in standard and GNU C, where they are
   supported and the cast to `double'.  */
#if __STDC__ - 0 || __GNUC__ - 0
#define _Mldbl(x) x##L
#else	/* Traditional C.  */
#define _Mldbl(x) x
#endif	/* Standard or GNU C.  */

#endif


/* Get machine-dependent inline versions (if there are any).  */
#if (!defined __NO_MATH_INLINES && defined __OPTIMIZE__) \
    || defined __LIBC_M81_MATH_INLINES
#include <__math.h>
#endif


__END_DECLS


#endif /* math.h  */
