/* Tests for waitid.
   Copyright (C) 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define TIMEOUT 15

static void
test_child (void)
{
  /* First thing, we stop ourselves.  */
  raise (SIGSTOP);

  /* Hey, we got continued!  */
  while (1)
    pause ();
}

#ifndef WEXITED
# define WEXITED	0
# define WCONTINUED	0
# define WSTOPPED	WUNTRACED
#endif

static int
do_test (int argc, char *argv[])
{
  pid_t pid = fork ();
  if (pid < 0)
    {
      error (0, errno, "fork");
      return EXIT_FAILURE;
    }
  else if (pid == 0)
    {
      test_child ();
      _exit (127);
    }

  /* Give the child a chance to stop.  */
  sleep (2);

  /* Now try a wait that should not succeed.  */
  siginfo_t info;
  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  int fail = waitid (P_PID, pid, &info, WEXITED|WCONTINUED|WNOHANG);
  switch (fail)
    {
    default:
      error (0, 0, "waitid returned bogus value %d\n", fail);
      return EXIT_FAILURE;
    case -1:
      error (0, errno, "waitid WNOHANG on stopped");
      return errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE;
    case 0:
      if (info.si_signo == 0)
	break;
      if (info.si_signo == SIGCHLD)
	error (0, 0, "waitid WNOHANG on stopped status %d\n", info.si_status);
      else
	error (0, 0, "waitid WNOHANG on stopped signal %d\n", info.si_signo);
      return EXIT_FAILURE;
    }

  /* Next the wait that should succeed right away.  */
  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  info.si_pid = -1;
  info.si_status = -1;
  fail = waitid (P_PID, pid, &info, WSTOPPED|WNOHANG);
  switch (fail)
    {
    default:
      error (0, 0, "waitid WSTOPPED|WNOHANG returned bogus value %d\n", fail);
      return EXIT_FAILURE;
    case -1:
      error (0, errno, "waitid WSTOPPED|WNOHANG on stopped");
      return errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE;
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped signal %d\n",
		 info.si_signo);
	  return EXIT_FAILURE;
	}
      if (info.si_code != CLD_STOPPED)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped code %d\n",
		 info.si_code);
	  return EXIT_FAILURE;
	}
      if (info.si_status != SIGSTOP)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped status %d\n",
		 info.si_status);
	  return EXIT_FAILURE;
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped pid %d != %d\n",
		 info.si_pid, pid);
	  return EXIT_FAILURE;
	}
    }

  if (kill (pid, SIGCONT) != 0)
    {
      error (0, errno, "kill (%d, SIGCONT)", pid);
      return EXIT_FAILURE;
    }

  /* Wait for the child to have continued.  */
  sleep (2);

#if 0
  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  info.si_pid = -1;
  info.si_status = -1;
  fail = waitid (P_PID, pid, &info, WCONTINUED);
  switch (fail)
    {
    default:
      error (0, 0, "waitid WCONTINUED returned bogus value %d\n", fail);
      return EXIT_FAILURE;
    case -1:
      error (0, errno, "waitid WCONTINUED on continued");
      return errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE;
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WCONTINUED on continued signal %d\n",
		 info.si_signo);
	  return EXIT_FAILURE;
	}
      if (info.si_code != CLD_CONTINUED)
	{
	  error (0, 0, "waitid WCONTINUED on continued code %d\n",
		 info.si_code);
	  return EXIT_FAILURE;
	}
      if (info.si_status != SIGCONT)
	{
	  error (0, 0, "waitid WCONTINUED on continued status %d\n",
		 info.si_status);
	  return EXIT_FAILURE;
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WCONTINUED on continued pid %d != %d\n",
		 info.si_pid, pid);
	  return EXIT_FAILURE;
	}
    }
#endif

  /* Die, child, die!  */
  if (kill (pid, SIGKILL) != 0)
    {
      error (0, errno, "kill (%d, SIGKILL)", pid);
      return EXIT_FAILURE;
    }

#ifdef WNOWAIT
  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  info.si_pid = -1;
  info.si_status = -1;
  fail = waitid (P_PID, pid, &info, WEXITED|WNOWAIT);
  switch (fail)
    {
    default:
      error (0, 0, "waitid WNOWAIT returned bogus value %d\n", fail);
      return EXIT_FAILURE;
    case -1:
      error (0, errno, "waitid WNOWAIT on killed");
      return errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE;
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WNOWAIT on killed signal %d\n",
		 info.si_signo);
	  return EXIT_FAILURE;
	}
      if (info.si_code != CLD_KILLED)
	{
	  error (0, 0, "waitid WNOWAIT on killed code %d\n",
		 info.si_code);
	  return EXIT_FAILURE;
	}
      if (info.si_status != SIGKILL)
	{
	  error (0, 0, "waitid WNOWAIT on killed status %d\n",
		 info.si_status);
	  return EXIT_FAILURE;
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WNOWAIT on killed pid %d != %d\n",
		 info.si_pid, pid);
	  return EXIT_FAILURE;
	}
    }
#else
  /* Allow enough time to be sure the child died; we didn't synchronize.  */
  sleep (2);
#endif

  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  info.si_pid = -1;
  info.si_status = -1;
  fail = waitid (P_PID, pid, &info, WEXITED|WNOHANG);
  switch (fail)
    {
    default:
      error (0, 0, "waitid WNOHANG returned bogus value %d\n", fail);
      return EXIT_FAILURE;
    case -1:
      error (0, errno, "waitid WNOHANG on killed");
      return EXIT_FAILURE;
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WNOHANG on killed signal %d\n",
		 info.si_signo);
	  return EXIT_FAILURE;
	}
      if (info.si_code != CLD_KILLED)
	{
	  error (0, 0, "waitid WNOHANG on killed code %d\n",
		 info.si_code);
	  return EXIT_FAILURE;
	}
      if (info.si_status != SIGKILL)
	{
	  error (0, 0, "waitid WNOHANG on killed status %d\n",
		 info.si_status);
	  return EXIT_FAILURE;
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WNOHANG on killed pid %d != %d\n",
		 info.si_pid, pid);
	  return EXIT_FAILURE;
	}
    }

  fail = waitid (P_PID, pid, &info, WEXITED);
  if (fail == -1)
    {
      if (errno != ECHILD)
	{
	  error (0, errno, "waitid WEXITED on killed");
	  return EXIT_FAILURE;
	}
    }
  else
    {
      error (0, 0, "waitid WEXITED returned bogus value %d\n", fail);
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

#include "../test-skeleton.c"
