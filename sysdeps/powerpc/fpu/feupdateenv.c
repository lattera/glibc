/* Install given floating-point environment and raise exceptions.
   Copyright (C) 1997,99,2000,01 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <fenv_libc.h>
#include <bp-sym.h>

int
__feupdateenv (const fenv_t *envp)
{
  fenv_union_t old, new;

  /* Save the currently set exceptions.  */
  new.fenv = *envp;
  old.fenv = fegetenv_register ();

  /* Copy the set exceptions from `old' to `new'.  */
  new.l[1] = (new.l[1] & 0xE00000FF) | (old.l[1] & 0x1FFFFF00);

  /* Atomically enable and raise (if appropriate) exceptions set in `new'. */
  fesetenv_register (new.fenv);

  /* Success.  */
  return 0;
}

#include <shlib-compat.h>
#if SHLIB_COMPAT (libm, GLIBC_2_1, GLIBC_2_2)
strong_alias (__feupdateenv, __old_feupdateenv)
compat_symbol (libm, BP_SYM (__old_feupdateenv), BP_SYM (feupdateenv), GLIBC_2_1);
#endif

versioned_symbol (libm, BP_SYM (__feupdateenv), BP_SYM (feupdateenv), GLIBC_2_2);
