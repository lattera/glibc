/* Read decimal floating point numbers.
   This file is part of the GNU C Library.
   Copyright (C) 1995-2002,2003,2004,2006,2007 Free Software Foundation, Inc.
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

#include <bits/wordsize.h>

#ifdef __LONG_DOUBLE_MATH_OPTIONAL
# include <wchar.h>
# define NEW(x) NEW1(x)
# define NEW1(x) __new_##x
long double ____new_strtold_internal (const char *, char **, int);
long double __new_strtold (const char *, char **);
long double ____new_wcstold_internal (const wchar_t *, wchar_t **, int);
long double __new_wcstold (const wchar_t *, wchar_t **);
libc_hidden_proto (____new_strtold_internal)
libc_hidden_proto (____new_wcstold_internal)
libc_hidden_proto (__new_strtold)
libc_hidden_proto (__new_wcstold)
#else
# define NEW(x) x
#endif

#define	FLOAT		long double
#define	FLT		LDBL
#ifdef USE_WIDE_CHAR
# define STRTOF		NEW (wcstold)
# define STRTOF_L	__wcstold_l
#else
# define STRTOF		NEW (strtold)
# define STRTOF_L	__strtold_l
#endif

#include "strtod.c"

#ifdef __LONG_DOUBLE_MATH_OPTIONAL
# include <math_ldbl_opt.h>
# ifdef USE_WIDE_CHAR
long_double_symbol (libc, __new_wcstold, wcstold);
long_double_symbol (libc, ____new_wcstold_internal, __wcstold_internal);
libc_hidden_ver (____new_wcstold_internal, __wcstold_internal)
# else
long_double_symbol (libc, __new_strtold, strtold);
long_double_symbol (libc, ____new_strtold_internal, __strtold_internal);
libc_hidden_ver (____new_strtold_internal, __strtold_internal)
# endif
#endif
