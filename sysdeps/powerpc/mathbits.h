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

/* FIXME! This file describes properties of the compiler, not the machine;
   it should not be part of libc!

   FIXME! This file does not deal with the -fshort-double option of
   gcc! */

#ifdef __GNUC__
#if __STDC__ == 1

/* In GNU or ANSI mode, gcc leaves `float' expressions as-is, I think.  */
typedef float float_t;		/* `float' expressions are evaluated as
				   `float'.  */
typedef double double_t;	/* `double' expressions are evaluated as
				   `double'.  */

/* Signal that types stay as they were declared.  */
#define FLT_EVAL_METHOD	0

/* Define `INFINITY' as value of type `float_t'.  */
#define INFINITY	HUGE_VALF

#else 

/* For `gcc -traditional', `float' expressions are evaluated as `double'. */
typedef double float_t;		/* `float' expressions are evaluated as
				   `double'.  */
typedef double double_t;	/* `double' expressions are evaluated as
				   `double'.  */

/* Signal that both types are `double'.  */
#define FLT_EVAL_METHOD	1

/* Define `INFINITY' as value of type `float_t'.  */
#define INFINITY	HUGE_VAL

#endif
#else

/* Wild guess at types for float_t and double_t. */
typedef double float_t;
typedef double double_t;

/* Strange compiler, we don't know how it works.  */
#define FLT_EVAL_METHOD	-1

/* Define `INFINITY' as value of type `float_t'.  */
#define INFINITY	HUGE_VAL

#endif

#endif /* mathbits.h */
