/* Declarations for math functions.
Copyright (C) 1991, 92, 93, 95, 96 Free Software Foundation, Inc.
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
  __MATHDECL (_Mdouble_, function,suffix, args)
#define __MATHDECL(type, function,suffix, args) \
  __MATHDECL_1(type, function,suffix, args); \
  __MATHDECL_1(type, __##function,suffix, args)
#define __MATHDECL_1(type, function,suffix, args) \
  extern type __MATH_PRECNAME(function,suffix) args

#define _Mdouble_ 		double
#define __MATH_PRECNAME(name,r)	name##r
#include <mathcalls.h>
#undef	_Mdouble_
#undef	__MATH_PRECNAME

#ifdef __USE_MISC
/* Include the file of declarations again, this type using `float'
   instead of `double' and appending f to each function name.  */

#define _Mdouble_ 		float
#define __MATH_PRECNAME(name,r)	name##f##r
#include <mathcalls.h>
#undef	_Mdouble_
#undef	__MATH_PRECNAME
#endif


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
   of the exceptional condition.  */
struct exception
  {
    int type;
    char *name;
    double arg1;
    double arg2;
    double retval;
  };

extern int matherr __P ((struct exception *));

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

#endif


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
