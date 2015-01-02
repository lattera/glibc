/* Test for vfork functions.
   Copyright (C) 2007-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2007.

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
#include <fcntl.h>
#include <mcheck.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static int do_test (void);
static void do_prepare (void);
char *tmpdirname;

#define TEST_FUNCTION do_test ()
#define PREPARE(argc, argv) do_prepare ()
#include "../test-skeleton.c"

static int
do_test (void)
{
  mtrace ();

  const char *path = getenv ("PATH");
  if (path == NULL)
    path = "/bin";
  char pathbuf[strlen (tmpdirname) + 1 + strlen (path) + 1];
  strcpy (stpcpy (stpcpy (pathbuf, tmpdirname), ":"), path);
  if (setenv ("PATH", pathbuf, 1) < 0)
    {
      puts ("setenv failed");
      return 1;
    }

  size_t i;
  char *argv[3] = { (char *) "script1.sh", (char *) "1", NULL };
  for (i = 0; i < 5; i++)
    {
      pid_t pid = vfork ();
      if (pid < 0)
	{
	  printf ("vfork failed: %m\n");
	  return 1;
	}
      else if (pid == 0)
	{
	  execvp ("script1.sh", argv);
	  _exit (errno);
	}
      int status;
      if (TEMP_FAILURE_RETRY (waitpid (pid, &status, 0)) != pid)
	{
	  puts ("waitpid failed");
	  return 1;
	}
      else if (status != 0)
	{
	  if (WIFEXITED (status))
	    printf ("script1.sh failed with status %d\n",
		    WEXITSTATUS (status));
	  else
	    printf ("script1.sh kill by signal %d\n",
		    WTERMSIG (status));
	  return 1;
	}
    }

  argv[0] = (char *) "script2.sh";
  argv[1] = (char *) "2";
  for (i = 0; i < 5; i++)
    {
      pid_t pid = vfork ();
      if (pid < 0)
	{
	  printf ("vfork failed: %m\n");
	  return 1;
	}
      else if (pid == 0)
	{
	  execvp ("script2.sh", argv);
	  _exit (errno);
	}
      int status;
      if (TEMP_FAILURE_RETRY (waitpid (pid, &status, 0)) != pid)
	{
	  puts ("waitpid failed");
	  return 1;
	}
      else if (status != 0)
	{
	  printf ("script2.sh failed with status %d\n", status);
	  return 1;
	}
    }

  for (i = 0; i < 5; i++)
    {
      pid_t pid = vfork ();
      if (pid < 0)
	{
	  printf ("vfork failed: %m\n");
	  return 1;
	}
      else if (pid == 0)
	{
	  execlp ("script2.sh", "script2.sh", "3", NULL);
	  _exit (errno);
	}
      int status;
      if (TEMP_FAILURE_RETRY (waitpid (pid, &status, 0)) != pid)
	{
	  puts ("waitpid failed");
	  return 1;
	}
      else if (status != 0)
	{
	  printf ("script2.sh failed with status %d\n", status);
	  return 1;
	}
    }

  unsetenv ("PATH");
  argv[0] = (char *) "echo";
  argv[1] = (char *) "script 4";
  for (i = 0; i < 5; i++)
    {
      pid_t pid = vfork ();
      if (pid < 0)
	{
	  printf ("vfork failed: %m\n");
	  return 1;
	}
      else if (pid == 0)
	{
	  execvp ("echo", argv);
	  _exit (errno);
	}
      int status;
      if (TEMP_FAILURE_RETRY (waitpid (pid, &status, 0)) != pid)
	{
	  puts ("waitpid failed");
	  return 1;
	}
      else if (status != 0)
	{
	  printf ("echo failed with status %d\n", status);
	  return 1;
	}
    }

  return 0;
}

static void
do_prepare (void)
{
  size_t len = strlen (test_dir) + sizeof ("/tst-vfork3.XXXXXX");
  tmpdirname = malloc (len);
  char *script1 = malloc (len + sizeof "/script1.sh");
  char *script2 = malloc (len + sizeof "/script2.sh");
  if (tmpdirname == NULL || script1 == NULL || script2 == NULL)
    {
      puts ("out of memory");
      exit (1);
    }
  strcpy (stpcpy (tmpdirname, test_dir), "/tst-vfork3.XXXXXX");

  tmpdirname = mkdtemp (tmpdirname);
  if (tmpdirname == NULL)
    {
      puts ("could not create temporary directory");
      exit (1);
    }

  strcpy (stpcpy (script1, tmpdirname), "/script1.sh");
  strcpy (stpcpy (script2, tmpdirname), "/script2.sh");

  /* Need to make sure tmpdirname is at the end of the linked list.  */
  add_temp_file (script1);
  add_temp_file (tmpdirname);
  add_temp_file (script2);

  const char content1[] = "#!/bin/sh\necho script $1\n";
  int fd = open (script1, O_WRONLY | O_CREAT, 0700);
  if (fd < 0
      || TEMP_FAILURE_RETRY (write (fd, content1, sizeof content1))
	 != sizeof content1
      || fchmod (fd, S_IRUSR | S_IXUSR) < 0)
    {
      printf ("Could not write %s\n", script1);
      exit (1);
    }
  close (fd);

  const char content2[] = "echo script $1\n";
  fd = open (script2, O_WRONLY | O_CREAT, 0700);
  if (fd < 0
      || TEMP_FAILURE_RETRY (write (fd, content2, sizeof content2))
	 != sizeof content2
      || fchmod (fd, S_IRUSR | S_IXUSR) < 0)
    {
      printf ("Could not write %s\n", script2);
      exit (1);
    }
  close (fd);
}
