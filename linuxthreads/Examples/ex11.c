/* Test program for timedout read/write lock functions.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2000.

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
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>


#define NWRITERS 15
#define WRITETRIES 10
#define NREADERS 15
#define READTRIES 15

#define TIMEOUT 1000000
#define DELAY   1000000

static pthread_rwlock_t lock = PTHREAD_RWLOCK_WRITER_NONRECURSIVE_INITIALIZER_NP;


static void *
writer_thread (void *nr)
{
  struct timespec ts;
  struct timespec delay;
  int n;

  ts.tv_sec = 0;
  ts.tv_nsec = TIMEOUT;

  delay.tv_sec = 0;
  delay.tv_nsec = DELAY;

  for (n = 0; n < WRITETRIES; ++n)
    {
      do
	{
	  clock_gettime (CLOCK_REALTIME, &ts);

	  ts.tv_nsec += 2 * TIMEOUT;

	  printf ("writer thread %ld tries again\n", (long int) nr);
	}
      //while (pthread_rwlock_wrlock (&lock), 0);
      while (pthread_rwlock_timedwrlock (&lock, &ts) == ETIMEDOUT);

      printf ("writer thread %ld succeeded\n", (long int) nr);

      nanosleep (&delay, NULL);

      pthread_rwlock_unlock (&lock);

      printf ("writer thread %ld released\n", (long int) nr);
    }

  return NULL;
}


static void *
reader_thread (void *nr)
{
  struct timespec ts;
  struct timespec delay;
  int n;

  delay.tv_sec = 0;
  delay.tv_nsec = DELAY;

  for (n = 0; n < READTRIES; ++n)
    {
      do
	{
	  clock_gettime (CLOCK_REALTIME, &ts);

	  ts.tv_nsec += TIMEOUT;

	  printf ("reader thread %ld tries again\n", (long int) nr);
	}
      //while (pthread_rwlock_rdlock (&lock), 0);
      while (pthread_rwlock_timedrdlock (&lock, &ts) == ETIMEDOUT);

      printf ("reader thread %ld succeeded\n", (long int) nr);

      nanosleep (&delay, NULL);

      pthread_rwlock_unlock (&lock);

      printf ("reader thread %ld released\n", (long int) nr);
    }

  return NULL;
}


int
main (void)
{
  pthread_t thwr[NWRITERS];
  pthread_t thrd[NREADERS];
  int n;
  void *res;

  /* Make standard error the same as standard output.  */
  dup2 (1, 2);

  /* Make sure we see all message, even those on stdout.  */
  setvbuf (stdout, NULL, _IONBF, 0);

  for (n = 0; n < NWRITERS; ++n)
    {
      int err = pthread_create (&thwr[n], NULL, writer_thread,
				(void *) (long int) n);

      if (err != 0)
	error (EXIT_FAILURE, err, "cannot create writer thread");
    }

  for (n = 0; n < NREADERS; ++n)
    {
      int err = pthread_create (&thrd[n], NULL, reader_thread,
				(void *) (long int) n);

      if (err != 0)
	error (EXIT_FAILURE, err, "cannot create reader thread");
    }

  /* Wait for all the threads.  */
  for (n = 0; n < NWRITERS; ++n)
    pthread_join (thwr[n], &res);
  for (n = 0; n < NREADERS; ++n)
    pthread_join (thrd[n], &res);

  return 0;
}
