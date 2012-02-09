/* Copyright (C) 1998,2000,2002,2003,2005,2006 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <sys/types.h>

#include <linux/posix_types.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

#ifdef __NR_getresgid

# ifdef __NR_getresgid32
#  if __ASSUME_32BITUIDS == 0
/* This variable is shared with all files that need to check for 32bit
   uids.  */
extern int __libc_missing_32bit_uids;
#  endif
# endif /* __NR_getresgid32 */


int
__getresgid (gid_t *rgid, gid_t *egid, gid_t *sgid)
{
# if __ASSUME_32BITUIDS > 0
  return INLINE_SYSCALL (getresgid32, 3, CHECK_1 (rgid),
			 CHECK_1 (egid), CHECK_1 (sgid));
# else
  __kernel_gid_t k_rgid, k_egid, k_sgid;
  int result;
#  ifdef __NR_getresgid32
  if (__libc_missing_32bit_uids <= 0)
    {
      int r;
      int saved_errno = errno;

      r = INLINE_SYSCALL (getresgid32, 3, CHECK_1 (rgid),
			  CHECK_1 (egid), CHECK_1 (sgid));
      if (r == 0 || errno != ENOSYS)
	return r;

      __set_errno (saved_errno);
      __libc_missing_32bit_uids = 1;
    }
#  endif /* __NR_getresgid32 */

  result = INLINE_SYSCALL (getresgid, 3, __ptrvalue (&k_rgid),
			   __ptrvalue (&k_egid), __ptrvalue (&k_sgid));

  if (result == 0)
    {
      *rgid = (gid_t) k_rgid;
      *egid = (gid_t) k_egid;
      *sgid = (gid_t) k_sgid;
    }

  return result;
# endif
}
libc_hidden_def (__getresgid)
weak_alias (__getresgid, getresgid)

#else
# include <posix/getresgid.c>
#endif
