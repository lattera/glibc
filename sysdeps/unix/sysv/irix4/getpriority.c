/* Copyright (C) 1994,96,97,2000,02, 2004 Free Software Foundation, Inc.
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
#include <sys/resource.h>
#include <sys/sysmp.h>

extern int __sysmp (int, ...);

/* Return the highest priority of any process specified by WHICH and WHO
   (see <sys/resource.h>); if WHO is zero, the current process, process group,
   or user (as specified by WHO) is used.  A lower priority number means higher
   priority.  Priorities range from PRIO_MIN to PRIO_MAX.  */
int
getpriority (which, who)
     enum __priority_which which;
     id_t who;
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

  __set_errno (EINVAL);
  return -1;
}
libc_hidden_def (getpriority)
