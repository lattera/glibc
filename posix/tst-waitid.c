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
  /* Wait a second to be sure the parent set his variables before we
     produce a SIGCHLD.  */
  sleep (1);

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

static sig_atomic_t expecting_sigchld, spurious_sigchld;
#ifdef SA_SIGINFO
static siginfo_t sigchld_info;

static void
sigchld (int signo, siginfo_t *info, void *ctx)
{
  if (signo != SIGCHLD)
    {
      error (0, 0, "SIGCHLD handler got signal %d instead!", signo);
      _exit (EXIT_FAILURE);
    }

  if (! expecting_sigchld)
    {
      spurious_sigchld = 1;
      error (0, 0,
	     "spurious SIGCHLD: signo %d code %d status %d pid %d\n",
	     info->si_signo, info->si_code, info->si_status, info->si_pid);
    }
  else
    {
      sigchld_info = *info;
      expecting_sigchld = 0;
    }
}

static void
check_sigchld (const char *phase, int *ok, int code, int status, pid_t pid)
{
  if (expecting_sigchld)
    {
      error (0, 0, "missing SIGCHLD on %s", phase);
      *ok = EXIT_FAILURE;
      expecting_sigchld = 0;
      return;
    }

  if (sigchld_info.si_signo != SIGCHLD)
    {
      error (0, 0, "SIGCHLD for %s signal %d", phase, sigchld_info.si_signo);
      *ok = EXIT_FAILURE;
    }
  if (sigchld_info.si_code != code)
    {
      error (0, 0, "SIGCHLD for %s code %d", phase, sigchld_info.si_code);
      *ok = EXIT_FAILURE;
    }
  if (sigchld_info.si_status != status)
    {
      error (0, 0, "SIGCHLD for %s status %d", phase, sigchld_info.si_status);
      *ok = EXIT_FAILURE;
    }
  if (sigchld_info.si_pid != pid)
    {
      error (0, 0, "SIGCHLD for %s pid %d", phase, sigchld_info.si_pid);
      *ok = EXIT_FAILURE;
    }
}
# define CHECK_SIGCHLD(phase, code_check, status_check) \
  check_sigchld ((phase), &status, (code_check), (status_check), pid)
#else
# define CHECK_SIGCHLD(phase, code, status) ((void) 0)
#endif

static int
do_test (int argc, char *argv[])
{
#ifdef SA_SIGINFO
  struct sigaction sa;
  sa.sa_flags = SA_SIGINFO|SA_RESTART;
  sa.sa_sigaction = &sigchld;
  if (sigemptyset (&sa.sa_mask) < 0 || sigaction (SIGCHLD, &sa, NULL) < 0)
    {
      error (0, errno, "setting SIGCHLD handler");
      return EXIT_FAILURE;
    }
#endif

  expecting_sigchld = 1;

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

  int status = EXIT_SUCCESS;
#define RETURN(ok) \
    do { if (status == EXIT_SUCCESS) status = (ok); goto out; } while (0)

  /* Give the child a chance to stop.  */
  sleep (3);

  CHECK_SIGCHLD ("stopped", CLD_STOPPED, SIGSTOP);

  /* Now try a wait that should not succeed.  */
  siginfo_t info;
  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  int fail = waitid (P_PID, pid, &info, WEXITED|WCONTINUED|WNOHANG);
  switch (fail)
    {
    default:
      error (0, 0, "waitid returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      error (0, errno, "waitid WNOHANG on stopped");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo == 0)
	break;
      if (info.si_signo == SIGCHLD)
	error (0, 0, "waitid WNOHANG on stopped status %d\n", info.si_status);
      else
	error (0, 0, "waitid WNOHANG on stopped signal %d\n", info.si_signo);
      RETURN (EXIT_FAILURE);
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
      RETURN (EXIT_FAILURE);
    case -1:
      error (0, errno, "waitid WSTOPPED|WNOHANG on stopped");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped signal %d\n",
		 info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_STOPPED)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped code %d\n",
		 info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGSTOP)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped status %d\n",
		 info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WSTOPPED|WNOHANG on stopped pid %d != %d\n",
		 info.si_pid, pid);
	  RETURN (EXIT_FAILURE);
	}
    }

  expecting_sigchld = WCONTINUED != 0;

  if (kill (pid, SIGCONT) != 0)
    {
      error (0, errno, "kill (%d, SIGCONT)", pid);
      RETURN (EXIT_FAILURE);
    }

  /* Wait for the child to have continued.  */
  sleep (2);

