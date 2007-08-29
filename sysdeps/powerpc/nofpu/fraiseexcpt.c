/* Raise given exceptions (soft-float edition).
   Copyright (C) 2002 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "soft-fp.h"
#include "soft-supp.h"
#include <signal.h>
#include <bp-sym.h>

#undef feraiseexcept
int
__feraiseexcept (int x)
{
  __sim_exceptions |= x;
  if (x & ~__sim_disabled_exceptions)
    raise (SIGFPE);
  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__feraiseexcept, __old_feraiseexcept)
compat_symbol (libm, BP_SYM (__old_feraiseexcept), BP_SYM (feraiseexcept), GLIBC_2_1);
#endif

libm_hidden_ver (__feraiseexcept, feraiseexcept)
versioned_symbol (libm, BP_SYM (__feraiseexcept), BP_SYM (feraiseexcept), GLIBC_2_2);
