/* Read decimal floating point numbers.
   This file is part of the GNU C Library.
   Copyright (C) 1995-2002, 2003, 2004 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@gnu.org>, 1995.

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

/* The actual implementation for all floating point sizes is in strtod.c.
   These macros tell it to produce the `float' version, `strtof'.  */

#define	FLOAT		float
#define	FLT		FLT
#ifdef USE_WIDE_CHAR
#define STRTOF		wcstof
#define STRTOF_L	__wcstof_l
#else
# define STRTOF		strtof
# define STRTOF_L	__strtof_l
#endif


#include "strtod.c"
