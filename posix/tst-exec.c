/* Tests for exec.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2000.

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
#include <error.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>


/* Nonzero if the program gets called via `exec'.  */
static int restart;


#define CMDLINE_OPTIONS \
  { "restart", no_argument, &restart, 1 },

/* Prototype for our test function.  */
extern void do_prepare (int argc, char *argv[]);
extern int do_test (int argc, char *argv[]);

/* We have a preparation function.  */
#define PREPARE do_prepare

#include "../test-skeleton.c"


/* Name of the temporary files.  */
static char *name1;
static char *name2;

/* The contents of our files.  */
static const char fd1string[] = "This file should get closed";
static const char fd2string[] = "This file should stay opened";


/* We have a preparation function.  */
void
do_prepare (int argc, char *argv[])
{
   size_t name_len;

   name_len = strlen (test_dir);
   name1 = malloc (name_len + sizeof ("/execXXXXXX"));
   mempcpy (mempcpy (name1, test_dir, name_len),
	    "/execXXXXXX", sizeof ("/execXXXXXX"));
   add_temp_file (name1);

   name2 = malloc (name_len + sizeof ("/execXXXXXX"));
   mempcpy (mempcpy (name2, test_dir, name_len),
	    "/execXXXXXX", sizeof ("/execXXXXXX"));
   add_temp_file (name2);
}


static int
handle_restart (const char *fd1s, const char *fd2s, const char *name)
{
  char buf[100];
  int fd1;
  int fd2;

  /* First get the descriptors.  */
  fd1 = atol (fd1s);
  fd2 = atol (fd2s);

  /* Sanity check.  */
  if (fd1 == fd2)
    error (EXIT_FAILURE, 0, "value of fd1 and fd2 is the same");

  /* First the easy part: read from the file descriptor which is
     supposed to be open.  */
  if (lseek (fd2, 0, SEEK_CUR) != strlen (fd2string))
    error (EXIT_FAILURE, errno, "file 2 not in right position");
  if (lseek (fd2, 0, SEEK_SET) != 0)
    error (EXIT_FAILURE, 0, "cannot reset position in file 2");
  if (read (fd2, buf, sizeof buf) != strlen (fd2string))
    error (EXIT_FAILURE, 0, "cannot read file 2");
  if (memcmp (fd2string, buf, strlen (fd2string)) != 0)
    error (EXIT_FAILURE, 0, "file 2 does not match");

  /* No try to read the first file.  First make sure it is not opened.  */
  if (lseek (fd1, 0, SEEK_CUR) != (off_t) -1 || errno != EBADF)
    error (EXIT_FAILURE, 0, "file 1 (%d) is not closed", fd1);

  /* Now open the file and read it.  */
  fd1 = open (name, O_RDONLY);
  if (fd1 == -1)
    error (EXIT_FAILURE, errno,
	   "cannot open first file \"%s\" for verification", name);

  if (read (fd1, buf, sizeof buf) != strlen (fd1string))
    error (EXIT_FAILURE, errno, "cannot read file 1");
  if (memcmp (fd1string, buf, strlen (fd1string)) != 0)
    error (EXIT_FAILURE, 0, "file 1 does not match");

  return 0;
}


int
do_test (int argc, char *argv[])
{
  pid_t pid;
  int fd1;
  int fd2;
  int flags;
  int status;

  /* We must have
     - four parameters left of called initially
       + path for ld.so
       + "--library-path"
       + the library path
       + the application name
     - three parameters left if called through re-execution
       + file descriptor number which is supposed to be closed
       + the open file descriptor
       + the name of the closed desriptor
  */

  if (restart)
    {
      if (argc != 4)
	error (EXIT_FAILURE, 0, "wrong number of arguments (%d)", argc);

      return handle_restart (argv[1], argv[2], argv[3]);
    }

  if (argc != 5)
    error (EXIT_FAILURE, 0, "wrong number of arguments (%d)", argc);

  /* Prepare the test.  We are creating two files: one which file descriptor
     will be marked with FD_CLOEXEC, another which is not.  */

   /* Open our test files.   */
   fd1 = mkstemp (name1);
   if (fd1 == -1)
     error (EXIT_FAILURE, errno, "cannot open test file `%s'", name1);
   fd2 = mkstemp (name2);
   if (fd2 == -1)
     error (EXIT_FAILURE, errno, "cannot open test file `%s'", name2);

   /* Set the bit.  */
   flags = fcntl (fd1, F_GETFD, 0);
   if (flags < 0)
     error (EXIT_FAILURE, errno, "cannot get flags");
   flags |= FD_CLOEXEC;
   if (fcntl (fd1, F_SETFD, flags) < 0)
     error (EXIT_FAILURE, errno, "cannot set flags");

   /* Write something in the files.  */
   if (write (fd1, fd1string, strlen (fd1string)) != strlen (fd1string))
     error (EXIT_FAILURE, errno, "cannot write to first file");
   if (write (fd2, fd2string, strlen (fd2string)) != strlen (fd2string))
     error (EXIT_FAILURE, errno, "cannot write to second file");

  /* We want to test the `exec' function.  To do this we restart the program
     with an additional parameter.  But first create another process.  */
  pid = fork ();
  if (pid == 0)
    {
      char fd1name[18];
      char fd2name[18];

      snprintf (fd1name, sizeof fd1name, "%d", fd1);
      snprintf (fd2name, sizeof fd2name, "%d", fd2);

      /* This is the child.  Construct the command line.  */
      execl (argv[1], argv[1], argv[2], argv[3], argv[4], "--direct",
	     "--restart", fd1name, fd2name, name1, NULL);

      error (EXIT_FAILURE, errno, "cannot exec");
    }
  else if (pid == (pid_t) -1)
    error (EXIT_FAILURE, errno, "cannot fork");

  /* Wait for the child.  */
  if (waitpid (pid, &status, 0) != pid)
    error (EXIT_FAILURE, errno, "wrong child");

  if (WTERMSIG (status) != 0)
    error (EXIT_FAILURE, 0, "Child terminated incorrectly");
  status = WEXITSTATUS (status);

  /* Remove the test files.  */
  unlink (name1);
  unlink (name2);

  return status;
}
