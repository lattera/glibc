/* Bounded-pointer checking macros for C.
   Copyright (C) 2000-2013 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Greg McGary <greg@mcgary.org>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _bp_checks_h_
#define _bp_checks_h_ 1

#if __BOUNDED_POINTERS__

# define BOUNDS_VIOLATED (__builtin_trap (), 0)

/* Verify that pointer's value >= low.  Return pointer value.  */
# define CHECK_BOUNDS_LOW(ARG)					\
  (((__ptrvalue (ARG) < __ptrlow (ARG)) && BOUNDS_VIOLATED),	\
   __ptrvalue (ARG))

/* Verify that pointer's value < high.  Return pointer value.  */
# define CHECK_BOUNDS_HIGH(ARG)				\
  (((__ptrvalue (ARG) > __ptrhigh (ARG)) && BOUNDS_VIOLATED),	\
   __ptrvalue (ARG))

# define _CHECK_N(ARG, N, COND)				\
  (((COND)						\
    && (__ptrvalue (ARG) < __ptrlow (ARG)		\
	|| __ptrvalue (ARG) + (N) > __ptrhigh (ARG))	\
    && BOUNDS_VIOLATED),				\
   __ptrvalue (ARG))

extern void *__unbounded __ubp_memchr (const void *__unbounded, int, unsigned);

# define _CHECK_STRING(ARG, COND)				\
  (((COND)							\
    && (__ptrvalue (ARG) < __ptrlow (ARG)			\
	|| !__ubp_memchr (__ptrvalue (ARG), '\0',			\
		      (__ptrhigh (ARG) - __ptrvalue (ARG))))	\
    && BOUNDS_VIOLATED),					\
   __ptrvalue (ARG))

/* Check bounds of a pointer seated to an array of N objects.  */
# define CHECK_N(ARG, N) _CHECK_N ((ARG), (N), 1)
/* Same as CHECK_N, but tolerate ARG == NULL.  */
# define CHECK_N_NULL_OK(ARG, N) _CHECK_N ((ARG), (N), __ptrvalue (ARG))

/* Check bounds of a pointer seated to a single object.  */
# define CHECK_1(ARG) CHECK_N ((ARG), 1)
/* Same as CHECK_1, but tolerate ARG == NULL.  */
# define CHECK_1_NULL_OK(ARG) CHECK_N_NULL_OK ((ARG), 1)

/* Check for NUL-terminator within string's bounds.  */
# define CHECK_STRING(ARG) _CHECK_STRING ((ARG), 1)
/* Same as CHECK_STRING, but tolerate ARG == NULL.  */
# define CHECK_STRING_NULL_OK(ARG) _CHECK_STRING ((ARG), __ptrvalue (ARG))

/* Return a bounded pointer with value PTR that satisfies CHECK_N (PTR, N).  */
# define BOUNDED_N(PTR, N) 				\
  ({ __typeof (PTR) __bounded _p_;			\
     __ptrvalue _p_ = __ptrlow _p_ = __ptrvalue (PTR);	\
     __ptrhigh _p_ = __ptrvalue _p_ + (N);		\
     _p_; })

#else /* !__BOUNDED_POINTERS__ */

/* Do nothing if not compiling with -fbounded-pointers.  */

# define BOUNDS_VIOLATED
# define CHECK_BOUNDS_LOW(ARG) (ARG)
# define CHECK_BOUNDS_HIGH(ARG) (ARG)
# define CHECK_1(ARG) (ARG)
# define CHECK_1_NULL_OK(ARG) (ARG)
# define CHECK_N(ARG, N) (ARG)
# define CHECK_N_NULL_OK(ARG, N) (ARG)
# define CHECK_STRING(ARG) (ARG)
# define BOUNDED_N(PTR, N) (PTR)

#endif /* !__BOUNDED_POINTERS__ */

#define BOUNDED_1(PTR) BOUNDED_N (PTR, 1)

#endif /* _bp_checks_h_ */
