/* Install given floating-point environment and raise exceptions
   (soft-float edition).
   Copyright (C) 2002, 2010 Free Software Foundation, Inc.
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
#include <signal.h>
#include <bp-sym.h>

int
__feupdateenv (const fenv_t *envp)
{
  int saved_exceptions;

  /* Save currently set exceptions.  */
  saved_exceptions = __sim_exceptions;

  /* Set environment.  */
  fesetenv (envp);

  /* Raise old exceptions.  */
  __sim_exceptions |= saved_exceptions;
  if (saved_exceptions & ~__sim_disabled_exceptions)
    raise (SIGFPE);

  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__feupdateenv, __old_feupdateenv)
compat_symbol (libm, BP_SYM (__old_feupdateenv), BP_SYM (feupdateenv), GLIBC_2_1);
#endif

libm_hidden_ver (__feupdateenv, feupdateenv)
versioned_symbol (libm, BP_SYM (__feupdateenv), BP_SYM (feupdateenv), GLIBC_2_2);
