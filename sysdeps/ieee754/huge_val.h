/* `HUGE_VAL' constant for IEEE 754 machines (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.

Copyright (C) 1992, 1995 Free Software Foundation, Inc.
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


/* GNU extensions: (float) HUGE_VALf and (long double) HUGE_VALl.  */

#ifdef	__USE_GNU

#if __BYTE_ORDER == __BIG_ENDIAN
#define	__HUGE_VALf_bytes	{ 0x7f, 0x80, 0, 0 }
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	__HUGE_VALf_bytes	{ 0, 0, 0x80, 0x7f }
#endif

#define __huge_valf_t	union { unsigned char __c[4]; float __f; }
#ifdef	__GNUC__
#define	HUGE_VALf	(__extension__ \
			 ((__huge_valf_t) { __c: __HUGE_VALf_bytes }).__f)
#else	/* Not GCC.  */
static __huge_valf_t __huge_valf = { __HUGE_VALf_bytes };
#define	HUGE_VALf	(__huge_valf.__f)
#endif	/* GCC.  */

#if __BYTE_ORDER == __BIG_ENDIAN
#define	__HUGE_VALl_bytes	{ 0x7f, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
#endif
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define	__HUGE_VALl_bytes	{ 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0x7f, 0, 0 }
#endif

#define __huge_vall_t	union { unsigned char __c[12]; long double __ld; }
#ifdef	__GNUC__
#define	HUGE_VALl	(__extension__ \
			 ((__huge_vall_t) { __c: __HUGE_VALl_bytes }).__ld)
#else	/* Not GCC.  */
static __huge_vall_t __huge_vall = { __HUGE_VALl_bytes };
#define	HUGE_VALl	(__huge_vall.__ld)
#endif	/* GCC.  */

#endif	/* __USE_GNU.  */


#endif	   /* huge_val.h */
