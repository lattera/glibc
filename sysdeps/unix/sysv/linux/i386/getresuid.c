/* Copyright (C) 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
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

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

#include <linux/posix_types.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include "kernel-features.h"

#ifdef __NR_getresuid

# ifdef __NR_getresuid32
#  if __ASSUME_32BITUIDS == 0
/* This variable is shared with all files that need to check for 32bit
   uids.  */
extern int __libc_missing_32bit_uids;
#  endif
# endif /* __NR_getresuid32 */

int
__getresuid (uid_t *ruid, uid_t *euid, uid_t *suid)
{
# if __ASSUME_32BITUIDS > 0
  return INLINE_SYSCALL (getresuid32, 3, CHECK_1 (ruid),
			 CHECK_1 (euid), CHECK_1 (suid));
# else
  __kernel_uid_t k_ruid, k_euid, k_suid;
  int result;
#  ifdef __NR_getresuid32
  if (__libc_missing_32bit_uids <= 0)
    {
      int r;
      int saved_errno = errno;

      r = INLINE_SYSCALL (getresuid32, 3, CHECK_1 (ruid),
			  CHECK_1 (euid), CHECK_1 (suid));
      if (r == 0 || errno != ENOSYS)
	return r;

      __set_errno (saved_errno);
      __libc_missing_32bit_uids = 1;
    }
#  endif /* __NR_getresuid32 */

  result = INLINE_SYSCALL (getresuid, 3, __ptrvalue (&k_ruid),
			   __ptrvalue (&k_euid), __ptrvalue (&k_suid));

  if (result == 0)
    {
      *ruid = (uid_t) k_ruid;
      *euid = (uid_t) k_euid;
      *suid = (uid_t) k_suid;
    }

  return result;
# endif
}
libc_hidden_def (__getresuid)
weak_alias (__getresuid, getresuid)

#else
# include <sysdeps/generic/getresuid.c>
#endif
