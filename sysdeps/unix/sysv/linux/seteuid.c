/* Copyright (C) 1998 Free Software Foundation, Inc.
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
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#ifdef __NR_setresuid

extern int __syscall_setresuid (uid_t ruid, uid_t euid, uid_t suid);

int
seteuid (uid_t uid)
{
  int result;

  if (uid == (uid_t) ~0)
    {
      __set_errno (EINVAL);
      return -1;
    }

  /* First try the syscall.  */
  result = __syscall_setresuid (-1, uid, -1);
  if (result == -1 && errno == ENOSYS)
    /* No system call available.  Use emulation.  This may not work
       since `setreuid' also sets the saved user ID when UID is not
       equal to the real user ID, making it impossible to switch back.  */
    result = __setreuid (-1, uid);

  return result;
}
#else
# include <sysdeps/unix/bsd/seteuid.c>
#endif
