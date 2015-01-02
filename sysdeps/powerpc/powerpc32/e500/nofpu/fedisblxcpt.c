/* Disable floating-point exceptions.  e500 version.
   Copyright (C) 2004-2015 Free Software Foundation, Inc.
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

#include <fenv_libc.h>
#include <sysdep.h>
#include <sys/prctl.h>

int
fedisableexcept (int excepts)
{
  int result = 0, pflags, r;
  INTERNAL_SYSCALL_DECL (err);

  r = INTERNAL_SYSCALL (prctl, err, 2, PR_GET_FPEXC, &pflags);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    return -1;

  /* Save old enable bits.  */
  result = __fexcepts_from_prctl (pflags);

  pflags &= ~__fexcepts_to_prctl (excepts);
  r = INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC,
			pflags | PR_FP_EXC_SW_ENABLE);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    return -1;

  /* If disabling signals for "inexact", also disable trapping to the
     kernel.  */
  if ((excepts & FE_INEXACT) != 0)
    {
      unsigned long fpescr;

      fpescr = fegetenv_register ();
      fpescr &= ~SPEFSCR_FINXE;
      fesetenv_register (fpescr);
    }

  return result;
}
