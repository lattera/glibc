/* Copyright (C) 1994-2016 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sysdep.h>

/* lockf is a simplified interface to fcntl's locking facilities.  */

int
lockf64 (int fd, int cmd, off64_t len64)
{
  struct flock64 fl64;
  int cmd64;
  int result;

  memset ((char *) &fl64, '\0', sizeof (fl64));
  fl64.l_whence = SEEK_CUR;
  fl64.l_start = 0;
  fl64.l_len = len64;

  switch (cmd)
    {
    case F_TEST:
      /* Test the lock: return 0 if FD is unlocked or locked by this process;
	 return -1, set errno to EACCES, if another process holds the lock.  */
      fl64.l_type = F_RDLCK;
      INTERNAL_SYSCALL_DECL (err);
      result = INTERNAL_SYSCALL (fcntl64, err, 3, fd, F_GETLK64, &fl64);
      if (__glibc_unlikely (INTERNAL_SYSCALL_ERROR_P (result, err)))
	return INLINE_SYSCALL_ERROR_RETURN_VALUE (INTERNAL_SYSCALL_ERRNO (result,
									  err));
      if (fl64.l_type == F_UNLCK || fl64.l_pid == __getpid ())
        return 0;
      return INLINE_SYSCALL_ERROR_RETURN_VALUE (EACCES);
    case F_ULOCK:
      fl64.l_type = F_UNLCK;
      cmd64 = F_SETLK64;
      break;
    case F_LOCK:
      fl64.l_type = F_WRLCK;
      cmd64 = F_SETLKW64;
      break;
    case F_TLOCK:
      fl64.l_type = F_WRLCK;
      cmd64 = F_SETLK64;
      break;

    default:
      return INLINE_SYSCALL_ERROR_RETURN_VALUE (EINVAL);
    }
  return INLINE_SYSCALL (fcntl64, 3, fd, cmd64, &fl64);
}
