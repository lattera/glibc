/* Test for Pthreads/mutexes.
   Copyright (C) 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kurt Garloff <garloff@suse.de>, 2000.

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

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void *thread_start (void *ptr) __attribute__ ((__noreturn__));


struct thr_ctrl
{
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  int retval;
};

static void
dump_mut (pthread_mutex_t * mut)
{
  size_t i;
  for (i = 0; i < sizeof (*mut); i++)
    printf (" %02x", *((unsigned char *) mut + i));
  printf ("\n");
};

/* Helper, the opposite of pthread_cond_wait (cond, mut).  */
static void
pthr_cond_signal_mutex (pthread_cond_t * cond, pthread_mutex_t * mut)
{
  int err;
  err = pthread_mutex_lock (mut);
  if (err)
    printf ("mutex_lock  : %s\n", strerror (err));
  err = pthread_cond_signal (cond);
  if (err)
    printf ("cond_signal : %s\n", strerror (err));
  err = pthread_mutex_unlock (mut);
  if (err)
    printf ("mutex_unlock: %s\n", strerror (err));
}

static void *
thread_start (void *ptr)
{
  struct thr_ctrl *tc = ptr;
  /* Do initialization.  */
  /* ... */
  /* Signal that we are ready.  */
  pthr_cond_signal_mutex (&tc->cond, &tc->mutex);
  sleep (2);
  pthr_cond_signal_mutex (&tc->cond, &tc->mutex);
  tc->retval = 0;
  pthread_exit (&tc->retval);
}

int
main (void)
{
  struct thr_ctrl threadctrl;
  pthread_t thread;
  int err;
  void *res = &threadctrl.retval;
  pthread_mutexattr_t mutattr;
  pthread_mutexattr_init (&mutattr);
  pthread_mutex_init (&threadctrl.mutex, &mutattr);
  pthread_cond_init (&threadctrl.cond, NULL);
  err = pthread_mutex_lock (&threadctrl.mutex);
  if (err)
    printf ("mutex_lock : %s\n", strerror (err));
  dump_mut (&threadctrl.mutex);
  pthread_create (&thread, NULL, thread_start, &threadctrl);
  /* Wait until it's ready.  */
  err = pthread_cond_wait (&threadctrl.cond, &threadctrl.mutex);
  if (err)
    printf ("cond_wait  : %s\n", strerror (err));
  /* Now, we should have acquired the mutex again!  */
  dump_mut (&threadctrl.mutex);
  sleep (1);
  dump_mut (&threadctrl.mutex);
  err = pthread_cond_wait (&threadctrl.cond, &threadctrl.mutex);
  if (err)
    {
      printf ("cond_wait  : %s\n", strerror (err));
      printf ("ERROR\n");
      abort ();
    };
  dump_mut (&threadctrl.mutex);
  pthread_join (thread, &res);
  printf ("OK\n");
  return 0;
}
