/* Copyright (C) 1991, 1992, 1993, 1996, 1997 Free Software Foundation, Inc.
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

#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>


/* SIGALRM signal handler for `sleep'.  This does nothing but return,
   but SIG_IGN isn't supposed to break `pause'.  */
static void
sleep_handler (int sig)
{
  return;
}

/* Make the process sleep for SECONDS seconds, or until a signal arrives
   and is not ignored.  The function returns the number of seconds less
   than SECONDS which it actually slept (zero if it slept the full time).
   If a signal handler does a `longjmp' or modifies the handling of the
   SIGALRM signal while inside `sleep' call, the handling of the SIGALRM
   signal afterwards is undefined.  There is no return value to indicate
   error, but if `sleep' returns SECONDS, it probably didn't work.  */
unsigned int
__sleep (unsigned int seconds)
{
  unsigned int remaining, slept;
  time_t before, after;
  sigset_t set, oset;
  struct sigaction act, oact;
  int save = errno;

  if (seconds == 0)
    return 0;

  /* Block SIGALRM signals while frobbing the handler.  */
  if (sigemptyset (&set) < 0 ||
      sigaddset (&set, SIGALRM) < 0 ||
      sigprocmask (SIG_BLOCK, &set, &oset))
    return seconds;

  act.sa_handler = sleep_handler;
  act.sa_flags = 0;
  act.sa_mask = oset;	/* execute handler with original mask */
  if (sigaction (SIGALRM, &act, &oact) < 0)
    return seconds;

  before = time ((time_t *) NULL);
  remaining = alarm (seconds);

  if (remaining > 0 && remaining < seconds)
    {
      /* The user's alarm will expire before our own would.
	 Restore the user's signal action state and let his alarm happen.  */
      (void) sigaction (SIGALRM, &oact, (struct sigaction *) NULL);
      alarm (remaining);	/* Restore sooner alarm.  */
      sigsuspend (&oset);	/* Wait for it to go off.  */
      after = time ((time_t *) NULL);
    }
  else
    {
      /* Atomically restore the old signal mask
	 (which had better not block SIGALRM),
	 and wait for a signal to arrive.  */
      sigsuspend (&oset);

      after = time ((time_t *) NULL);

      /* Restore the old signal action state.  */
      (void) sigaction (SIGALRM, &oact, (struct sigaction *) NULL);
    }

  /* Notice how long we actually slept.  */
  slept = after - before;

  /* Restore the user's alarm if we have not already past it.
     If we have, be sure to turn off the alarm in case a signal
     other than SIGALRM was what woke us up.  */
  (void) alarm (remaining > slept ? remaining - slept : 0);

  /* Restore the original signal mask.  */
  (void) sigprocmask (SIG_SETMASK, &oset, (sigset_t *) NULL);

  /* Restore the `errno' value we started with.
     Some of the calls we made might have failed, but we didn't care.  */
  __set_errno (save);

  return slept > seconds ? 0 : seconds - slept;
}
weak_alias (__sleep, sleep)
