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

/* Handling of signals */

#include <errno.h>
#include <signal.h>
#include "pthread.h"
#include "internals.h"
#include "spinlock.h"

int pthread_sigmask(int how, const sigset_t * newmask, sigset_t * oldmask)
{
  sigset_t mask;

  if (newmask != NULL) {
    mask = *newmask;
    /* Don't allow __pthread_sig_restart to be unmasked.
       Don't allow __pthread_sig_cancel to be masked. */
    switch(how) {
    case SIG_SETMASK:
      sigaddset(&mask, __pthread_sig_restart);
      sigdelset(&mask, __pthread_sig_cancel);
      break;
    case SIG_BLOCK:
      sigdelset(&mask, __pthread_sig_cancel);
      break;
    case SIG_UNBLOCK:
      sigdelset(&mask, __pthread_sig_restart);
      break;
    }
    newmask = &mask;
  }
  if (sigprocmask(how, newmask, oldmask) == -1)
    return errno;
  else
    return 0;
}

int pthread_kill(pthread_t thread, int signo)
{
  pthread_handle handle = thread_handle(thread);
  int pid;

  __pthread_lock(&handle->h_lock);
  if (invalid_handle(handle, thread)) {
    __pthread_unlock(&handle->h_lock);
    return ESRCH;
  }
  pid = handle->h_descr->p_pid;
  __pthread_unlock(&handle->h_lock);
  if (kill(pid, signo) == -1)
    return errno;
  else
    return 0;
}

/* User-provided signal handlers */
static __sighandler_t sighandler[NSIG];

/* The wrapper around user-provided signal handlers */
static void pthread_sighandler(int signo)
{
  pthread_descr self = thread_self();
  char * in_sighandler;
  /* If we're in a sigwait operation, just record the signal received
     and return without calling the user's handler */
  if (THREAD_GETMEM(self, p_sigwaiting)) {
    THREAD_SETMEM(self, p_sigwaiting, 0);
    THREAD_SETMEM(self, p_signal, signo);
    return;
  }
  /* Record that we're in a signal handler and call the user's
     handler function */
  in_sighandler = THREAD_GETMEM(self, p_in_sighandler);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, CURRENT_STACK_FRAME);
  sighandler[signo](signo);
  if (in_sighandler == NULL)
    THREAD_SETMEM(self, p_in_sighandler, NULL);
}

int sigaction(int sig, const struct sigaction * act,
              struct sigaction * oact)
{
  struct sigaction newact;
  struct sigaction *newactp;

  if (sig == __pthread_sig_restart || sig == __pthread_sig_cancel)
    return EINVAL;
  if (act)
    {
      newact = *act;
      if (act->sa_handler != SIG_IGN && act->sa_handler != SIG_DFL)
	newact.sa_handler = pthread_sighandler;
      newactp = &newact;
    }
  else
    newactp = NULL;
  if (__sigaction(sig, newactp, oact) == -1)
    return -1;
  if (oact != NULL) oact->sa_handler = sighandler[sig];
  if (act)
    sighandler[sig] = act->sa_handler;
  return 0;
}

int sigwait(const sigset_t * set, int * sig)
{
  volatile pthread_descr self = thread_self();
  sigset_t mask;
  int s;
  sigjmp_buf jmpbuf;

  /* Get ready to block all signals except those in set
     and the cancellation signal */
  sigfillset(&mask);
  sigdelset(&mask, __pthread_sig_cancel);
  for (s = 1; s <= NSIG; s++) {
    if (sigismember(set, s) && s != __pthread_sig_cancel)
      sigdelset(&mask, s);
  }
  /* Test for cancellation */
  if (sigsetjmp(jmpbuf, 1) == 0) {
    THREAD_SETMEM(self, p_cancel_jmp, &jmpbuf);
    if (! (THREAD_GETMEM(self, p_canceled)
	   && THREAD_GETMEM(self, p_cancelstate) == PTHREAD_CANCEL_ENABLE)) {
      /* Reset the signal count */
      THREAD_SETMEM(self, p_signal, 0);
      /* Say we're in sigwait */
      THREAD_SETMEM(self, p_sigwaiting, 1);
      /* Unblock the signals and wait for them */
      sigsuspend(&mask);
    }
  }
  THREAD_SETMEM(self, p_cancel_jmp, NULL);
  /* The signals are now reblocked.  Check for cancellation */
  pthread_testcancel();
  /* We should have self->p_signal != 0 and equal to the signal received */
  *sig = THREAD_GETMEM(self, p_signal);
  return 0;
}

int raise (int sig)
{
  int retcode = pthread_kill(pthread_self(), sig);
  if (retcode == 0)
    return 0;
  else {
    errno = retcode;
    return -1;
  }
}
