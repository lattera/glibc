/* Copyright (C) 1999, 2002, 2004 Free Software Foundation, Inc.
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

#include <math.h>
#include <stdlib.h>
#include <xlocale.h>


extern double ____strtod_l_internal (const char *, char **, int, __locale_t);


/* There is no `long double' type, use the `double' implementations.  */
long double
____strtold_l_internal (const char *nptr, char **endptr, int group,
			__locale_t loc)
{
  return ____strtod_l_internal (nptr, endptr, group, loc);
}


long double
strtold (const char *nptr, char **endptr, __locale_t loc)
{
  return ____strtod_l_internal (nptr, endptr, 0, loc);
}
