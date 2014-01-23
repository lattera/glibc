/* Copyright (C) 2013 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */


/* This test checks that TLS in a dlopened object works when first accessed
   from a signal handler.  */

#include <assert.h>
#include <atomic.h>
#include <dlfcn.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void *
spin (void *ignored)
{
  while (1)
    {
      /* busywork */
      free (malloc (128));
    }

  /* never reached */
  return NULL;
}

static void (*tls7mod_action) (int, siginfo_t *, void *);

static void
action (int signo, siginfo_t *info, void *ignored)
{
  sem_t *sem = info->si_value.sival_ptr;

  atomic_read_barrier ();
  assert (tls7mod_action != NULL);
  (*tls7mod_action) (signo, info, ignored);

  /* This sem_post may trigger dlclose, which will invalidate tls7mod_action.
     It is important to do that only after tls7mod_action is no longer
     active.  */
  sem_post (sem);
}

int
do_test (void)
{
  pthread_t th[10];

  for (int i = 0; i < 10; ++i)
    {
      if (pthread_create (&th[i], NULL, spin, NULL))
        {
          puts ("pthread_create failed");
          exit (1);
        }
    }
#define NITERS 75

  for (int i = 0; i < NITERS; ++i)
    {
      void *h = dlopen ("tst-tls7mod.so", RTLD_LAZY);
      if (h == NULL)
        {
          puts ("dlopen failed");
          exit (1);
        }

      tls7mod_action = dlsym (h, "action");
      if (tls7mod_action == NULL)
        {
          puts ("dlsym for action failed");
          exit (1);
        }
      atomic_write_barrier ();

      struct sigaction sa;
      sa.sa_sigaction = action;
      sigemptyset (&sa.sa_mask);
      sa.sa_flags = SA_SIGINFO;
      if (sigaction (SIGUSR1, &sa, NULL))
        {
          puts ("sigaction failed");
          exit (1);
        }

      sem_t sem;
      if (sem_init (&sem, 0, 0))
        {
          puts ("sem_init failed");
        }

      sigval_t val;
      val.sival_ptr = &sem;
      for (int i = 0; i < 10; ++i)
        {
          if (pthread_sigqueue (th[i], SIGUSR1, val))
            {
              puts ("pthread_sigqueue failed");
            }
        }


      for (int i = 0; i < 10; ++i)
        {
          if (sem_wait (&sem))
          {
            puts ("sem_wait failed");
          }
        }

      /* Paranoia.  */
      tls7mod_action = NULL;

      if (dlclose (h))
        {
          puts ("dlclose failed");
          exit (1);
        }
    }
  return 0;
}

#define TIMEOUT 8

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
