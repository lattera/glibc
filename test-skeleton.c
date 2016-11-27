/* Skeleton for test programs.
   Copyright (C) 1998-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <malloc.h>
#include <paths.h>
#include <search.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <time.h>
#include <stdint.h>

/* The test function is normally called `do_test' and it is called
   with argc and argv as the arguments.  We nevertheless provide the
   possibility to overwrite this name.

   The TEST_FUNCTION expression should have a type of 'int' and should
   return 0 to indicate a passing test, 1 to indicate a failing test,
   or 77 to indicate an unsupported test.  Other result values could be
   used to indicate a failing test, but the result of the expression
   is passed to exit and exit only returns the lower 8 bits of its input.
   A non-zero return with some values could cause a test to incorrectly
   be considered passing when it really failed. For this reason the
   expression should always return 0, 1, or 77.

   The test function may print out diagnostic or warning messages as well
   as messages about failures.  These messages should be printed to stdout
   and not stderr so that the output is properly ordered with respect to
   the rest of the glibc testsuite run output.  */

#ifndef TEST_FUNCTION
# define TEST_FUNCTION do_test (argc, argv)
#endif

#ifndef TEST_DATA_LIMIT
# define TEST_DATA_LIMIT (64 << 20) /* Data limit (bytes) to run with.  */
#endif

#ifndef TIMEOUT
  /* Default timeout is twenty seconds.  Tests should normally complete faster
     than this, but if they don't, that's abnormal (a bug) anyways.  */
# define TIMEOUT 20
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
static pid_t pid;

/* Directory to place temporary files in.  */
static const char *test_dir;

#define _FAIL(...) \
  printf ("error: %s:%d: ", __FILE__, __LINE__); \
  printf (__VA_ARGS__); \
  printf ("\n"); \

#define FAIL_RET(...) \
  ({ \
     _FAIL (__VA_ARGS__); \
     return 1; \
  })

#define FAIL_EXIT(value, ...) \
  ({ \
     _FAIL (__VA_ARGS__); \
     exit (value); \
  })

#define FAIL_EXIT1(...) FAIL_EXIT(1, __VA_ARGS__)

static void
oom_error (const char *fn, size_t size)
{
  printf ("%s: unable to allocate %zu bytes: %m\n", fn, size);
  exit (1);
}

/* Allocate N bytes of memory dynamically, with error checking.  */
__attribute__ ((unused))
static void *
xmalloc (size_t n)
{
  void *p;

  p = malloc (n);
  if (p == NULL)
    oom_error ("malloc", n);
  return p;
}

/* Allocate memory for N elements of S bytes, with error checking.  */
__attribute__ ((unused))
static void *
xcalloc (size_t n, size_t s)
{
  void *p;

  p = calloc (n, s);
  if (p == NULL)
    oom_error ("calloc", n * s);
  return p;
}

/* Change the size of an allocated block of memory P to N bytes,
   with error checking.  */
__attribute__ ((unused))
static void *
xrealloc (void *p, size_t n)
{
  void *result = realloc (p, n);
  if (result == NULL && (n > 0 || p == NULL))
    oom_error ("realloc", n);
  return result;
}

/* Call asprintf with error checking.  */
__attribute__ ((always_inline, format (printf, 1, 2)))
static __inline__ char *
xasprintf (const char *format, ...)
{
  char *result;
  if (asprintf (&result, format, __builtin_va_arg_pack ()) < 0)
    {
      printf ("error: asprintf: %m\n");
      exit (1);
    }
  return result;
}

/* Write a message to standard output.  Can be used in signal
   handlers.  */
static void
__attribute__ ((unused))
write_message (const char *message)
{
  ssize_t unused __attribute__ ((unused));
  unused = write (STDOUT_FILENO, message, strlen (message));
}

/* List of temporary files.  */
struct temp_name_list
{
  struct qelem q;
  char *name;
} *temp_name_list;

/* Add temporary files in list.  */
static void
__attribute__ ((unused))
add_temp_file (const char *name)
{
  struct temp_name_list *newp
    = (struct temp_name_list *) xcalloc (sizeof (*newp), 1);
  char *newname = strdup (name);
  if (newname != NULL)
    {
      newp->name = newname;
      if (temp_name_list == NULL)
	temp_name_list = (struct temp_name_list *) &newp->q;
      else
	insque (newp, temp_name_list);
    }
  else
    free (newp);
}

/* Delete all temporary files.  */
static void
delete_temp_files (void)
{
  while (temp_name_list != NULL)
    {
      remove (temp_name_list->name);
      free (temp_name_list->name);

      struct temp_name_list *next
	= (struct temp_name_list *) temp_name_list->q.q_forw;
      free (temp_name_list);
      temp_name_list = next;
    }
}

