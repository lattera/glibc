/* Pseudo implementation of waitid.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Zack Weinberg <zack@rabi.phys.columbia.edu>, 1997.

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

#include <assert.h>
#include <errno.h>
#include <signal.h>
#define __need_NULL
#include <stddef.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sysdep-cancel.h>


#ifdef DO_WAITID
# define OUR_WAITID DO_WAITID
#elif !defined NO_DO_WAITID
# define OUR_WAITID do_waitid
#endif

#ifdef OUR_WAITID
static int
OUR_WAITID (idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
  pid_t pid, child;
  int status;

  switch (idtype)
    {
    case P_PID:
      if(id <= 0)
	goto invalid;
      pid = (pid_t) id;
      break;
    case P_PGID:
      if (id < 0 || id == 1)
	goto invalid;
      pid = (pid_t) -id;
      break;
    case P_ALL:
      pid = -1;
      break;
    default:
    invalid:
      __set_errno (EINVAL);
      return -1;
    }

  /* Technically we're supposed to return EFAULT if infop is bogus,
     but that would involve mucking with signals, which is
     too much hassle.  User will have to deal with SIGSEGV/SIGBUS.
     We just check for a null pointer. */

  if (infop == NULL)
    {
      __set_errno (EFAULT);
      return -1;
    }

  /* This emulation using waitpid cannot support the waitid modes in which
     we do not reap the child, or match only stopped and not dead children.  */
  if (0
#ifdef WNOWAIT
      || (options & WNOWAIT)
#endif
#ifdef WEXITED
      || ((options & (WEXITED|WSTOPPED|WCONTINUED))
	  != (WEXITED | (options & WUNTRACED)))
#endif
      )
    {
      __set_errno (ENOTSUP);
      return -1;
    }

  /* Note the waitid() is a cancellation point.  But since we call
     waitpid() which itself is a cancellation point we do not have
     to do anything here.  */
  child = __waitpid (pid, &status,
		     options
#ifdef WEXITED
		     &~ WEXITED
#endif
		     );

  if (child == -1)
    /* `waitpid' set `errno' for us.  */
    return -1;

  if (child == 0)
    {
      /* The WHOHANG bit in OPTIONS is set and there are children available
	 but none has a status for us.  The XPG docs do not mention this
	 case so we clear the `siginfo_t' struct and return successfully.  */
      infop->si_signo = 0;
      infop->si_code = 0;
      return 0;
    }

  /* Decode the status field and set infop members... */
  infop->si_signo = SIGCHLD;
  infop->si_pid = child;
  infop->si_errno = 0;

  if (WIFEXITED (status))
    {
      infop->si_code = CLD_EXITED;
      infop->si_status = WEXITSTATUS (status);
    }
  else if (WIFSIGNALED (status))
    {
      infop->si_code = WCOREDUMP (status) ? CLD_DUMPED : CLD_KILLED;
      infop->si_status = WTERMSIG (status);
    }
  else if (WIFSTOPPED (status))
    {
      infop->si_code = CLD_STOPPED;
      infop->si_status = WSTOPSIG (status);
    }
#ifdef WIFCONTINUED
  else if (WIFCONTINUED (status))
    {
      infop->si_code = CLD_CONTINUED;
      infop->si_status = SIGCONT;
    }
#endif
  else
    /* Can't happen. */
    assert (! "What?");

  return 0;
}
#endif


int
__waitid (idtype_t idtype, id_t id, siginfo_t *infop, int options)
{
  if (SINGLE_THREAD_P)
    return do_waitid (idtype, id, infop, options);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = do_waitid (idtype, id, infop, options);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
weak_alias (__waitid, waitid)
strong_alias (__waitid, __libc_waitid)
