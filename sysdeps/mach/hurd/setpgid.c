/* Copyright (C) 1993, 1994, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <unistd.h>
#include <hurd.h>
#include <hurd/port.h>

/* Set the process group ID of the process matching PID to PGID.
   If PID is zero, the current process's process group ID is set.
   If PGID is zero, the process ID of the process is used.  */
int
DEFUN(__setpgid, (pid, pgid), pid_t pid AND pid_t pgid)
{
  error_t err;
  unsigned int stamp;

  stamp = _hurd_pids_changed_stamp; /* Atomic fetch.  */

  if (err = __USEPORT (PROC, __proc_setpgrp (port, pid, pgid)))
    return __hurd_fail (err);

  if (pid == 0 || pid == _hurd_pid)
    /* Synchronize with the signal thread to make sure we have
       received and processed proc_newids before returning to the user.  */
    while (_hurd_pids_changed_stamp == stamp)
      {
#ifdef noteven
	/* XXX we have no need for a mutex, but cthreads demands one.  */
	__condition_wait (&_hurd_pids_changed_sync, NULL);
#else
	__swtch_pri(0);
#endif
      }

  return 0;

}

weak_alias (__setpgid, setpgid)
weak_alias (__setpgid, setpgrp)
