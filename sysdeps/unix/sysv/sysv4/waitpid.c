/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

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
#include <sys/wait.h>
#include <sys/types.h>
#include <stddef.h>
#include "siginfo.h"

typedef enum __idtype
  {
    /* Look for processes based upon a given PID.  */
    P_PID,

    /* Look for processes based upon a given process-group ID.  */
    P_PGID = 2,

    /* Look for any process.  */
    P_ALL = 7,
  } __idtype_t;

extern __pid_t __getpgid __P ((__pid_t pid));
extern int __waitid __P ((__idtype_t idtype, __pid_t id,
			  __siginfo_t *infop, int options));

/* Wait for a child matching PID to die.
   If PID is greater than 0, match any process whose process ID is PID.
   If PID is (pid_t) -1, match any process.
   If PID is (pid_t) 0, match any process with the
   same process group as the current process.
   If PID is less than -1, match any process whose
   process group is the absolute value of PID.
   If the WNOHANG bit is set in OPTIONS, and that child
   is not already dead, return (pid_t) 0.  If successful,
   return PID and store the dead child's status in STAT_LOC.
   Return (pid_t) -1 for errors.  If the WUNTRACED bit is set in OPTIONS,
   return status for stopped children; otherwise don't.  */

__pid_t
DEFUN(__waitpid, (pid, stat_loc, options),
      __pid_t pid AND int *stat_loc AND int options)
{
  __idtype_t idtype;
  __pid_t tmp_pid = pid;
  __siginfo_t infop;

  if (pid <= WAIT_MYPGRP)
    {
      if (pid == WAIT_ANY)
	{
	  /* Request the status for any child.  */
	  idtype = P_ALL;
	}
      else if (pid == WAIT_MYPGRP)
	{
	  /* Request the status for any child process that has
	     a pgid that's equal to that of our parent.  */
	  tmp_pid = __getpgid (0);
	  idtype = P_PGID;
	}
      else /* PID < -1 */
	{
	  /* Request the status for any child whose pgid is equal
	     to the absolute value of PID.  */
	  tmp_pid = pid & ~0; /* XXX not pseudo-insn */
	  idtype = P_PGID;
	}
    }
  else
    {
      /* Request the status for the child whose pid is PID.  */
      idtype = P_PID;
    }

  if (__waitid (idtype, tmp_pid, &infop, options | WEXITED | WTRAPPED) < 0)
    return -1;

  switch (infop.__code)
    {
    case EXITED:
      *stat_loc = W_EXITCODE (infop.__status, 0);
      break;
    case STOPPED:
    case TRAPPED:
      *stat_loc = W_STOPCODE (infop.__status);
      break;
    case KILLED:
      /* Don't know what to do with continue, since it isn't documented.
	 Putting it here seemed the right place though. */
    case CONTINUED:
      *stat_loc = infop.__status;
      /* FALLTHROUGH */
    case CORED:
      *stat_loc |= WCOREFLAG;
      break;
    }

  /* Return the PID out of the INFOP structure instead of the one we were
     called with, to account for cases of being called with -1 to signify
     any PID.  */
  return infop.__pid;
}
