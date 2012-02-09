/* Convert string representing a number to integer value, using given locale.
   Copyright (C) 1997, 1998, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <stddef.h>
#include <xlocale.h>


extern double ____wcstod_l_internal (const wchar_t *, wchar_t **, int,
				     __locale_t);
extern unsigned long long int ____wcstoull_l_internal (const wchar_t *,
						       wchar_t **, int, int,
						       __locale_t);

#define	USE_WIDE_CHAR	1

#include <stdlib/strtod_l.c>
