/* Tests for fork in multi-threaded environment.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2000.

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
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/wait.h>

enum
{
  PREPARE_BIT = 1,
  PARENT_BIT = 2,
  CHILD_BIT = 4
};

static int var;

static void
prepare (void)
{
  var |= PREPARE_BIT;
}

static void
parent (void)
{
  var |= PARENT_BIT;
}

static void
child (void)
{
  var |= CHILD_BIT;
}


static void *thread (void *arg);


int
main (void)
{
  pthread_t th;
  void *res;

  pthread_atfork (prepare, parent, child);

  if (pthread_create (&th, NULL, thread, NULL) != 0)
    error (EXIT_FAILURE, 0, "cannot create thread");

  pthread_join (th, &res);

  return (int) (long int) res;
}


static void *
thread (void *arg)
{
  int status;
  pid_t pid;

  pid = fork ();
  if (pid == 0)
    {
      /* We check whether the `prepare' and `child' function ran.  */
      exit (var != (PREPARE_BIT | CHILD_BIT));
    }
  else if (pid == (pid_t) -1)
    error (EXIT_FAILURE, errno, "cannot fork");

  if (waitpid (pid, &status, 0) != pid)
    error (EXIT_FAILURE, errno, "wrong child");

  if (WTERMSIG (status) != 0)
    error (EXIT_FAILURE, 0, "Child terminated incorrectly");
  status = WEXITSTATUS (status);

  if (status == 0)
    status = var != (PREPARE_BIT | PARENT_BIT);

  return (void *) (long int) status;
}
