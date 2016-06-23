/* Test case for async-signal-safe fork (with respect to malloc).
   Copyright (C) 2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

/* This test will fail if the process is multi-threaded because we
   only have an async-signal-safe fork in the single-threaded case
   (where we skip acquiring the malloc heap locks).

   This test only checks async-signal-safety with regards to malloc;
   other, more rarely-used glibc subsystems could have locks which
   still make fork unsafe, even in single-threaded processes.  */

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/* How many malloc objects to keep arond.  */
enum { malloc_objects = 1009 };

/* The maximum size of an object.  */
enum { malloc_maximum_size = 70000 };

/* How many signals need to be delivered before the test exits.  */
enum { signal_count = 1000 };

static int do_test (void);
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"

/* Process ID of the subprocess which sends SIGUSR1 signals.  */
static pid_t sigusr1_sender_pid;

/* Set to 1 if SIGUSR1 is received.  Used to detect a signal during
   malloc/free.  */
static volatile sig_atomic_t sigusr1_received;

/* Periodically set to 1, to indicate that the process is making
   progress.  Checked by liveness_signal_handler.  */
static volatile sig_atomic_t progress_indicator = 1;

static void
sigusr1_handler (int signo)
{
  /* Let the main program make progress, by temporarily suspending
     signals from the subprocess.  */
  if (sigusr1_received)
    return;
  /* sigusr1_sender_pid might not be initialized in the parent when
     the first SIGUSR1 signal arrives.  */
  if (sigusr1_sender_pid > 0 && kill (sigusr1_sender_pid, SIGSTOP) != 0)
    {
      write_message ("error: kill (SIGSTOP)\n");
      abort ();
    }
  sigusr1_received = 1;

  /* Perform a fork with a trivial subprocess.  */
  pid_t pid = fork ();
  if (pid == -1)
    {
      write_message ("error: fork\n");
      abort ();
    }
  if (pid == 0)
    _exit (0);
  int status;
  int ret = TEMP_FAILURE_RETRY (waitpid (pid, &status, 0));
  if (ret < 0)
    {
      write_message ("error: waitpid\n");
      abort ();
    }
  if (status != 0)
    {
      write_message ("error: unexpected exit status from subprocess\n");
      abort ();
    }
}

static void
liveness_signal_handler (int signo)
{
  if (progress_indicator)
    progress_indicator = 0;
  else
    write_message ("warning: process seems to be stuck\n");
}

static void
__attribute__ ((noreturn))
signal_sender (int signo, bool sleep)
{
  pid_t target = getppid ();
  while (true)
    {
      if (kill (target, signo) != 0)
        {
          dprintf (STDOUT_FILENO, "error: kill: %m\n");
          abort ();
        }
      if (sleep)
        usleep (1 * 1000 * 1000);
      else
        /* Reduce the rate at which we send signals.  */
        sched_yield ();
    }
}

static int
do_test (void)
{
  struct sigaction action =
    {
      .sa_handler = sigusr1_handler,
    };
  sigemptyset (&action.sa_mask);

  if (sigaction (SIGUSR1, &action, NULL) != 0)
    {
      printf ("error: sigaction: %m");
      return 1;
    }

  action.sa_handler = liveness_signal_handler;
  if (sigaction (SIGUSR2, &action, NULL) != 0)
    {
      printf ("error: sigaction: %m");
      return 1;
    }

  pid_t sigusr2_sender_pid = fork ();
  if (sigusr2_sender_pid == 0)
    signal_sender (SIGUSR2, true);
  sigusr1_sender_pid = fork ();
  if (sigusr1_sender_pid == 0)
    signal_sender (SIGUSR1, false);

  void *objects[malloc_objects] = {};
  unsigned signals = 0;
  unsigned seed = 1;
  time_t last_report = 0;
  while (signals < signal_count)
    {
      progress_indicator = 1;
      int slot = rand_r (&seed) % malloc_objects;
      size_t size = rand_r (&seed) % malloc_maximum_size;
      if (kill (sigusr1_sender_pid, SIGCONT) != 0)
        {
          printf ("error: kill (SIGCONT): %m\n");
          signal (SIGUSR1, SIG_IGN);
          kill (sigusr1_sender_pid, SIGKILL);
          kill (sigusr2_sender_pid, SIGKILL);
          return 1;
        }
      sigusr1_received = false;
      free (objects[slot]);
      objects[slot] = malloc (size);
      if (sigusr1_received)
        {
          ++signals;
          time_t current = time (0);
          if (current != last_report)
            {
              printf ("info: SIGUSR1 signal count: %u\n", signals);
              last_report = current;
            }
        }
      if (objects[slot] == NULL)
        {
          printf ("error: malloc: %m\n");
          signal (SIGUSR1, SIG_IGN);
          kill (sigusr1_sender_pid, SIGKILL);
          kill (sigusr2_sender_pid, SIGKILL);
          return 1;
        }
    }

  /* Clean up allocations.  */
  for (int slot = 0; slot < malloc_objects; ++slot)
    free (objects[slot]);

  /* Terminate the signal-sending subprocess.  The SIGUSR1 handler
     should no longer run because it uses sigusr1_sender_pid.  */
  signal (SIGUSR1, SIG_IGN);
  kill (sigusr1_sender_pid, SIGKILL);
  kill (sigusr2_sender_pid, SIGKILL);

  return 0;
}
