/* Copyright (C) 1991, 1992, 1993, 1994, 1995 Free Software Foundation, Inc.
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

/* Get the process group ID of process PID.  */
int
DEFUN(__getpgid, (pid), pid_t pid)
{
  error_t err;
  pid_t pgrp;

  if (pid == 0)
    {
      /* Assume atomic word fetch and store, so don't lock _hurd_pid_lock.  */
      pgrp = _hurd_pgrp;
      err = 0;
    }
  else
    err = __USEPORT (PROC, __proc_getpgrp (port, pid, &pgrp));

  return err ? __hurd_fail (err) : pgrp;
}

weak_alias (__getpgid, getpgid)
