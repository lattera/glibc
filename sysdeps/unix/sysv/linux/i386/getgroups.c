/* Copyright (C) 1997, 1998, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/types.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <linux/posix_types.h>
#include <kernel-features.h>


extern int __syscall_getgroups (int, __kernel_gid_t *__unbounded);

#ifdef __NR_getgroups32
extern int __syscall_getgroups32 (int, __kernel_gid32_t *__unbounded);
# if __ASSUME_32BITUIDS == 0
/* This variable is shared with all files that need to check for 32bit
   uids.  */
extern int __libc_missing_32bit_uids;
# endif
#endif /* __NR_getgroups32 */

/* For Linux we must convert the array of groups from the format that the
   kernel returns.  */
int
__getgroups (int n, gid_t *groups)
{
#if __ASSUME_32BITUIDS > 0
  return INLINE_SYSCALL (getgroups32, 2, n, CHECK_N (groups, n));
#else
  if (__builtin_expect (n, 1) < 0)
    {
      __set_errno (EINVAL);
      return -1;
    }
  else
    {
      int i, ngids;
      __kernel_gid_t kernel_groups[n = MIN (n, __sysconf (_SC_NGROUPS_MAX))];
# ifdef __NR_getgroups32
      if (__libc_missing_32bit_uids <= 0)
	{
	  int result;
	  int saved_errno = errno;

	  result = INLINE_SYSCALL (getgroups32, 2, n, CHECK_N (groups, n));
	  if (result == 0 || errno != ENOSYS)
	    return result;

	  __set_errno (saved_errno);
	  __libc_missing_32bit_uids = 1;
	}
# endif /* __NR_getgroups32 */

      ngids = INLINE_SYSCALL (getgroups, 2, n, CHECK_N (kernel_groups, n));
      if (n != 0 && ngids > 0)
	for (i = 0; i < ngids; i++)
	  (__ptrvalue (groups))[i] = kernel_groups[i];
      return ngids;
    }
#endif
}

weak_alias (__getgroups, getgroups)
