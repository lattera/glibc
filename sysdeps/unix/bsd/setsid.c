/* Copyright (C) 1991, 1995, 1996, 1997 Free Software Foundation, Inc.
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
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

/* Create a new session with the calling process as its leader.
   The process group IDs of the session and the calling process
   are set to the process ID of the calling process, which is returned.  */
int
__setsid ()
{
  pid_t pid = getpid ();
  int tty;
  int save = errno;

  if (__getpgid (pid) == pid)
    {
      /* Already the leader.  */
      __set_errno (EPERM);
      return -1;
    }

  if (setpgid (pid, pid) < 0)
    return -1;

  tty = open ("/dev/tty", 0);
  if (tty < 0)
    {
      __set_errno (save);
      return 0;
    }
  (void) __ioctl (tty, TIOCNOTTY, 0);
  (void) __close (tty);

  __set_errno (save);
  return 0;
}

weak_alias (__setsid, setsid)
