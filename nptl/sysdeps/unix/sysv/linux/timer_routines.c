/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <sysdep.h>
#include <kernel-features.h>
#include "kernel-posix-timers.h"


#ifdef __NR_timer_create
/* Helper thread to call the user-provided function.  */
static void *
timer_sigev_thread (void *arg)
{
  struct timer *tk = (struct timer *) arg;

  /* Call the user-provided function.  */
  tk->thrfunc (tk->sival);

  return NULL;
}


/* Helper function to support starting threads for SIGEV_THREAD.  */
void *
attribute_hidden
__timer_helper_thread (void *arg)
{
  /* Block all signals.  */
  sigset_t ss;

  sigfillset (&ss);
  (void) pthread_sigmask (SIG_BLOCK, &ss, NULL);

  struct timer *tk = (struct timer *) arg;

  /* Synchronize with the parent.  */
  (void) pthread_barrier_wait (&tk->bar);

  /* We will only wait for the signal the kernel will send.  */
  sigemptyset (&ss);
  sigaddset (&ss, TIMER_SIG);

  /* Endless loop of waiting for signals.  The loop is only ended when
     the thread is canceled.  */
  while (1)
    {
      siginfo_t si;

      if (sigwaitinfo (&ss, &si) > 0 && si.si_timerid == tk->ktimerid)
	{
	  /* That the signal we are waiting for.  */
	  pthread_t th;
	  (void) pthread_create (&th, &tk->attr, timer_sigev_thread, arg);
	}
    }
}

#endif

#ifndef __ASSUME_POSIX_TIMERS
# include <nptl/sysdeps/pthread/timer_routines.c>
#endif
