/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
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
#include "kernel-features.h"

#ifdef __NR_setresuid

extern int __syscall_setresuid (__kernel_uid_t rgid, __kernel_uid_t egid,
				__kernel_uid_t sgid);

# ifdef __NR_setresuid32
extern int __syscall_setresuid32 (__kernel_uid32_t rgid, __kernel_uid32_t egid,
				  __kernel_uid32_t sgid);
#  if __ASSUME_32BITUIDS == 0
/* This variable is shared with all files that need to check for 32bit
   uids.  */
extern int __libc_missing_32bit_uids;
#  endif
# endif /* __NR_setresuid32 */

extern int __setresuid (uid_t ruid, uid_t euid, uid_t suid);

int
__setresuid (uid_t ruid, uid_t euid, uid_t suid)
{
# if __ASSUME_32BITUIDS > 0
  return INLINE_SYSCALL (setresuid32, 3, ruid, euid, suid);
# else
#  ifdef __NR_setresuid32
  if (__libc_missing_32bit_uids <= 0)
    {
      int result;
      int saved_errno = errno;

      result = INLINE_SYSCALL (setresuid32, 3, ruid, euid, suid);
      if (result == 0 || errno != ENOSYS)
	return result;

      __set_errno (saved_errno);
      __libc_missing_32bit_uids = 1;
    }
#  endif /* __NR_setresuid32 */

  if (((ruid + 1) > (uid_t) ((__kernel_uid_t) -1U))
      || ((euid + 1) > (uid_t) ((__kernel_uid_t) -1U))
      || ((suid + 1) > (uid_t) ((__kernel_uid_t) -1U)))
    {
      __set_errno (EINVAL);
      return -1;
    }

  return INLINE_SYSCALL (setresuid, 3, ruid, euid, suid);
# endif
}
weak_alias (__setresuid, setresuid)
#endif
