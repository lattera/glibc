/* Copyright (C) 1994, 1995 Free Software Foundation, Inc.
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

#include <limits.h>
#include <hurd.h>
#include <hurd/resource.h>

/* Return the highest priority of any process specified by WHICH and WHO
   (see <sys/resource.h>); if WHO is zero, the current process, process group,
   or user (as specified by WHO) is used.  A lower priority number means higher
   priority.  Priorities range from PRIO_MIN to PRIO_MAX.  */
int
getpriority (enum __priority_which which, int who)
{
  error_t err, onerr;
  int maxpri = INT_MIN;
  struct procinfo *pip;		/* Just for sizeof.  */
  int pibuf[sizeof *pip + 2 * sizeof (pip->threadinfos[0])], *pi = pibuf;
  unsigned int pisize = sizeof pibuf / sizeof pibuf[0];

  error_t getonepriority (pid_t pid, struct procinfo *pip)
    {
      if (pip)
	onerr = 0;
      else
	{
	  int *oldpi = pi;
	  unsigned int oldpisize = pisize;
	  char *tw = 0;
	  size_t twsz = 0;
	  onerr = __USEPORT (PROC, __proc_getprocinfo (port, pid,
						       PI_FETCH_TASKINFO,
						       &pi, &pisize,
						       &tw, &twsz));
	  if (twsz)
	    __vm_deallocate (__mach_task_self (), (vm_address_t) tw, twsz);
	  if (pi != oldpi && oldpi != pibuf)
	    /* Old buffer from last call was not reused; free it.  */
	    __vm_deallocate (__mach_task_self (),
			     (vm_address_t) oldpi, oldpisize * sizeof pi[0]);
	  pip = (struct procinfo *) pi;
	}
      if (!onerr && pip->taskinfo.base_priority > maxpri)
	maxpri = pip->taskinfo.base_priority;
      return 0;
    }

  onerr = 0;
  err = _hurd_priority_which_map (which, who,
				  getonepriority, PI_FETCH_TASKINFO);

  if (pi != pibuf)
    __vm_deallocate (__mach_task_self (),
		     (vm_address_t) pi, pisize * sizeof pi[0]);

  if (!err && maxpri == INT_MIN)
    /* No error, but no pids found.  */
    err = onerr ?: ESRCH;

  if (err)
    return __hurd_fail (err);

  return MACH_PRIORITY_TO_NICE (maxpri);
}
