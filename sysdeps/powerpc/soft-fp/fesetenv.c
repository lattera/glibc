/* Set floating point environment (soft-float edition).
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
#include <bp-sym.h>

extern int __sim_exceptions attribute_hidden;
extern int __sim_disabled_exceptions attribute_hidden;
extern int __sim_round_mode attribute_hidden;

int
__fesetenv (const fenv_t *envp)
{
  fenv_union_t u;

  u.fenv = *envp;
  __sim_exceptions = u.l[0] & FE_ALL_EXCEPT;
  __sim_round_mode = u.l[0] & 0x3;
  __sim_disabled_exceptions = u.l[1];
  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__fesetenv, __old_fesetenv)
compat_symbol (libm, BP_SYM (__old_fesetenv), BP_SYM (fesetenv), GLIBC_2_1);
#endif

libm_hidden_ver (__fesetenv, fesetenv)
versioned_symbol (libm, BP_SYM (__fesetenv), BP_SYM (fesetenv), GLIBC_2_2);
