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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stdlib.h>
#include <wchar.h>
#include <locale/localeinfo.h>


#ifndef FLOAT
# define FLOAT double
# ifdef USE_WIDE_CHAR
#  define STRTOF wcstod
#  define STRTOF_L __wcstod_l
# else
#  define STRTOF strtod
#  define STRTOF_L __strtod_l
# endif
#endif

#ifdef USE_WIDE_CHAR
# include <wctype.h>
# define STRING_TYPE wchar_t
#else
# define STRING_TYPE char
#endif

#define INTERNAL(x) INTERNAL1(x)
#define INTERNAL1(x) __##x##_internal


FLOAT
INTERNAL (STRTOF) (nptr, endptr, group)
     const STRING_TYPE *nptr;
     STRING_TYPE **endptr;
     int group;
{
  return INTERNAL(STRTOF_L) (nptr, endptr, group, _NL_CURRENT_LOCALE);
}
#if defined _LIBC
libc_hidden_def (INTERNAL (STRTOF))
#endif


FLOAT
#ifdef weak_function
weak_function
#endif
STRTOF (nptr, endptr)
     const STRING_TYPE *nptr;
     STRING_TYPE **endptr;
{
  return INTERNAL(STRTOF_L) (nptr, endptr, 0, _NL_CURRENT_LOCALE);
}
