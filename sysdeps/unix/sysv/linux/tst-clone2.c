/* Test if CLONE_VM does not change pthread pid/tid field (BZ #19957)
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <sched.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <tls.h> /* for THREAD_* macros.  */

static int sig;
static int pipefd[2];

static int
f (void *a)
{
  close (pipefd[0]);

  pid_t pid = THREAD_GETMEM (THREAD_SELF, pid);
  pid_t tid = THREAD_GETMEM (THREAD_SELF, tid);

  while (write (pipefd[1], &pid, sizeof pid) < 0)
    continue;
  while (write (pipefd[1], &tid, sizeof tid) < 0)
    continue;

  return 0;
}


static int
clone_test (int clone_flags)
{
  sig = SIGRTMIN;
  sigset_t ss;
  sigemptyset (&ss);
  sigaddset (&ss, sig);
  if (sigprocmask (SIG_BLOCK, &ss, NULL) != 0)
    {
      printf ("sigprocmask failed: %m\n");
      return 1;
    }

  if (pipe2 (pipefd, O_CLOEXEC))
    {
      printf ("sigprocmask failed: %m\n");
      return 1;
    }

  pid_t ppid = getpid ();

#ifdef __ia64__
  extern int __clone2 (int (*__fn) (void *__arg), void *__child_stack_base,
		       size_t __child_stack_size, int __flags,
		       void *__arg, ...);
  char st[256 * 1024] __attribute__ ((aligned));
  pid_t p = __clone2 (f, st, sizeof (st), clone_flags, 0);
#else
  char st[128 * 1024] __attribute__ ((aligned));
#if _STACK_GROWS_DOWN
  pid_t p = clone (f, st + sizeof (st), clone_flags, 0);
#elif _STACK_GROWS_UP
  pid_t p = clone (f, st, clone_flags, 0);
#else
#error "Define either _STACK_GROWS_DOWN or _STACK_GROWS_UP"
#endif
#endif
  close (pipefd[1]);

  if (p == -1)
    {
      printf ("clone failed: %m\n");
      return 1;
    }

  pid_t pid, tid;
  if (read (pipefd[0], &pid, sizeof pid) != sizeof pid)
    {
      printf ("read pid failed: %m\n");
      kill (p, SIGKILL);
      return 1;
    }
  if (read (pipefd[0], &tid, sizeof tid) != sizeof tid)
    {
      printf ("read pid failed: %m\n");
      kill (p, SIGKILL);
      return 1;
    }

  close (pipefd[0]);

  int ret = 0;

  /* For CLONE_VM glibc clone implementation does not change the pthread
     pid/tid field.  */
  if ((clone_flags & CLONE_VM) == CLONE_VM)
    {
      if ((ppid != pid) || (ppid != tid))
	{
	  printf ("parent pid (%i) != received pid/tid (%i/%i)\n",
		  (int)ppid, (int)pid, (int)tid);
	  ret = 1;
	}
    }
  /* For any other flag clone updates the new pthread pid and tid with
     the clone return value.  */
  else
    {
      if ((p != pid) || (p != tid))
	{
	  printf ("child pid (%i) != received pid/tid (%i/%i)\n",
		  (int)p, (int)pid, (int)tid);
	  ret = 1;
	}
    }

  int e;
  if (waitpid (p, &e, __WCLONE) != p)
    {
      puts ("waitpid failed");
      kill (p, SIGKILL);
      return 1;
    }
  if (!WIFEXITED (e))
    {
      if (WIFSIGNALED (e))
	printf ("died from signal %s\n", strsignal (WTERMSIG (e)));
      else
	puts ("did not terminate correctly");
      return 1;
    }
  if (WEXITSTATUS (e) != 0)
    {
      printf ("exit code %d\n", WEXITSTATUS (e));
      return 1;
    }

  return ret;
}

int
do_test (void)
{
  /* First, check that the clone implementation, without any flag, updates
     the struct pthread to contain the new PID and TID.  */
  int ret = clone_test (0);
  /* Second, check that with CLONE_VM the struct pthread PID and TID fields
     remain unmodified after the clone.  Any modifications would cause problem
     for the parent as described in bug 19957.  */
  ret += clone_test (CLONE_VM);
  return ret;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
