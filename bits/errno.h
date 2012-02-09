/* Copyright (C) 1991, 1994, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This file defines the `errno' constants.  */

#if !defined __Emath_defined && (defined _ERRNO_H || defined __need_Emath)
#undef	__need_Emath
#define	__Emath_defined	1

# define EDOM	XXX	<--- fill in what is actually needed
# define EILSEQ	XXX	<--- fill in what is actually needed
# define ERANGE	XXX	<--- fill in what is actually needed
#endif

#ifdef	_ERRNO_H
# error "Define here all the missing error messages for the port.  These"
# error "must match the numbers of the kernel."
# define Exxxx	XXX
...
#endif
