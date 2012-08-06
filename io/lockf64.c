/* Copyright (C) 1994-2012 Free Software Foundation, Inc.
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

/* lockf.c defines lockf64 as an lias if __OFF_T_MATCHES_OFF64_T.  */
#ifndef __OFF_T_MATCHES_OFF64_T

/* lockf is a simplified interface to fcntl's locking facilities.  */

int
lockf64 (int fd, int cmd, off64_t len64)
{
  struct flock fl;
  off_t len = (off_t) len64;

  if (len64 != (off64_t) len)
    {
      /* We can't represent the length.  */
      __set_errno (EOVERFLOW);
      return -1;
    }

  memset ((char *) &fl, '\0', sizeof (fl));

  /* lockf is always relative to the current file position.  */
  fl.l_whence = SEEK_CUR;
  fl.l_start = 0;
  fl.l_len = len;

  switch (cmd)
    {
    case F_TEST:
      /* Test the lock: return 0 if FD is unlocked or locked by this process;
	 return -1, set errno to EACCES, if another process holds the lock.  */
      fl.l_type = F_RDLCK;
      if (__fcntl (fd, F_GETLK, &fl) < 0)
	return -1;
      if (fl.l_type == F_UNLCK || fl.l_pid == __getpid ())
	return 0;
      __set_errno (EACCES);
      return -1;

    case F_ULOCK:
      fl.l_type = F_UNLCK;
      cmd = F_SETLK;
      break;
    case F_LOCK:
      fl.l_type = F_WRLCK;
      cmd = F_SETLKW;
      break;
    case F_TLOCK:
      fl.l_type = F_WRLCK;
      cmd = F_SETLK;
      break;

    default:
      __set_errno (EINVAL);
      return -1;
    }

  return __fcntl (fd, cmd, &fl);
}

#endif
