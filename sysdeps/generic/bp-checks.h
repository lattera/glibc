/* Bounded-pointer checking macros for C.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Greg McGary <greg@mcgary.org>

   This file is part of the GNU C Library.  Its master source is NOT part of
   the C library, however.  The master source lives in the GNU MP Library.

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

#ifndef _bp_checks_h_
# define _bp_checks_h_ 1

# if !__ASSEMBLER__

#  if __BOUNDED_POINTERS__

/* GKM FIXME: when gcc is ready, add real bounds checks */
#   define BOUNDS_VIOLATED (__builtin_trap (), 0)
extern int __ubp_memchr (const char *__unbounded, int, unsigned);

/* Verify that pointer's value >= low.  Return pointer value.  */
#   define CHECK_BOUNDS_LOW(ARG)				\
  (((__ptrvalue (ARG) < __ptrlow (ARG)) && BOUNDS_VIOLATED),	\
   __ptrvalue (ARG))

/* Verify that pointer's value < high.  Return pointer value.  */
#   define CHECK_BOUNDS_HIGH(ARG)				\
  (((__ptrvalue (ARG) > __ptrhigh (ARG)) && BOUNDS_VIOLATED),	\
   __ptrvalue (ARG))

/* Check bounds of a pointer seated to a single object.  */
#   define CHECK_1(ARG) CHECK_N ((ARG), 1)

/* Same as CHECK_1, but tolerate ARG == NULL.  */
#   define CHECK_1opt(ARG) CHECK_Nopt ((ARG), 1)

/* Check bounds of a pointer seated to an array of N objects.  */
#   define CHECK_N(ARG, N)				\
  (((__ptrvalue (ARG) < __ptrlow (ARG)			\
     || __ptrvalue (ARG) + (N) > __ptrhigh (ARG))	\
    && BOUNDS_VIOLATED), __ptrvalue (ARG))

/* Same as CHECK_N, but tolerate ARG == NULL.  */
#   define CHECK_Nopt(ARG, N)				\
  (((__ptrvalue (ARG)					\
     && (__ptrvalue (ARG) < __ptrlow (ARG)		\
	 || __ptrvalue (ARG) + (N) > __ptrhigh (ARG)))	\
    && BOUNDS_VIOLATED), __ptrvalue (ARG))

/* Check for NUL-terminator within string's bounds.  */
#   define CHECK_STRING(ARG)					\
  (((__ptrvalue (ARG) < __ptrlow (ARG)				\
     || !__ubp_memchr (__ptrvalue (ARG), '\0',			\
		       (__ptrhigh (ARG) - __ptrvalue (ARG))))	\
     && BOUNDS_VIOLATED),					\
   __ptrvalue (ARG))

#  else /* !__BOUNDED_POINTERS__ */

/* Do nothing if not compiling with -fbounded-pointers.  */

#   define BOUNDS_VIOLATED
#   define CHECK_BOUNDS_LOW(ARG) (ARG)
#   define CHECK_BOUNDS_HIGH(ARG) (ARG)
#   define CHECK_1(ARG) (ARG)
#   define CHECK_1opt(ARG) (ARG)
#   define CHECK_N(ARG, N) (ARG)
#   define CHECK_Nopt(ARG, N) (ARG)
#   define CHECK_STRING(ARG) (ARG)

#  endif /* !__BOUNDED_POINTERS__ */

#  if defined (_IOC_SIZESHIFT) && defined (_IOC_SIZEBITS)

/* Extract the size of the ioctl parameter argument and check its bounds.  */
#   define CHECK_IOCTL(ARG, CMD) \
  CHECK_N ((ARG), (((CMD) >> _IOC_SIZESHIFT) & ((1 << _IOC_SIZEBITS) - 1)))

#  else
#   define CHECK_IOCTL(ARG, CMD) __ptrvalue (ARG)
#  endif

# endif /* !__ASSEMBLER__ */

#endif /* _bp_checks_h_ */
