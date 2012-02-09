/* Copyright (C) 1997,1998,2000,2002,2004,2006,2011
   Free Software Foundation, Inc.
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

#include <errno.h>
#include <grp.h>
#include <unistd.h>
#include <sys/types.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <setxid.h>
#include <linux/posix_types.h>
#include <kernel-features.h>


#ifdef __NR_setgroups32
# if __ASSUME_32BITUIDS == 0
/* This variable is shared with all files that need to check for 32bit
   uids.  */
extern int __libc_missing_32bit_uids;
# endif
#endif /* __NR_setgroups32 */

/* Set the group set for the current user to GROUPS (N of them).  For
   Linux we must convert the array of groups into the format that the
   kernel expects.  */
int
setgroups (size_t n, const gid_t *groups)
{
#if __ASSUME_32BITUIDS > 0
  return INLINE_SETXID_SYSCALL (setgroups32, 2, n, CHECK_N (groups, n));
#else
  if (n > (size_t) __sysconf (_SC_NGROUPS_MAX))
    {
      __set_errno (EINVAL);
      return -1;
    }
  else
    {
      size_t i;
      __kernel_gid_t kernel_groups[n];

# ifdef __NR_setgroups32
      if (__libc_missing_32bit_uids <= 0)
	{
	  int result;
	  int saved_errno = errno;

	  result = INLINE_SETXID_SYSCALL (setgroups32, 2, n,
					  CHECK_N (groups, n));
	  if (result == 0 || errno != ENOSYS)
	    return result;

	  __set_errno (saved_errno);
	  __libc_missing_32bit_uids = 1;
	}
# endif /* __NR_setgroups32 */
      for (i = 0; i < n; i++)
	{
	  kernel_groups[i] = (__ptrvalue (groups))[i];
	  if (groups[i] != (gid_t) ((__kernel_gid_t) groups[i]))
	    {
	      __set_errno (EINVAL);
	      return -1;
	    }
	}

      return INLINE_SETXID_SYSCALL (setgroups, 2, n,
				    CHECK_N (kernel_groups, n));
    }
#endif
}
libc_hidden_def (setgroups)
