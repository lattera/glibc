/* Copyright (C) 1998, 2000, 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <setxid.h>
#include "kernel-features.h"


#if defined __NR_setresgid || __ASSUME_SETRESGID_SYSCALL > 0

extern int __setresgid (gid_t rgid, gid_t egid, gid_t sgid);

int
setegid (gid_t gid)
{
  int result;

  if (gid == (gid_t) ~0)
    {
      __set_errno (EINVAL);
      return -1;
    }

# if __ASSUME_32BITUIDS > 0 && defined __NR_setresgid32
  result = INLINE_SETXID_SYSCALL (setresgid32, 3, -1, gid, -1);
# else
  /* First try the syscall.  */
  result = INLINE_SETXID_SYSCALL (setresgid, 3, -1, gid, -1);
#  if __ASSUME_SETRESGID_SYSCALL == 0
  if (result == -1 && errno == ENOSYS)
    /* No system call available.  Use emulation.  This may not work
       since `setregid' also sets the saved group ID when GID is not
       equal to the real group ID, making it impossible to switch back. */
    result = __setregid (-1, gid);
#  endif
# endif

  return result;
}
#ifndef setegid
libc_hidden_def (setegid)
#endif
#else
# include <sysdeps/unix/bsd/setegid.c>
#endif
