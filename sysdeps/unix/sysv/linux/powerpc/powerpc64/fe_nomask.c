/* Procedure definition for FE_NOMASK_ENV for Linux/ppc64.
   Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <fenv.h>
#include <errno.h>
#include <sysdep.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
#include "kernel-features.h"

const fenv_t *
__fe_nomask_env (void)
{
#if defined PR_SET_FPEXC && defined PR_FP_EXC_PRECISE
  int result;
  INTERNAL_SYSCALL_DECL (err);
  result = INTERNAL_SYSCALL (prctl, err, 2, PR_SET_FPEXC, PR_FP_EXC_PRECISE);
# ifndef __ASSUME_NEW_PRCTL_SYSCALL
  if (INTERNAL_SYSCALL_ERROR_P (result, err)
      && INTERNAL_SYSCALL_ERRNO (result, err) == EINVAL)
    __set_errno (ENOSYS);
# endif
#else  
  __set_errno (ENOSYS);
#endif
  return FE_ENABLED_ENV;
}
