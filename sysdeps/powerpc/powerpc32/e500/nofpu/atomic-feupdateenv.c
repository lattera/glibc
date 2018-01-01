/* Install given floating-point environment and raise exceptions for
   atomic compound assignment.  e500 version.
   Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

#include <fenv_libc.h>
#include <stdlib.h>
#include <sysdep.h>
#include <sys/prctl.h>

void
__atomic_feupdateenv (const fenv_t *envp)
{
  int exc;
  fenv_union_t u;
  INTERNAL_SYSCALL_DECL (err);
  int r;

  /* Save the currently set exceptions.  */
  exc = fegetenv_register () & SPEFSCR_ALL_EXCEPT;

  u.fenv = *envp;

  fesetenv_register (u.l[1]);
  r = INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC,
			u.l[0] | PR_FP_EXC_SW_ENABLE);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    abort ();

  /* Raise (if appropriate) saved exceptions. */
  __feraiseexcept_soft (exc);
}