/* Create a temporary file.  Return the opened file descriptor on
   success, or -1 on failure.  Write the file name to *FILENAME if
   FILENAME is not NULL.  In this case, the caller is expected to free
   *FILENAME.  */
static int
__attribute__ ((unused))
create_temp_file (const char *base, char **filename)
{
  char *fname;
  int fd;

  fname = (char *) xmalloc (strlen (test_dir) + 1 + strlen (base)
			    + sizeof ("XXXXXX"));
  strcpy (stpcpy (stpcpy (stpcpy (fname, test_dir), "/"), base), "XXXXXX");

  fd = mkstemp (fname);
  if (fd == -1)
    {
      printf ("cannot open temporary file '%s': %m\n", fname);
      free (fname);
      return -1;
    }

  add_temp_file (fname);
  if (filename != NULL)
    *filename = fname;
  else
    free (fname);

  return fd;
}

/* Timeout handler.  We kill the child and exit with an error.  */
static void
__attribute__ ((noreturn))
signal_handler (int sig __attribute__ ((unused)))
{
  int killed;
  int status;

  assert (pid > 1);
  /* Kill the whole process group.  */
  kill (-pid, SIGKILL);
  /* In case setpgid failed in the child, kill it individually too.  */
  kill (pid, SIGKILL);

  /* Wait for it to terminate.  */
  int i;
  for (i = 0; i < 5; ++i)
    {
      killed = waitpid (pid, &status, WNOHANG|WUNTRACED);
      if (killed != 0)
	break;

      /* Delay, give the system time to process the kill.  If the
	 nanosleep() call return prematurely, all the better.  We
	 won't restart it since this probably means the child process
	 finally died.  */
      struct timespec ts;
      ts.tv_sec = 0;
      ts.tv_nsec = 100000000;
      nanosleep (&ts, NULL);
    }
  if (killed != 0 && killed != pid)
    {
      printf ("Failed to kill test process: %m\n");
      exit (1);
    }

#ifdef CLEANUP_HANDLER
  CLEANUP_HANDLER;
#endif

  if (sig == SIGINT)
    {
      signal (sig, SIG_DFL);
      raise (sig);
    }

  /* If we expected this signal: good!  */
#ifdef EXPECTED_SIGNAL
  if (EXPECTED_SIGNAL == SIGALRM)
    exit (0);
#endif

  if (killed == 0 || (WIFSIGNALED (status) && WTERMSIG (status) == SIGKILL))
    puts ("Timed out: killed the child process");
  else if (WIFSTOPPED (status))
    printf ("Timed out: the child process was %s\n",
	    strsignal (WSTOPSIG (status)));
  else if (WIFSIGNALED (status))
    printf ("Timed out: the child process got signal %s\n",
	    strsignal (WTERMSIG (status)));
  else
    printf ("Timed out: killed the child process but it exited %d\n",
	    WEXITSTATUS (status));

  /* Exit with an error.  */
  exit (1);
}

/* Avoid all the buffer overflow messages on stderr.  */
static void
__attribute__ ((unused))
ignore_stderr (void)
{
  int fd = open (_PATH_DEVNULL, O_WRONLY);
  if (fd == -1)
    close (STDERR_FILENO);
  else
    {
      dup2 (fd, STDERR_FILENO);
      close (fd);
    }
  setenv ("LIBC_FATAL_STDERR_", "1", 1);
}

/* Set fortification error handler.  Used when tests want to verify that bad
   code is caught by the library.  */
static void
__attribute__ ((unused))
set_fortify_handler (void (*handler) (int sig))
{
  struct sigaction sa;

  sa.sa_handler = handler;
  sa.sa_flags = 0;
  sigemptyset (&sa.sa_mask);

  sigaction (SIGABRT, &sa, NULL);
  ignore_stderr ();
}

/* Show people how to run the program.  */
static void
usage (void)
{
  size_t i;

  printf ("Usage: %s [options]\n"
	  "\n"
	  "Environment Variables:\n"
	  "  TIMEOUTFACTOR          An integer used to scale the timeout\n"
	  "  TMPDIR                 Where to place temporary files\n"
	  "\n",
	  program_invocation_short_name);
  printf ("Options:\n");
  for (i = 0; options[i].name; ++i)
    {
      int indent;

      indent = printf ("  --%s", options[i].name);
      if (options[i].has_arg == required_argument)
	indent += printf (" <arg>");
      printf ("%*s", 25 - indent, "");
      switch (options[i].val)
	{
	case OPT_DIRECT:
	  printf ("Run the test directly (instead of forking & monitoring)");
	  break;
	case OPT_TESTDIR:
	  printf ("Override the TMPDIR env var");
	  break;
	}
      printf ("\n");
    }
}

