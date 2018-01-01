/* Copyright (C) 2003-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int do_test (void);

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"

#include "eintr.c"


static void *
tf (void *arg)
{
  pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock (&m);
  /* This call must not return.  */
  pthread_mutex_lock (&m);

  puts ("tf: mutex_lock returned");
  exit (1);
}


static int
do_test (void)
{
  pthread_t self = pthread_self ();

  setup_eintr (SIGUSR1, &self);

  pthread_t th;
  char buf[100];
  int e = pthread_create (&th, NULL, tf, NULL);
  if (e != 0)
    {
      printf ("main: pthread_create failed: %s\n",
	      strerror_r (e, buf, sizeof (buf)));
      exit (1);
    }

  delayed_exit (1);
  /* This call must never return.  */
  xpthread_join (th);
  puts ("error: pthread_join returned");
  return 1;
}
