/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef	__need_HUGE_VAL

/* Floating-point constants for Vaxen.  */

#define	FLT_RADIX	2

#define	FLT_ROUNDS	_FLT_ROUNDS_TONEAREST

#define	FLT_MANT_DIG	23
#define	DBL_MANT_DIG	55
#define	LDBL_MANT_DIG	55

#define	FLT_DIG		6
#define	DBL_DIG		16
#define	LDBL_DIG	16

#define	FLT_MIN_EXP	(-128)
#define	DBL_MIN_EXP	(-128)
#define	LDBL_MIN_EXP	(-128)

#define	FLT_MIN_10_EXP	(-38)
#define	DBL_MIN_10_EXP	(-38)
#define	LDBL_MIN_10_EXP	(-38)

#define	FLT_MAX_EXP	127
#define	DBL_MAX_EXP	127
#define	LDBL_MAX_EXP	127

#define	FLT_MAX_10_EXP	38
#define	DBL_MAX_10_EXP	38
#define	LDBL_MAX_10_EXP	38

#define	FLT_MAX		1.7014116e38
#define	DBL_MAX		1.70141182460469227e38
#define	LDBL_MAX	DBL_MAX

#define	FLT_EPSILON	2.384186e-7
#define	DBL_EPSILON	5.55111512312578270e-17
#define	LDBL_EPSILON	DBL_EPSILON

#define	FLT_MIN		0.2938736e-38
#define	DBL_MIN		0.29387358770557187e-38
#define	LDBL_MIN	DBL_MIN

#else	/* Need HUGE_VAL.  */

/* Used by <stdlib.h> and <math.h> functions for overflow.	*/
#define	HUGE_VAL	1.70141182460469227e38

#endif	/* Don't need HUGE_VAL.  */
