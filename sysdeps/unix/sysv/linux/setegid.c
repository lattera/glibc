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
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __NR_setresgid

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

  /* First try the syscall.  */
  result = __setresgid (-1, gid, -1);
  if (result == -1 && errno == ENOSYS)
    /* No system call available.  Use emulation.  This may not work
       since `setregid' also sets the saved group ID when GID is not
       equal to the real group ID, making it impossible to switch back. */
    result = __setregid (-1, gid);

  return result;
}
#else
# include <sysdeps/unix/bsd/setegid.c>
#endif
