/* Tests for pthread_mutex_timedlock function.
   Copyright (C) 2000, 2001, 2002 Free Software Foundation, Inc.
   Contributed by Kaz Kylheku <kaz@ashi.footprints.net>, 2000.

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
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 10
#define NUM_ITERS   50
#define TIMEOUT_NS  100000000L

static void *thread (void *)  __attribute__ ((__noreturn__));
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int
main (void)
{
  pthread_t th;
  int i;

  for (i = 0; i < NUM_THREADS; i++)
    {
      if (pthread_create (&th, NULL, thread, NULL) != 0)
	error (EXIT_FAILURE, 0, "cannot create thread");
    }

  (void) thread (NULL);
  /* notreached */
  return 0;
}


static void *
thread (void *arg)
{
  int i;
  pthread_t self = pthread_self ();
  static int linecount; /* protected by flockfile(stdout) */

  for (i = 0; i < NUM_ITERS; i++)
    {
      struct timespec ts;

      for (;;)
	{
	  int err;

	  clock_gettime (CLOCK_REALTIME, &ts);

	  ts.tv_nsec += TIMEOUT_NS;

	  if (ts.tv_nsec >= 1000000000L) {
	     ts.tv_sec++;
	     ts.tv_nsec -= 1000000000L;
	  }

	  switch ((err = pthread_mutex_timedlock (&mutex, &ts)))
	    {
	    case 0:
	      flockfile (stdout);
	      printf ("%04d: thread %lu got mutex\n", ++linecount,
		      (unsigned long) self);
	      funlockfile (stdout);
	      break;
	    case ETIMEDOUT:
	      flockfile (stdout);
	      printf ("%04d: thread %lu timed out on mutex\n", ++linecount,
		      (unsigned long) self);
	      funlockfile (stdout);
	      continue;
	    default:
	      error (EXIT_FAILURE, err, "pthread_mutex_timedlock failure");
	    }
	  break;
	}

      ts.tv_sec = 0;
      ts.tv_nsec = TIMEOUT_NS;
      nanosleep (&ts, NULL);

      flockfile (stdout);
      printf ("%04d: thread %lu releasing mutex\n", ++linecount,
	      (unsigned long) self);
      funlockfile (stdout);
      pthread_mutex_unlock (&mutex);
    }

  pthread_exit (NULL);
}
