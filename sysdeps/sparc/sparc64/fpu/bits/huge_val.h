/* `HUGE_VAL' constants for IEEE 754 machines (where it is infinity).
   Used by <stdlib.h> and <math.h> functions for overflow.
   Copyright (C) 1992, 1995, 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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

#ifndef _MATH_H
# error "Never use <bits/huge_val.h> directly; include <math.h> instead."
#endif

#include <features.h>
#include <bits/wordsize.h>

/* IEEE positive infinity (-HUGE_VAL is negative infinity).  */

#ifdef	__GNUC__

# if __GNUC_PREREQ(2,96)

#  define HUGE_VAL (__extension__ 0x1.0p2047)

# else

#  define HUGE_VAL \
  (__extension__							      \
   ((union { unsigned __l __attribute__((__mode__(__DI__))); double __d; })   \
    { __l: 0x7ff0000000000000ULL }).__d)

# endif

#else /* not GCC */

typedef union { unsigned char __c[8]; double __d; } __huge_val_t;

# define __HUGE_VAL_bytes	{ 0x7f, 0xf0, 0, 0, 0, 0, 0, 0 }

static __huge_val_t __huge_val = { __HUGE_VAL_bytes };
# define HUGE_VAL	(__huge_val.__d)

#endif	/* GCC.  */


/* ISO C99 extensions: (float) HUGE_VALF and (long double) HUGE_VALL.  */

#ifdef __USE_ISOC99

# if __GNUC_PREREQ(2,96)

#  define HUGE_VALF (__extension__ 0x1.0p255f)
#  if __WORDSIZE == 32
#   define HUGE_VALL HUGE_VAL
#  else
/* Sparc64 uses IEEE 754 128bit long double */
#   define HUGE_VALL (__extension__ 0x1.0p32767L)
#  endif

# else

#  ifdef __GNUC__

#   define HUGE_VALF \
  (__extension__							      \
   ((union { unsigned __l __attribute__((__mode__(__SI__))); float __d; })    \
    { __l: 0x7f800000UL }).__d)

#  else /* not GCC */

typedef union { unsigned char __c[4]; float __f; } __huge_valf_t;

#   define __HUGE_VALF_bytes	{ 0x7f, 0x80, 0, 0 }

static __huge_valf_t __huge_valf = { __HUGE_VALF_bytes };
#   define HUGE_VALF	(__huge_valf.__f)

#  endif /* GCC.  */

#  if __WORDSIZE == 32

/* Sparc32 has IEEE 754 64bit long double */
#   define HUGE_VALL HUGE_VAL

#  else

/* Sparc64 uses IEEE 754 128bit long double */

#   ifdef __GNUC__

#    define HUGE_VALL \
  (__extension__									\
   ((union { struct { unsigned long __h, __l; } __i; long double __d; })		\
    { __i: { __h: 0x7fff000000000000UL, __l: 0 } }).__d)

#   else /* not GCC */

typedef union { unsigned char __c[16]; long double __d; } __huge_vall_t;

#    define __HUGE_VALL_bytes	{ 0x7f, 0xff, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }

static __huge_vall_t __huge_vall = { __HUGE_VALL_bytes };
#    define HUGE_VALL	(__huge_vall.__d)

#   endif /* GCC.  */

#  endif

# endif /* GCC 2.95.  */

#endif /* __USE_ISOC99.  */