/* We provide the entry point here.  */
int
main (int argc, char *argv[])
{
  int direct = 0;	/* Directly call the test function?  */
  int status;
  int opt;
  unsigned int timeoutfactor = 1;
  pid_t termpid;

#ifndef TEST_NO_MALLOPT
  /* Make uses of freed and uninitialized memory known.  */
  mallopt (M_PERTURB, 42);
#endif

#ifdef STDOUT_UNBUFFERED
  setbuf (stdout, NULL);
#endif

  while ((opt = getopt_long (argc, argv, "+", options, NULL)) != -1)
    switch (opt)
      {
      case '?':
	usage ();
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

  /* If set, read the test TIMEOUTFACTOR value from the environment.
     This value is used to scale the default test timeout values. */
  char *envstr_timeoutfactor = getenv ("TIMEOUTFACTOR");
  if (envstr_timeoutfactor != NULL)
    {
      char *envstr_conv = envstr_timeoutfactor;
      unsigned long int env_fact;

      env_fact = strtoul (envstr_timeoutfactor, &envstr_conv, 0);
      if (*envstr_conv == '\0' && envstr_conv != envstr_timeoutfactor)
	timeoutfactor = MAX (env_fact, 1);
    }

  /* Set TMPDIR to specified test directory.  */
  if (test_dir != NULL)
    {
      setenv ("TMPDIR", test_dir, 1);

      if (chdir (test_dir) < 0)
	{
	  printf ("chdir: %m\n");
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

  /* Make sure temporary files are deleted.  */
  atexit (delete_temp_files);

  /* Correct for the possible parameters.  */
  argv[optind - 1] = argv[0];
  argv += optind - 1;
  argc -= optind - 1;

  /* Call the initializing function, if one is available.  */
#ifdef PREPARE
  PREPARE (argc, argv);
#endif

  const char *envstr_direct = getenv ("TEST_DIRECT");
  if (envstr_direct != NULL)
    {
      FILE *f = fopen (envstr_direct, "w");
      if (f == NULL)
        {
          printf ("cannot open TEST_DIRECT output file '%s': %m\n",
                  envstr_direct);
          exit (1);
        }

      fprintf (f, "timeout=%u\ntimeoutfactor=%u\n", TIMEOUT, timeoutfactor);
#ifdef EXPECTED_STATUS
      fprintf (f, "exit=%u\n", EXPECTED_STATUS);
#endif
#ifdef EXPECTED_SIGNAL
      switch (EXPECTED_SIGNAL)
        {
        default: abort ();
# define init_sig(signo, name, text) \
        case signo: fprintf (f, "signal=%s\n", name); break;
# include <siglist.h>
# undef init_sig
        }
#endif

      if (temp_name_list != NULL)
        {
          struct temp_name_list *n;
          fprintf (f, "temp_files=(\n");
          for (n = temp_name_list;
               n != NULL;
               n = (struct temp_name_list *) n->q.q_forw)
            fprintf (f, "  '%s'\n", n->name);
          fprintf (f, ")\n");
        }

      fclose (f);
      direct = 1;
    }

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

      /* We put the test process in its own pgrp so that if it bogusly
	 generates any job control signals, they won't hit the whole build.  */
      if (setpgid (0, 0) != 0)
	printf ("Failed to set the process group ID: %m\n");

      /* Execute the test function and exit with the return value.   */
      exit (TEST_FUNCTION);
    }
  else if (pid < 0)
    {
      printf ("Cannot fork test program: %m\n");
      exit (1);
    }

  /* Set timeout.  */
  signal (SIGALRM, signal_handler);
  alarm (TIMEOUT * timeoutfactor);

  /* Make sure we clean up if the wrapper gets interrupted.  */
  signal (SIGINT, signal_handler);

  /* Wait for the regular termination.  */
  termpid = TEMP_FAILURE_RETRY (waitpid (pid, &status, 0));
  if (termpid == -1)
    {
      printf ("Waiting for test program failed: %m\n");
      exit (1);
    }
  if (termpid != pid)
    {
      printf ("Oops, wrong test program terminated: expected %ld, got %ld\n",
	      (long int) pid, (long int) termpid);
      exit (1);
    }

  /* Process terminated normaly without timeout etc.  */
  if (WIFEXITED (status))
    {
#ifndef EXPECTED_STATUS
# ifndef EXPECTED_SIGNAL
      /* Simply exit with the return value of the test.  */
      return WEXITSTATUS (status);
# else
      printf ("Expected signal '%s' from child, got none\n",
	      strsignal (EXPECTED_SIGNAL));
      exit (1);
# endif
#else
      if (WEXITSTATUS (status) != EXPECTED_STATUS)
        {
          printf ("Expected status %d, got %d\n",
	          EXPECTED_STATUS, WEXITSTATUS (status));
          exit (1);
        }

      return 0;
#endif
    }
  /* Process was killed by timer or other signal.  */
  else
    {
#ifndef EXPECTED_SIGNAL
      printf ("Didn't expect signal from child: got `%s'\n",
	      strsignal (WTERMSIG (status)));
      exit (1);
#else
      if (WTERMSIG (status) != EXPECTED_SIGNAL)
	{
	  printf ("Incorrect signal from child: got `%s', need `%s'\n",
		  strsignal (WTERMSIG (status)),
		  strsignal (EXPECTED_SIGNAL));
	  exit (1);
	}

      return 0;
#endif
    }
}

/* The following functionality is only available if <pthread.h> was
   included before this file.  */
#ifdef _PTHREAD_H

/* Call pthread_sigmask with error checking.  */
static void
xpthread_sigmask (int how, const sigset_t *set, sigset_t *oldset)
{
  if (pthread_sigmask (how, set, oldset) != 0)
    {
      write_message ("error: pthread_setmask failed\n");
      _exit (1);
    }
}

/* Call pthread_mutex_lock with error checking.  */
__attribute__ ((unused))
static void
xpthread_mutex_lock (pthread_mutex_t *mutex)
{
  int ret = pthread_mutex_lock (mutex);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_mutex_lock: %m\n");
      exit (1);
    }
}

/* Call pthread_spin_lock with error checking.  */
__attribute__ ((unused))
static void
xpthread_spin_lock (pthread_spinlock_t *lock)
{
  int ret = pthread_spin_lock (lock);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_spin_lock: %m\n");
      exit (1);
    }
}

