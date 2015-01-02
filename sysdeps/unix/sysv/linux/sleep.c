/* Implementation of the POSIX sleep function using nanosleep.
   Copyright (C) 1996-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include <errno.h>
#include <time.h>
#include <signal.h>
#include <string.h>	/* For the real memset prototype.  */
#include <unistd.h>
#include <sys/param.h>
#include <nptl/pthreadP.h>


#if 0
static void
cl (void *arg)
{
  (void) __sigprocmask (SIG_SETMASK, arg, (sigset_t *) NULL);
}
#endif


/* We are going to use the `nanosleep' syscall of the kernel.  But the
   kernel does not implement the stupid SysV SIGCHLD vs. SIG_IGN
   behaviour for this syscall.  Therefore we have to emulate it here.  */
unsigned int
__sleep (unsigned int seconds)
{
  const unsigned int max
    = (unsigned int) (((unsigned long int) (~((time_t) 0))) >> 1);
  struct timespec ts;
  sigset_t set, oset;
  unsigned int result;

  /* This is not necessary but some buggy programs depend on this.  */
  if (__glibc_unlikely (seconds == 0))
    {
#ifdef CANCELLATION_P
      CANCELLATION_P (THREAD_SELF);
#endif
      return 0;
    }

  ts.tv_sec = 0;
  ts.tv_nsec = 0;
 again:
  if (sizeof (ts.tv_sec) <= sizeof (seconds))
    {
      /* Since SECONDS is unsigned assigning the value to .tv_sec can
	 overflow it.  In this case we have to wait in steps.  */
      ts.tv_sec += MIN (seconds, max);
      seconds -= (unsigned int) ts.tv_sec;
    }
  else
    {
      ts.tv_sec = (time_t) seconds;
      seconds = 0;
    }

  /* Linux will wake up the system call, nanosleep, when SIGCHLD
     arrives even if SIGCHLD is ignored.  We have to deal with it
     in libc.  We block SIGCHLD first.  */
  __sigemptyset (&set);
  __sigaddset (&set, SIGCHLD);
  if (__sigprocmask (SIG_BLOCK, &set, &oset))
    return -1;

  /* If SIGCHLD is already blocked, we don't have to do anything.  */
  if (!__sigismember (&oset, SIGCHLD))
    {
      int saved_errno;
      struct sigaction oact;

      __sigemptyset (&set);
      __sigaddset (&set, SIGCHLD);

      /* We get the signal handler for SIGCHLD.  */
      if (__sigaction (SIGCHLD, (struct sigaction *) NULL, &oact) < 0)
	{
	  saved_errno = errno;
	  /* Restore the original signal mask.  */
	  (void) __sigprocmask (SIG_SETMASK, &oset, (sigset_t *) NULL);
	  __set_errno (saved_errno);
	  return -1;
	}

      /* Note the sleep() is a cancellation point.  But since we call
	 nanosleep() which itself is a cancellation point we do not
	 have to do anything here.  */
      if (oact.sa_handler == SIG_IGN)
	{
	  //__libc_cleanup_push (cl, &oset);

	  /* We should leave SIGCHLD blocked.  */
	  while (1)
	    {
	      result = __nanosleep (&ts, &ts);

	      if (result != 0 || seconds == 0)
		break;

	      if (sizeof (ts.tv_sec) <= sizeof (seconds))
		{
		  ts.tv_sec = MIN (seconds, max);
		  seconds -= (unsigned int) ts.tv_nsec;
		}
	    }

	  //__libc_cleanup_pop (0);

	  saved_errno = errno;
	  /* Restore the original signal mask.  */
	  (void) __sigprocmask (SIG_SETMASK, &oset, (sigset_t *) NULL);
	  __set_errno (saved_errno);

	  goto out;
	}

      /* We should unblock SIGCHLD.  Restore the original signal mask.  */
      (void) __sigprocmask (SIG_SETMASK, &oset, (sigset_t *) NULL);
    }

  result = __nanosleep (&ts, &ts);
  if (result == 0 && seconds != 0)
    goto again;

 out:
  if (result != 0)
    /* Round remaining time.  */
    result = seconds + (unsigned int) ts.tv_sec + (ts.tv_nsec >= 500000000L);

  return result;
}
weak_alias (__sleep, sleep)
