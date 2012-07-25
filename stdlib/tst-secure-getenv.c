/* Copyright (C) 2012 Free Software Foundation, Inc.
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

/* Test that secure_getenv works by invoking the test as a SGID
   program with a group ID from the supplementary group list.  This
   test can fail spuriously if the user is not a member of a suitable
   supplementary group. */

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static char MAGIC_ARGUMENT[] = "run-actual-test";
#define MAGIC_STATUS 19

static const char *test_dir;

/* Return a GID which is not our current GID, but is present in the
   supplementary group list. */
static gid_t
choose_gid (void)
{
  const int count = 64;
  gid_t groups[count];
  int ret = getgroups (count, groups);
  if (ret < 0)
    {
      perror ("getgroups");
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


/* Copies the executable into a restricted directory, so that we can
   safely make it SGID with the TARGET group ID.  Then runs the
   executable. */
static int
run_executable_sgid (gid_t target)
{
  char *dirname = 0;
  char *execname = 0;
  int infd = -1;
  int outfd = -1;
  int ret = -1;
  if (asprintf (&dirname, "%s/secure-getenv.%jd",
		test_dir, (intmax_t) getpid ()) < 0)
    {
      perror ("asprintf");
      goto err;
    }
  if (mkdir (dirname, 0700) < 0)
    {
      perror ("mkdir");
      goto err;
    }
  if (asprintf (&execname, "%s/bin", dirname) < 0)
    {
      perror ("asprintf");
      goto err;
    }
  infd = open ("/proc/self/exe", O_RDONLY);
  if (infd < 0)
    {
      perror ("open");
      goto err;
    }
  outfd = open (execname, O_WRONLY | O_CREAT | O_EXCL, 0700);
  if (outfd < 0)
    {
      perror ("open");
      goto err;
    }
  char buf[4096];
  for (;;)
    {
      ssize_t rdcount = read (infd, buf, sizeof (buf));
      if (rdcount < 0)
	{
	  perror ("read");
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
	      perror ("write");
	      goto err;
	    }
	  p += wrcount;
	}
    }
  if (fchown (outfd, getuid (), target) < 0)
    {
      perror ("fchown");
      goto err;
    }
  if (fchmod (outfd, 02750) < 0)
    {
      perror ("fchmod");
      goto err;
    }
  if (close (outfd) < 0)
    {
      perror ("close");
      goto err;
    }
  if (close (infd) < 0)
    {
      perror ("close");
      goto err;
    }

  int kid = fork ();
  if (kid < 0)
    {
      perror ("fork");
      goto err;
    }
  if (kid == 0)
    {
      /* Child process. */
      char *args[] = { execname, MAGIC_ARGUMENT, NULL };
      execve (execname, args, environ);
      perror ("execve");
      _exit (1);
    }
  int status;
  if (waitpid (kid, &status, 0) < 0)
    {
      perror ("waitpid");
      goto err;
    }
  if (!WIFEXITED (status) || WEXITSTATUS (status) != MAGIC_STATUS)
    {
      fprintf (stderr, "Unexpected exit status %d from child process\n",
	       status);
      goto err;
    }
  ret = 0;

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

static int
do_test (void)
{
  if (getenv ("PATH") == NULL)
    {
      fprintf (stderr, "PATH not set\n");
      exit (1);
    }
  if (secure_getenv ("PATH") == NULL)
    {
      fprintf (stderr, "PATH not set according to secure_getenv\n");
      exit (1);
    }
  if (strcmp (getenv ("PATH"), secure_getenv ("PATH")) != 0)
    {
      fprintf (stderr, "PATH mismatch (%s, %s)\n",
	       getenv ("PATH"), secure_getenv ("PATH"));
      exit (1);
    }

  gid_t target = choose_gid ();
  if (target == 0)
    {
      fprintf (stderr, "Could not find a suitable GID user %jd\n",
	       (intmax_t) getuid ());
      exit (1);
    }
  return run_executable_sgid (target);
}

static void
alternative_main (int argc, char **argv)
{
  if (argc == 2 && strcmp (argv[1], MAGIC_ARGUMENT) == 0)
    {
      if (getgid () == getegid ())
	{
	  fprintf (stderr, "SGID failed: GID and EGID match (%jd)\n",
		   (intmax_t) getgid ());
	  exit (2);
	}
      if (getenv ("PATH") == NULL)
	{
	  fprintf (stderr, "PATH variable not present\n");
	  exit (3);
	}
      if (secure_getenv ("PATH") != NULL)
	{
	  fprintf (stderr, "PATH variable not filtered out\n");
	  exit (4);
	}
      exit (MAGIC_STATUS);
    }
}

#define PREPARE(argc, argv) alternative_main(argc, argv)
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