#if WCONTINUED != 0
  if (expecting_sigchld)
    {
      error (0, 0, "no SIGCHLD seen for SIGCONT (optional)");
      expecting_sigchld = 0;
    }
  else
    CHECK_SIGCHLD ("continued", CLD_CONTINUED, SIGCONT);

  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  info.si_pid = -1;
  info.si_status = -1;
  fail = waitid (P_PID, pid, &info, WCONTINUED|WNOWAIT);
  switch (fail)
    {
    default:
      error (0, 0,
	     "waitid WCONTINUED|WNOWAIT returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      error (0, errno, "waitid WCONTINUED|WNOWAIT on continued");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WCONTINUED|WNOWAIT on continued signal %d\n",
		 info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_CONTINUED)
	{
	  error (0, 0, "waitid WCONTINUED|WNOWAIT on continued code %d\n",
		 info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGCONT)
	{
	  error (0, 0, "waitid WCONTINUED|WNOWAIT on continued status %d\n",
		 info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WCONTINUED|WNOWAIT on continued pid %d != %d\n",
		 info.si_pid, pid);
	  RETURN (EXIT_FAILURE);
	}
    }

  /* That should leave the CLD_CONTINUED state waiting to be seen again.  */
  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  info.si_pid = -1;
  info.si_status = -1;
  fail = waitid (P_PID, pid, &info, WCONTINUED);
  switch (fail)
    {
    default:
      error (0, 0, "waitid WCONTINUED returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      error (0, errno, "waitid WCONTINUED on continued");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WCONTINUED on continued signal %d\n",
		 info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_CONTINUED)
	{
	  error (0, 0, "waitid WCONTINUED on continued code %d\n",
		 info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGCONT)
	{
	  error (0, 0, "waitid WCONTINUED on continued status %d\n",
		 info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WCONTINUED on continued pid %d != %d\n",
		 info.si_pid, pid);
	  RETURN (EXIT_FAILURE);
	}
    }

  /* Now try a wait that should not succeed.  */
  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  fail = waitid (P_PID, pid, &info, WCONTINUED|WNOHANG);
  switch (fail)
    {
    default:
      error (0, 0, "waitid returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      error (0, errno, "waitid WCONTINUED|WNOHANG on waited continued");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo == 0)
	break;
      if (info.si_signo == SIGCHLD)
	error (0, 0,
	       "waitid WCONTINUED|WNOHANG on waited continued status %d\n",
	       info.si_status);
      else
	error (0, 0,
	       "waitid WCONTINUED|WNOHANG on waited continued signal %d\n",
	       info.si_signo);
      RETURN (EXIT_FAILURE);
    }
#endif

  expecting_sigchld = 1;

  /* Die, child, die!  */
  if (kill (pid, SIGKILL) != 0)
    {
      error (0, errno, "kill (%d, SIGKILL)", pid);
      RETURN (EXIT_FAILURE);
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
      RETURN (EXIT_FAILURE);
    case -1:
      error (0, errno, "waitid WNOWAIT on killed");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WNOWAIT on killed signal %d\n",
		 info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_KILLED)
	{
	  error (0, 0, "waitid WNOWAIT on killed code %d\n",
		 info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGKILL)
	{
	  error (0, 0, "waitid WNOWAIT on killed status %d\n",
		 info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WNOWAIT on killed pid %d != %d\n",
		 info.si_pid, pid);
	  RETURN (EXIT_FAILURE);
	}
    }
#else
  /* Allow enough time to be sure the child died; we didn't synchronize.  */
  sleep (2);
#endif

  CHECK_SIGCHLD ("killed", CLD_KILLED, SIGKILL);

  info.si_signo = 0;		/* A successful call sets it to SIGCHLD.  */
  info.si_pid = -1;
  info.si_status = -1;
  fail = waitid (P_PID, pid, &info, WEXITED|WNOHANG);
  switch (fail)
    {
    default:
      error (0, 0, "waitid WNOHANG returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      error (0, errno, "waitid WNOHANG on killed");
      RETURN (EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  error (0, 0, "waitid WNOHANG on killed signal %d\n",
		 info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_KILLED)
	{
	  error (0, 0, "waitid WNOHANG on killed code %d\n",
		 info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGKILL)
	{
	  error (0, 0, "waitid WNOHANG on killed status %d\n",
		 info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  error (0, 0, "waitid WNOHANG on killed pid %d != %d\n",
		 info.si_pid, pid);
	  RETURN (EXIT_FAILURE);
	}
    }

  fail = waitid (P_PID, pid, &info, WEXITED);
  if (fail == -1)
    {
      if (errno != ECHILD)
	{
	  error (0, errno, "waitid WEXITED on killed");
	  RETURN (EXIT_FAILURE);
	}
    }
  else
    {
      error (0, 0, "waitid WEXITED returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    }

#undef RETURN
 out:
  if (spurious_sigchld)
    status = EXIT_FAILURE;
  signal (SIGCHLD, SIG_IGN);
  kill (pid, SIGKILL);		/* Make sure it's dead if we bailed early.  */
  return status;
}

#include "../test-skeleton.c"
