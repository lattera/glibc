/* Note a change to floating-point exceptions.
   Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

/* Inform the kernel of a change to floating-point exceptions.  */

void
__fe_note_change (void)
{
  int pflags, r;
  INTERNAL_SYSCALL_DECL (err);

  r = INTERNAL_SYSCALL (prctl, err, 2, PR_GET_FPEXC, &pflags);
  if (INTERNAL_SYSCALL_ERROR_P (r, err))
    return;
  if ((pflags & PR_FP_EXC_SW_ENABLE) == 0)
    INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC,
		      pflags | PR_FP_EXC_SW_ENABLE);
}

libm_hidden_def (__fe_note_change)
