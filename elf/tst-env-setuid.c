/* Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

/* Verify that tunables correctly filter out unsafe environment variables like
   MALLOC_CHECK_ and MALLOC_MMAP_THRESHOLD_ but also retain
   MALLOC_MMAP_THRESHOLD_ in an unprivileged child.  */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

#include <support/support.h>
#include <support/test-driver.h>

static char SETGID_CHILD[] = "setgid-child";
#define CHILD_STATUS 42

/* Return a GID which is not our current GID, but is present in the
   supplementary group list.  */
static gid_t
choose_gid (void)
{
  const int count = 64;
  gid_t groups[count];
  int ret = getgroups (count, groups);
  if (ret < 0)
    {
      printf ("getgroups: %m\n");
      exit (1);
    }
  gid_t current = getgid ();
  for (int i = 0; i < ret; ++i)
    {
      if (groups[i] != current)
	return groups[i];
    }
  return 0;
}

/* Spawn and execute a program and verify that it returns the CHILD_STATUS.  */
static pid_t
do_execve (char **args)
{
  pid_t kid = vfork ();

  if (kid < 0)
    {
      printf ("vfork: %m\n");
      return -1;
    }

  if (kid == 0)
    {
      /* Child process.  */
      execve (args[0], args, environ);
      _exit (-errno);
    }

  if (kid < 0)
    return 1;

  int status;

  if (waitpid (kid, &status, 0) < 0)
    {
      printf ("waitpid: %m\n");
      return 1;
    }

  if (WEXITSTATUS (status) == EXIT_UNSUPPORTED)
    return EXIT_UNSUPPORTED;

  if (!WIFEXITED (status) || WEXITSTATUS (status) != CHILD_STATUS)
    {
      printf ("Unexpected exit status %d from child process\n",
	      WEXITSTATUS (status));
      return 1;
    }
  return 0;
}

/* Copies the executable into a restricted directory, so that we can
   safely make it SGID with the TARGET group ID.  Then runs the
   executable.  */
static int
run_executable_sgid (gid_t target)
{
  char *dirname = xasprintf ("%s/tst-tunables-setuid.%jd",
			     test_dir, (intmax_t) getpid ());
  char *execname = xasprintf ("%s/bin", dirname);
  int infd = -1;
  int outfd = -1;
  int ret = 0;
  if (mkdir (dirname, 0700) < 0)
    {
      printf ("mkdir: %m\n");
      goto err;
    }
  infd = open ("/proc/self/exe", O_RDONLY);
  if (infd < 0)
    {
      printf ("open (/proc/self/exe): %m\n");
      goto err;
    }
  outfd = open (execname, O_WRONLY | O_CREAT | O_EXCL, 0700);
  if (outfd < 0)
    {
      printf ("open (%s): %m\n", execname);
      goto err;
    }
  char buf[4096];
  for (;;)
    {
      ssize_t rdcount = read (infd, buf, sizeof (buf));
      if (rdcount < 0)
	{
	  printf ("read: %m\n");
	  goto err;
	}
      if (rdcount == 0)
	break;
      char *p = buf;
      char *end = buf + rdcount;
      while (p != end)
	{
	  ssize_t wrcount = write (outfd, buf, end - p);
	  if (wrcount == 0)
	    errno = ENOSPC;
	  if (wrcount <= 0)
	    {
	      printf ("write: %m\n");
	      goto err;
	    }
	  p += wrcount;
	}
    }
  if (fchown (outfd, getuid (), target) < 0)
    {
      printf ("fchown (%s): %m\n", execname);
      goto err;
    }
  if (fchmod (outfd, 02750) < 0)
    {
      printf ("fchmod (%s): %m\n", execname);
      goto err;
    }
  if (close (outfd) < 0)
    {
      printf ("close (outfd): %m\n");
      goto err;
    }
  if (close (infd) < 0)
    {
      printf ("close (infd): %m\n");
      goto err;
    }

  char *args[] = {execname, SETGID_CHILD, NULL};

  ret = do_execve (args);

err:
  if (outfd >= 0)
    close (outfd);
  if (infd >= 0)
    close (infd);
  if (execname)
    {
      unlink (execname);
      free (execname);
    }
  if (dirname)
    {
      rmdir (dirname);
      free (dirname);
    }
  return ret;
}

#ifndef test_child
static int
test_child (void)
{
  if (getenv ("MALLOC_CHECK_") != NULL)
    {
      printf ("MALLOC_CHECK_ is still set\n");
      return 1;
    }

  if (getenv ("MALLOC_MMAP_THRESHOLD_") == NULL)
    {
      printf ("MALLOC_MMAP_THRESHOLD_ lost\n");
      return 1;
    }

  if (getenv ("LD_HWCAP_MASK") != NULL)
    {
      printf ("LD_HWCAP_MASK still set\n");
      return 1;
    }

  return 0;
}
#endif

#ifndef test_parent
static int
test_parent (void)
{
  if (getenv ("MALLOC_CHECK_") == NULL)
    {
      printf ("MALLOC_CHECK_ lost\n");
      return 1;
    }

  if (getenv ("MALLOC_MMAP_THRESHOLD_") == NULL)
    {
      printf ("MALLOC_MMAP_THRESHOLD_ lost\n");
      return 1;
    }

  if (getenv ("LD_HWCAP_MASK") == NULL)
    {
      printf ("LD_HWCAP_MASK lost\n");
      return 1;
    }

  return 0;
}
#endif

static int
do_test (int argc, char **argv)
{
  /* Setgid child process.  */
  if (argc == 2 && strcmp (argv[1], SETGID_CHILD) == 0)
    {
      if (getgid () == getegid ())
	{
	  /* This can happen if the file system is mounted nosuid.  */
	  fprintf (stderr, "SGID failed: GID and EGID match (%jd)\n",
		   (intmax_t) getgid ());
	  exit (EXIT_UNSUPPORTED);
	}

      int ret = test_child ();

      if (ret != 0)
	exit (1);

      exit (CHILD_STATUS);
    }
  else
    {
      if (test_parent () != 0)
	exit (1);

      /* Try running a setgid program.  */
      gid_t target = choose_gid ();
      if (target == 0)
	{
	  fprintf (stderr,
		   "Could not find a suitable GID for user %jd, skipping test\n",
		   (intmax_t) getuid ());
	  exit (0);
	}

      return run_executable_sgid (target);
    }

  /* Something went wrong and our argv was corrupted.  */
  _exit (1);
}

#define TEST_FUNCTION_ARGV do_test
#include <support/test-driver.c>
