/* Copyright (C) 1995-1998,2000,2002,2003,2004 Free Software Foundation, Inc.
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


#ifdef __NR_setresgid
extern int __setresgid (uid_t rgid, uid_t egid, uid_t sgid);
#endif

int
setegid (gid)
     gid_t gid;
{
  int result;

  if (gid == (gid_t) ~0)
    {
      __set_errno (EINVAL);
      return -1;
    }

#if __ASSUME_32BITUIDS > 0
  result = INLINE_SETXID_SYSCALL (setresgid32, 3, -1, gid, -1);
#else
  /* First try the syscall.  */
# ifdef __NR_setresgid
  result = __setresgid (-1, gid, -1);
#  if __ASSUME_SETRESGID_SYSCALL > 0
  if (0)
#  else
  if (result == -1 && errno == ENOSYS)
#  endif
    /* No system call available.  Use emulation.  This may not work
       since `setregid' also sets the saved user ID when GID is not
       equal to the real user ID, making it impossible to switch back.  */
# endif
    result = __setregid (-1, gid);
#endif

  return result;
}
libc_hidden_def (setegid)
