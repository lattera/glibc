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
    /* Don't allow PTHREAD_SIG_RESTART to be unmasked.
       Don't allow PTHREAD_SIG_CANCEL to be masked. */
    switch(how) {
    case SIG_SETMASK:
      sigaddset(&mask, PTHREAD_SIG_RESTART);
      sigdelset(&mask, PTHREAD_SIG_CANCEL);
      break;
    case SIG_BLOCK:
      sigdelset(&mask, PTHREAD_SIG_CANCEL);
      break;
    case SIG_UNBLOCK:
      sigdelset(&mask, PTHREAD_SIG_RESTART);
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

  acquire(&handle->h_spinlock);
  if (invalid_handle(handle, thread)) {
    release(&handle->h_spinlock);
    return ESRCH;
  }
  pid = handle->h_descr->p_pid;
  release(&handle->h_spinlock);
  if (kill(pid, signo) == -1)
    return errno;
  else
    return 0;
}

/* The set of signals on which some thread is doing a sigwait */
static sigset_t sigwaited;
static pthread_mutex_t sigwaited_mut = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t sigwaited_changed = PTHREAD_COND_INITIALIZER;

int sigwait(const sigset_t * set, int * sig)
{
  volatile pthread_descr self = thread_self();
  sigset_t mask;
  int s;
  struct sigaction action, saved_signals[NSIG];
  sigjmp_buf jmpbuf;

  pthread_mutex_lock(&sigwaited_mut);
  /* Make sure no other thread is waiting on our signals */
test_again:
  for (s = 1; s < NSIG; s++) {
    if (sigismember(set, s) && sigismember(&sigwaited, s)) {
      pthread_cond_wait(&sigwaited_changed, &sigwaited_mut);
      goto test_again;
    }
  }
  /* Get ready to block all signals except those in set
     and the cancellation signal */
  sigfillset(&mask);
  sigdelset(&mask, PTHREAD_SIG_CANCEL);
  /* Signals in set are assumed blocked on entrance */
  /* Install our signal handler on all signals in set,
     and unblock them in mask.
     Also mark those signals as being sigwaited on */
  for (s = 1; s < NSIG; s++) {
    if (sigismember(set, s) && s != PTHREAD_SIG_CANCEL) {
      sigdelset(&mask, s);
      action.sa_handler = __pthread_sighandler;
      sigemptyset(&action.sa_mask);
      action.sa_flags = 0;
      sigaction(s, &action, &(saved_signals[s]));
      sigaddset(&sigwaited, s);
    }
  }
  pthread_mutex_unlock(&sigwaited_mut);

  /* Test for cancellation */
  if (sigsetjmp(jmpbuf, 1) == 0) {
    self->p_cancel_jmp = &jmpbuf;
    if (! (self->p_canceled && self->p_cancelstate == PTHREAD_CANCEL_ENABLE)) {
      /* Reset the signal count */
      self->p_signal = 0;
      /* Unblock the signals and wait for them */
      sigsuspend(&mask);
    }
  }
  self->p_cancel_jmp = NULL;
  /* The signals are now reblocked. Restore the sighandlers. */
  pthread_mutex_lock(&sigwaited_mut);
  for (s = 1; s < NSIG; s++) {
    if (sigismember(set, s) && s != PTHREAD_SIG_CANCEL) {
      sigaction(s, &(saved_signals[s]), NULL);
      sigdelset(&sigwaited, s);
    }
  }
  pthread_cond_broadcast(&sigwaited_changed);
  pthread_mutex_unlock(&sigwaited_mut);
  /* Check for cancellation */
  pthread_testcancel();
  /* We should have self->p_signal != 0 and equal to the signal received */
  *sig = self->p_signal;
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