/* Call pthread_cond_wait with error checking.  */
__attribute__ ((unused))
static void
xpthread_cond_wait (pthread_cond_t * cond,
		    pthread_mutex_t * mutex)
{
  int ret = pthread_cond_wait (cond, mutex);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_cond_wait: %m\n");
      exit (1);
    }
}

/* Call pthread_barrier_wait with error checking.  */
__attribute__ ((unused))
static int
xpthread_barrier_wait (pthread_barrier_t *barrier)
{
  int ret = pthread_barrier_wait (barrier);
  if (ret != 0 && ret != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      errno = ret;
      printf ("error: pthread_barrier_wait: %m\n");
      exit (1);
    }
  return ret;
}

/* Call pthread_create with error checking.  */
static pthread_t
xpthread_create (pthread_attr_t *attr,
		 void *(*thread_func) (void *), void *closure)
{
  pthread_t thr;
  int ret = pthread_create (&thr, attr, thread_func, closure);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_create: %m\n");
      exit (1);
    }
  return thr;
}

/* Call pthread_detach with error checking.  */
static void
xpthread_detach (pthread_t thr)
{
  int ret = pthread_detach (thr);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_detach: %m\n");
      exit (1);
    }
}

/* Call pthread_join with error checking.  */
__attribute__ ((unused))
static void *
xpthread_join (pthread_t thr)
{
  void *result;
  int ret = pthread_join (thr, &result);
  if (ret != 0)
    {
      errno = ret;
      printf ("error: pthread_join: %m\n");
      exit (1);
    }
  return result;
}

/* Used to implement the delayed_exit function defined below.  */
static void *
delayed_exit_thread (void *seconds_as_ptr)
{
  int seconds = (uintptr_t) seconds_as_ptr;
  struct timespec delay = { seconds, 0 };
  struct timespec remaining = { 0 };
  if (nanosleep (&delay, &remaining) != 0)
    {
      printf ("error: nanosleep: %m\n");
      _exit (1);
    }
  /* Exit the process sucessfully.  */
  exit (0);
  return NULL;
}

/* Exit (with status 0) after SECONDS have elapsed, from a helper
   thread.  The process is terminated with the exit function, so
   atexit handlers are executed.  */
__attribute__ ((unused))
static void
delayed_exit (int seconds)
{
  /* Create the new thread with all signals blocked.  */
  sigset_t all_blocked;
  sigfillset (&all_blocked);
  sigset_t old_set;
  xpthread_sigmask (SIG_SETMASK, &all_blocked, &old_set);
  /* Create a detached thread. */
  pthread_t thr = xpthread_create
    (NULL, delayed_exit_thread, (void *) (uintptr_t) seconds);
  xpthread_detach (thr);
  /* Restore the original signal mask.  */
  xpthread_sigmask (SIG_SETMASK, &old_set, NULL);
}

#endif	/* _PTHREAD_H */
