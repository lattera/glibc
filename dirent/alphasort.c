/* Copyright (C) 1992-2015 Free Software Foundation, Inc.
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

/* We need to avoid the header declaration of alphasort64, because
   the types don't match alphasort and then the compiler will
   complain about the mismatch when we do the alias below.  */
#define alphasort64     __renamed_alphasort64

#include <dirent.h>

#undef  alphasort64

#include <string.h>

int
alphasort (const struct dirent **a, const struct dirent **b)
{
  return strcoll ((*a)->d_name, (*b)->d_name);
}

#ifdef _DIRENT_MATCHES_DIRENT64
weak_alias (alphasort, alphasort64)
#endif
