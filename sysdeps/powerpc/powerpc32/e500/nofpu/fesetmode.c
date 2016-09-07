/* Install given floating-point control modes.  e500 version.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#define SPEFSCR_STATUS 0xff3eff00

int
fesetmode (const femode_t *modep)
{
  fenv_union_t u;
  INTERNAL_SYSCALL_DECL (err);
  int r;

  u.fenv = *modep;
  unsigned int spefscr = fegetenv_register ();
  spefscr = (spefscr & SPEFSCR_STATUS) | (u.l[1] & ~SPEFSCR_STATUS);

  fesetenv_register (spefscr);
  r = INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC,
			u.l[0] | PR_FP_EXC_SW_ENABLE);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    return -1;

  return 0;
}
