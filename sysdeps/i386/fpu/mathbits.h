/* Copyright (C) 1997 Free Software Foundation, Inc.
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

#ifndef _MATHBITS_H
#define _MATHBITS_H	1

/* The ix87 FPUs evaluate all values in the 80 bit floating-point format
   which is also available for the user as `long double'.  Therefore
   we define:  */
typedef long double float_t;	/* `float' expressions are evaluated as
				   `long double'.  */
typedef long double double_t;	/* `double' expressions are evaluated as
				   `long double'.  */

/* Signal that both types are `long double'.  */
#define FLT_EVAL_METHOD	2

/* Define `INFINITY' as value of type `float_t'.  */
#define INFINITY	HUGE_VALL

#endif /* mathbits.h */
