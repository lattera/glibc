/* Clear floating-point exceptions for atomic compound assignment.
   e500 version.
   Copyright (C) 2004-2016 Free Software Foundation, Inc.
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
__atomic_feclearexcept (void)
{
  unsigned int fpescr, old_fpescr;

  /* Get the current state.  */
  old_fpescr = fpescr = fegetenv_register ();

  /* Clear the relevant bits.  */
  fpescr &= ~SPEFSCR_ALL_EXCEPT;

  /* Put the new state in effect.  */
  fesetenv_register (fpescr);

  /* Let the kernel know if the "invalid" or "underflow" bit was
     cleared.  */
  if (old_fpescr & (SPEFSCR_FINVS | SPEFSCR_FUNFS))
    {
      int pflags __attribute__ ((__unused__)), r;
      INTERNAL_SYSCALL_DECL (err);

      r = INTERNAL_SYSCALL (prctl, err, 2, PR_GET_FPEXC, &pflags);
      if (INTERNAL_SYSCALL_ERROR_P (r, err))
	abort ();
    }
}
