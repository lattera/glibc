/* Store current representation for exceptions (soft-float edition).
   Copyright (C) 2002-2012 Free Software Foundation, Inc.
   Contributed by Aldy Hernandez <aldyh@redhat.com>, 2002.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include "soft-fp.h"
#include "soft-supp.h"
#include <bp-sym.h>

int
__fegetexceptflag (fexcept_t *flagp, int excepts)
{
  *flagp = (fexcept_t) __sim_exceptions  & excepts & FE_ALL_EXCEPT;

  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__fegetexceptflag, __old_fegetexceptflag)
compat_symbol (libm, BP_SYM (__old_fegetexceptflag), BP_SYM (fegetexceptflag), GLIBC_2_1);
#endif

versioned_symbol (libm, BP_SYM (__fegetexceptflag), BP_SYM (fegetexceptflag), GLIBC_2_2);
