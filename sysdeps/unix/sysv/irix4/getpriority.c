/* Copyright (C) 1994 Free Software Foundation, Inc.
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
#include <sys/resource.h>
#include <sys/sysmp.h>

extern int __sysmp __P ((int, ...));

/* Return the highest priority of any process specified by WHICH and WHO
   (see <sys/resource.h>); if WHO is zero, the current process, process group,
   or user (as specified by WHO) is used.  A lower priority number means higher
   priority.  Priorities range from PRIO_MIN to PRIO_MAX.  */
int
DEFUN(getpriority, (which, who),
      enum __priority_which which AND int who)
{
  switch (which)
    {
    case PRIO_PROCESS:
      return __sysmp (MP_SCHED, MPTS_GTNICE_PROC, who);
    case PRIO_PGRP:
      return __sysmp (MP_SCHED, MPTS_GTNICE_PGRP, who);
    case PRIO_USER:
      return __sysmp (MP_SCHED, MPTS_GTNICE_USER, who);
    }

  errno = EINVAL;
  return -1;
}
