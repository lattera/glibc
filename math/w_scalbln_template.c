/* Wrapper for __scalbln handles setting errno.
   Copyright (C) 2014-2017 Free Software Foundation, Inc.
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

#include <errno.h>
#include <math.h>
#include <math_private.h>

FLOAT
M_DECL_FUNC (__w_scalbln) (FLOAT x, long int n)
{
  if (!isfinite (x) || x == 0)
    return x + x;

  x = M_SUF (__scalbln) (x, n);

  if (!isfinite (x) || x == 0)
    __set_errno (ERANGE);

  return x;
}

/* Define strong_alias to nothing because we don't want
   declare_mgen_alias to create a strong alias for scalblnl.  */
#undef strong_alias
#define strong_alias(name, alias_name)
declare_mgen_alias (__w_scalbln, scalbln)
