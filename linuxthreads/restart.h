/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

#include <signal.h>

/* Primitives for controlling thread execution */

static inline void restart(pthread_descr th)
{
  kill(th->p_pid, __pthread_sig_restart);
}

static inline void suspend(pthread_descr self)
{
  sigset_t mask;

  sigprocmask(SIG_SETMASK, NULL, &mask); /* Get current signal mask */
  sigdelset(&mask, __pthread_sig_restart); /* Unblock the restart signal */
  do {
    self->p_signal = 0;
    sigsuspend(&mask);                   /* Wait for signal */
  } while (self->p_signal !=__pthread_sig_restart );
}

static inline void suspend_with_cancellation(pthread_descr self)
{
  sigset_t mask;
  sigjmp_buf jmpbuf;

  sigprocmask(SIG_SETMASK, NULL, &mask); /* Get current signal mask */
  sigdelset(&mask, __pthread_sig_restart); /* Unblock the restart signal */
  /* No need to save the signal mask, we'll restore it ourselves */
  if (sigsetjmp(jmpbuf, 0) == 0) {
    self->p_cancel_jmp = &jmpbuf;
    if (! (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE)) {
      do {
	self->p_signal = 0;
        sigsuspend(&mask);               /* Wait for a signal */
      } while (self->p_signal != __pthread_sig_restart);
    }
    self->p_cancel_jmp = NULL;
  } else {
    sigaddset(&mask, __pthread_sig_restart); /* Reblock the restart signal */
    sigprocmask(SIG_SETMASK, &mask, NULL);
  }
}
