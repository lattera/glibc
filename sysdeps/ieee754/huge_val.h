/* `HUGE_VAL' constants for IEEE 754 machines (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.
   Copyright (C) 1992, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#ifndef	   _HUGE_VAL_H
#define	   _HUGE_VAL_H	1

#include <features.h>
#include <sys/cdefs.h>
#include <endian.h>

/* IEEE positive infinity (-HUGE_VAL is negative infinity).  */

#if __BYTE_ORDER == __BIG_ENDIAN
#define	__HUGE_VAL_bytes	{ 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 }
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	__HUGE_VAL_bytes	{ 0, 0, 0, 0, 0, 0, 0xf0, 0x7f }
#endif

#define __huge_val_t	union { unsigned char __c[8]; double __d; }
#ifdef	__GNUC__
#define	HUGE_VAL	(__extension__ \
			 ((__huge_val_t) { __c: __HUGE_VAL_bytes }).__d)
#else	/* Not GCC.  */
static __huge_val_t __huge_val = { __HUGE_VAL_bytes };
#define	HUGE_VAL	(__huge_val.__d)
#endif	/* GCC.  */


/* ISO C 9X extensions: (float) HUGE_VALF and (long double) HUGE_VALL.  */

#ifdef __USE_ISOC9X

#if __BYTE_ORDER == __BIG_ENDIAN
#define	__HUGE_VALF_bytes	{ 0x7f, 0x80, 0, 0 }
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	__HUGE_VALF_bytes	{ 0, 0, 0x80, 0x7f }
#endif

#define __huge_valf_t	union { unsigned char __c[4]; float __f; }
#ifdef	__GNUC__
#define	HUGE_VALF	(__extension__ \
			 ((__huge_valf_t) { __c: __HUGE_VALF_bytes }).__f)
#else	/* Not GCC.  */
static __huge_valf_t __huge_valf = { __HUGE_VALF_bytes };
#define	HUGE_VALF	(__huge_valf.__f)
#endif	/* GCC.  */


/* Generally there is no separate `long double' format and it is the
   same as `double'.  */
#define HUGE_VALL HUGE_VAL

#endif	/* __USE_ISOC9X.  */

#endif	   /* huge_val.h */
