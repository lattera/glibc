/* `HUGE_VAL' constants for ix86 (where it is infinity).
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

#ifndef _MATH_H
# error "Never use <bits/huge_val.h> directly; include <math.h> instead."
#endif

#include <features.h>

/* IEEE positive infinity (-HUGE_VAL is negative infinity).  */

#define	__HUGE_VAL_bytes	{ 0, 0, 0, 0, 0, 0, 0xf0, 0x7f }

#define __huge_val_t	union { unsigned char __c[8]; double __d; }
#ifdef	__GNUC__
# define HUGE_VAL	(__extension__ \
			 ((__huge_val_t) { __c: __HUGE_VAL_bytes }).__d)
#else	/* Not GCC.  */
static __huge_val_t __huge_val = { __HUGE_VAL_bytes };
# define HUGE_VAL	(__huge_val.__d)
#endif	/* GCC.  */


/* ISO C 9X extensions: (float) HUGE_VALF and (long double) HUGE_VALL.  */

#ifdef __USE_ISOC9X

# define __HUGE_VALF_bytes	{ 0, 0, 0x80, 0x7f }

# define __huge_valf_t	union { unsigned char __c[4]; float __f; }
# ifdef	__GNUC__
#  define HUGE_VALF	(__extension__ \
			 ((__huge_valf_t) { __c: __HUGE_VALF_bytes }).__f)
# else	/* Not GCC.  */
static __huge_valf_t __huge_valf = { __HUGE_VALF_bytes };
#  define HUGE_VALF	(__huge_valf.__f)
# endif	/* GCC.  */


# define __HUGE_VALL_bytes	{ 0, 0, 0, 0, 0, 0, 0, 0x80, 0xff, 0x7f, 0, 0 }

# define __huge_vall_t	union { unsigned char __c[12]; long double __ld; }
# ifdef	__GNUC__
#  define HUGE_VALL	(__extension__ \
			 ((__huge_vall_t) { __c: __HUGE_VALL_bytes }).__ld)
# else	/* Not GCC.  */
static __huge_vall_t __huge_vall = { __HUGE_VALL_bytes };
#  define HUGE_VALL	(__huge_vall.__ld)
# endif	/* GCC.  */

#endif	/* __USE_ISOC9X.  */
