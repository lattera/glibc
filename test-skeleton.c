/* Skeleton for test programs.
   Copyright (C) 1998, 2000 Free Software Foundation, Inc.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <getopt.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>

/* The test function is normally called `do_test' and it is called
   with argc and argv as the arguments.  We nevertheless provide the
   possibility to overwrite this name.  */
#ifndef TEST_FUNCTION
# define TEST_FUNCTION do_test (argc, argv)
#endif


#define OPT_DIRECT 1000
#define OPT_TESTDIR 1001

static struct option options[] =
{
#ifdef CMDLINE_OPTIONS
  CMDLINE_OPTIONS
#endif
  { "direct", no_argument, NULL, OPT_DIRECT },
  { "test-dir", required_argument, NULL, OPT_TESTDIR },
  { NULL, 0, NULL, 0 }
};

/* PID of the test itself.  */
static int pid;

/* Directory to place temporary files in.  */
static const char *test_dir;

/* List of temporary files.  */
struct temp_name_list
{
  struct qelem q;
  const char *name;
} *temp_name_list;

/* Add temporary files in list.  */
static void
add_temp_file (const char *name)
{
  struct temp_name_list *newp
    = (struct temp_name_list *) calloc (sizeof (*newp), 1);
  if (newp != NULL)
    {
      newp->name = name;
      if (temp_name_list == NULL)
	temp_name_list = (struct temp_name_list *) &newp->q;
      else
	insque (newp, temp_name_list);
    }
}

/* Delete all temporary files.  */
static void
delete_temp_files (void)
{
  while (temp_name_list != NULL)
    {
      remove (temp_name_list->name);
      temp_name_list = (struct temp_name_list *) temp_name_list->q.q_forw;
    }
}

/* Timeout handler.  We kill the child and exit with an error.  */
static void
__attribute__ ((noreturn))
timeout_handler (int sig __attribute__ ((unused)))
{
  int killed;

  /* Send signal.  */
  kill (pid, SIGKILL);

  /* Wait for it to terminate.  */
  killed = waitpid (pid, NULL, WNOHANG);
  if (killed != 0 && killed != pid)
    {
      perror ("Failed to killed test process");
      exit (1);
    }

#ifdef CLEANUP_HANDLER
  CLEANUP_HANDLER;
#endif

  fputs ("Timed out: killed the child process\n", stderr);

  /* Exit with an error.  */
  exit (1);
}

/* We provide the entry point here.  */
int
main (int argc, char *argv[])
{
  int direct = 0;	/* Directly call the test function?  */
  int status;
  int opt;

#ifdef STDOUT_UNBUFFERED
  setbuf (stdout, NULL);
#endif

  while ((opt = getopt_long (argc, argv, "", options, NULL)) != -1)
    switch (opt)
      {
      case '?':
	exit (1);
      case OPT_DIRECT:
	direct = 1;
	break;
      case OPT_TESTDIR:
	test_dir = optarg;
	break;
#ifdef CMDLINE_PROCESS
	CMDLINE_PROCESS
#endif
      }

  /* Set TMPDIR to specified test directory.  */
  if (test_dir != NULL)
    {
      setenv ("TMPDIR", test_dir, 1);

      if (chdir (test_dir) < 0)
	{
	  perror ("chdir");
	  exit (1);
	}
    }
  else
    {
      test_dir = getenv ("TMPDIR");
      if (test_dir == NULL || test_dir[0] == '\0')
	test_dir = "/tmp";
    }

  /* Make sure we see all message, even those on stdout.  */
  setvbuf (stdout, NULL, _IONBF, 0);

  /* make sure temporary files are deleted.  */
  atexit (delete_temp_files);

  /* Correct for the possible parameters.  */
  argv += optind - 1;
  argc -= optind - 1;

  /* Call the initializing function, if one is available.  */
#ifdef PREPARE
  PREPARE (argc, argv);
#endif

  /* If we are not expected to fork run the function immediately.  */
  if (direct)
    return TEST_FUNCTION;

  /* Set up the test environment:
     - prevent core dumps
     - set up the timer
     - fork and execute the function.  */

  pid = fork ();
  if (pid == 0)
    {
      /* This is the child.  */
#ifdef RLIMIT_CORE
      /* Try to avoid dumping core.  */
      struct rlimit core_limit;
      core_limit.rlim_cur = 0;
      core_limit.rlim_max = 0;
      setrlimit (RLIMIT_CORE, &core_limit);
#endif

      /* Execute the test function and exit with the return value.   */
      exit (TEST_FUNCTION);
    }
  else if (pid < 0)
    {
      perror ("Cannot fork test program");
      exit (1);
    }

  /* Set timeout.  */
#ifndef TIMEOUT
  /* Default timeout is two seconds.  */
# define TIMEOUT 2
#endif
  alarm (TIMEOUT);
  signal (SIGALRM, timeout_handler);

  /* Wait for the regular termination.  */
  if (waitpid (pid, &status, 0) != pid)
    {
      perror ("Oops, wrong test program terminated");
      exit (1);
    }

#ifndef EXPECTED_SIGNAL
  /* We don't expect any signal.  */
# define EXPECTED_SIGNAL 0
#endif
  if (WTERMSIG (status) != EXPECTED_SIGNAL)
    {
      if (EXPECTED_SIGNAL != 0)
	fprintf (stderr, "Incorrect signal from child: got `%s', need `%s'\n",
		 strsignal (WTERMSIG (status)), strsignal (EXPECTED_SIGNAL));
      else
	fprintf (stderr, "Didn't expect signal from child: got `%s'\n",
		 strsignal (WTERMSIG (status)));
      exit (1);
    }

  /* Simply exit with the return value of the test.  */
  return WEXITSTATUS (status);
}
