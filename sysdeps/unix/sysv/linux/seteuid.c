/* Copyright (C) 1998, 1999, 2002, 2003, 2004 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include "kernel-features.h"
#include <pthread-functions.h>


#if defined __NR_setresuid || __ASSUME_SETRESUID_SYSCALL > 0

extern int __setresuid (uid_t ruid, uid_t euid, uid_t suid);

int
seteuid (uid_t uid)
{
  int result;

  if (uid == (uid_t) ~0)
    {
      __set_errno (EINVAL);
      return -1;
    }

# if __ASSUME_32BITUIDS > 0 && defined __NR_setresuid32
  result = INLINE_SYSCALL (setresuid32, 3, -1, uid, -1);
# else
  /* First try the syscall.  */
  result = INLINE_SYSCALL (setresuid, 3, -1, uid, -1);
#  if __ASSUME_SETRESUID_SYSCALL == 0
  if (result == -1 && errno == ENOSYS)
    /* No system call available.  Use emulation.  This may not work
       since `setreuid' also sets the saved user ID when UID is not
       equal to the real user ID, making it impossible to switch back.  */
    result = __setreuid (-1, uid);
#  endif
# endif

#if defined HAVE_PTR__NPTL_SETXID && !defined SINGLE_THREAD
  if (result == 0 && __libc_pthread_functions.ptr__nptl_setxid != NULL)
    {
      struct xid_command cmd;
# ifdef __NR_setresuid32
      cmd.syscall_no = __NR_setresuid32;
# else
      cmd.syscall_no = __NR_setresuid;
# endif
      cmd.id[0] = -1;
      cmd.id[1] = uid;
      cmd.id[2] = -1;
      __libc_pthread_functions.ptr__nptl_setxid (&cmd);
    }
#endif

  return result;
}
#ifndef seteuid
libc_hidden_def (seteuid)
#endif
#else
# include <sysdeps/unix/bsd/seteuid.c>
#endif
