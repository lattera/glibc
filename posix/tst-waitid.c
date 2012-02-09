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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <stdio.h>
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
      printf ("SIGCHLD handler got signal %d instead!\n", signo);
      _exit (EXIT_FAILURE);
    }

  if (! expecting_sigchld)
    {
      spurious_sigchld = 1;
      printf ("spurious SIGCHLD: signo %d code %d status %d pid %d\n",
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
      printf ("missing SIGCHLD on %s\n", phase);
      *ok = EXIT_FAILURE;
      expecting_sigchld = 0;
      return;
    }

  if (sigchld_info.si_signo != SIGCHLD)
    {
      printf ("SIGCHLD for %s signal %d\n", phase, sigchld_info.si_signo);
      *ok = EXIT_FAILURE;
    }
  if (sigchld_info.si_code != code)
    {
      printf ("SIGCHLD for %s code %d\n", phase, sigchld_info.si_code);
      *ok = EXIT_FAILURE;
    }
  if (sigchld_info.si_status != status)
    {
      printf ("SIGCHLD for %s status %d\n", phase, sigchld_info.si_status);
      *ok = EXIT_FAILURE;
    }
  if (sigchld_info.si_pid != pid)
    {
      printf ("SIGCHLD for %s pid %d\n", phase, sigchld_info.si_pid);
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
      printf ("setting SIGCHLD handler: %m\n");
      return EXIT_FAILURE;
    }
#endif

  expecting_sigchld = 1;

  pid_t pid = fork ();
  if (pid < 0)
    {
      printf ("fork: %m\n");
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
      printf ("waitid returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      printf ("waitid WNOHANG on stopped: %m\n");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo == 0)
	break;
      if (info.si_signo == SIGCHLD)
	printf ("waitid WNOHANG on stopped status %d\n", info.si_status);
      else
	printf ("waitid WNOHANG on stopped signal %d\n", info.si_signo);
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
      printf ("waitid WSTOPPED|WNOHANG returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      printf ("waitid WSTOPPED|WNOHANG on stopped: %m\n");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  printf ("waitid WSTOPPED|WNOHANG on stopped signal %d\n",
		  info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_STOPPED)
	{
	  printf ("waitid WSTOPPED|WNOHANG on stopped code %d\n",
		  info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGSTOP)
	{
	  printf ("waitid WSTOPPED|WNOHANG on stopped status %d\n",
		  info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  printf ("waitid WSTOPPED|WNOHANG on stopped pid %d != %d\n",
		  info.si_pid, pid);
	  RETURN (EXIT_FAILURE);
	}
    }

  expecting_sigchld = WCONTINUED != 0;

  if (kill (pid, SIGCONT) != 0)
    {
      printf ("kill (%d, SIGCONT): %m\n", pid);
      RETURN (EXIT_FAILURE);
    }

  /* Wait for the child to have continued.  */
  sleep (2);

#if WCONTINUED != 0
  if (expecting_sigchld)
    {
      printf ("no SIGCHLD seen for SIGCONT (optional)\n");
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
      printf ("waitid WCONTINUED|WNOWAIT returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      printf ("waitid WCONTINUED|WNOWAIT on continued: %m\n");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  printf ("waitid WCONTINUED|WNOWAIT on continued signal %d\n",
		  info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_CONTINUED)
	{
	  printf ("waitid WCONTINUED|WNOWAIT on continued code %d\n",
		  info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGCONT)
	{
	  printf ("waitid WCONTINUED|WNOWAIT on continued status %d\n",
		  info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  printf ("waitid WCONTINUED|WNOWAIT on continued pid %d != %d\n",
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
      printf ("waitid WCONTINUED returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      printf ("waitid WCONTINUED on continued: %m\n");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  printf ("waitid WCONTINUED on continued signal %d\n", info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_CONTINUED)
	{
	  printf ("waitid WCONTINUED on continued code %d\n", info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGCONT)
	{
	  printf ("waitid WCONTINUED on continued status %d\n",
		  info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  printf ("waitid WCONTINUED on continued pid %d != %d\n",
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
      printf ("waitid returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      printf ("waitid WCONTINUED|WNOHANG on waited continued: %m\n");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo == 0)
	break;
      if (info.si_signo == SIGCHLD)
	printf ("waitid WCONTINUED|WNOHANG on waited continued status %d\n",
		info.si_status);
      else
	printf ("waitid WCONTINUED|WNOHANG on waited continued signal %d\n",
		info.si_signo);
      RETURN (EXIT_FAILURE);
    }

  /* Now stop him again and test waitpid with WCONTINUED.  */
  expecting_sigchld = 1;
  if (kill (pid, SIGSTOP) != 0)
    {
      printf ("kill (%d, SIGSTOP): %m\n", pid);
      RETURN (EXIT_FAILURE);
    }
  pid_t wpid = waitpid (pid, &fail, WUNTRACED);
  if (wpid < 0)
    {
      printf ("waitpid WUNTRACED on stopped: %m\n");
      RETURN (EXIT_FAILURE);
    }
  else if (wpid != pid)
    {
      printf ("waitpid WUNTRACED on stopped returned %d != %d (status %x)\n",
	      wpid, pid, fail);
      RETURN (EXIT_FAILURE);
    }
  else if (!WIFSTOPPED (fail) || WIFSIGNALED (fail) || WIFEXITED (fail)
	   || WIFCONTINUED (fail) || WSTOPSIG (fail) != SIGSTOP)
    {
      printf ("waitpid WUNTRACED on stopped: status %x\n", fail);
      RETURN (EXIT_FAILURE);
    }
  CHECK_SIGCHLD ("stopped", CLD_STOPPED, SIGSTOP);

  expecting_sigchld = 1;
  if (kill (pid, SIGCONT) != 0)
    {
      printf ("kill (%d, SIGCONT): %m\n", pid);
      RETURN (EXIT_FAILURE);
    }

  /* Wait for the child to have continued.  */
  sleep (2);

  if (expecting_sigchld)
    {
      printf ("no SIGCHLD seen for SIGCONT (optional)\n");
      expecting_sigchld = 0;
    }
  else
    CHECK_SIGCHLD ("continued", CLD_CONTINUED, SIGCONT);

  wpid = waitpid (pid, &fail, WCONTINUED);
  if (wpid < 0)
    {
      if (errno == EINVAL)
	printf ("waitpid does not support WCONTINUED\n");
      else
	{
	  printf ("waitpid WCONTINUED on continued: %m\n");
	  RETURN (EXIT_FAILURE);
	}
    }
  else if (wpid != pid)
    {
      printf ("\
waitpid WCONTINUED on continued returned %d != %d (status %x)\n",
	     wpid, pid, fail);
      RETURN (EXIT_FAILURE);
    }
  else if (WIFSTOPPED (fail) || WIFSIGNALED (fail) || WIFEXITED (fail)
	   || !WIFCONTINUED (fail))
    {
      printf ("waitpid WCONTINUED on continued: status %x\n", fail);
      RETURN (EXIT_FAILURE);
    }
#endif

  expecting_sigchld = 1;

  /* Die, child, die!  */
  if (kill (pid, SIGKILL) != 0)
    {
      printf ("kill (%d, SIGKILL): %m\n", pid);
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
      printf ("waitid WNOWAIT returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      printf ("waitid WNOWAIT on killed: %m\n");
      RETURN (errno == ENOTSUP ? EXIT_SUCCESS : EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  printf ("waitid WNOWAIT on killed signal %d\n", info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_KILLED)
	{
	  printf ("waitid WNOWAIT on killed code %d\n", info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGKILL)
	{
	  printf ("waitid WNOWAIT on killed status %d\n", info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  printf ("waitid WNOWAIT on killed pid %d != %d\n", info.si_pid, pid);
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
      printf ("waitid WNOHANG returned bogus value %d\n", fail);
      RETURN (EXIT_FAILURE);
    case -1:
      printf ("waitid WNOHANG on killed: %m\n");
      RETURN (EXIT_FAILURE);
    case 0:
      if (info.si_signo != SIGCHLD)
	{
	  printf ("waitid WNOHANG on killed signal %d\n", info.si_signo);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_code != CLD_KILLED)
	{
	  printf ("waitid WNOHANG on killed code %d\n", info.si_code);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_status != SIGKILL)
	{
	  printf ("waitid WNOHANG on killed status %d\n", info.si_status);
	  RETURN (EXIT_FAILURE);
	}
      if (info.si_pid != pid)
	{
	  printf ("waitid WNOHANG on killed pid %d != %d\n", info.si_pid, pid);
	  RETURN (EXIT_FAILURE);
	}
    }

  fail = waitid (P_PID, pid, &info, WEXITED);
  if (fail == -1)
    {
      if (errno != ECHILD)
	{
	  printf ("waitid WEXITED on killed: %m\n");
	  RETURN (EXIT_FAILURE);
	}
    }
  else
    {
      printf ("waitid WEXITED returned bogus value %d\n", fail);
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
