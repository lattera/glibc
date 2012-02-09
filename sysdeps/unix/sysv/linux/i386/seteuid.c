/* Copyright (C) 1998,2000,2002,2003,2004,2006 Free Software Foundation, Inc.
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
#include <setxid.h>
#include <kernel-features.h>


#ifdef __NR_setresuid
extern int __setresuid (uid_t ruid, uid_t euid, uid_t suid);
#endif

int
seteuid (uid_t uid)
{
  int result;

  if (uid == (uid_t) ~0)
    {
      __set_errno (EINVAL);
      return -1;
    }

#if __ASSUME_32BITUIDS > 0
  result = INLINE_SETXID_SYSCALL (setresuid32, 3, -1, uid, -1);
#else
  /* First try the syscall.  */
# ifdef __NR_setresuid
  result = __setresuid (-1, uid, -1);
#  if __ASSUME_SETRESUID_SYSCALL > 0
  if (0)
#  else
  if (result == -1 && errno == ENOSYS)
#  endif
    /* No system call available.  Use emulation.  This may not work
       since `setreuid' also sets the saved user ID when UID is not
       equal to the real user ID, making it impossible to switch back.  */
# endif
    result = __setreuid (-1, uid);
#endif

  return result;
}
libc_hidden_def (seteuid)
