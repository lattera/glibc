/* Tests for spawn.
   Copyright (C) 2000-2018 Free Software Foundation, Inc.
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
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <wait.h>
#include <sys/param.h>
#include <support/check.h>
#include <support/xunistd.h>


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
static char *name3;

/* Descriptors for the temporary files.  */
static int temp_fd1 = -1;
static int temp_fd2 = -1;
static int temp_fd3 = -1;

/* The contents of our files.  */
static const char fd1string[] = "This file should get closed";
static const char fd2string[] = "This file should stay opened";
static const char fd3string[] = "This file will be opened";


/* We have a preparation function.  */
void
do_prepare (int argc, char *argv[])
{
  /* We must not open any files in the restart case.  */
  if (restart)
    return;

  temp_fd1 = create_temp_file ("spawn", &name1);
  temp_fd2 = create_temp_file ("spawn", &name2);
  temp_fd3 = create_temp_file ("spawn", &name3);
  if (temp_fd1 < 0 || temp_fd2 < 0 || temp_fd3 < 0)
    exit (1);
}


static int
handle_restart (const char *fd1s, const char *fd2s, const char *fd3s,
		const char *fd4s, const char *name)
{
  char buf[100];
  int fd1;
  int fd2;
  int fd3;
  int fd4;

  /* First get the descriptors.  */
  fd1 = atol (fd1s);
  fd2 = atol (fd2s);
  fd3 = atol (fd3s);
  fd4 = atol (fd4s);

  /* Sanity check.  */
  if (fd1 == fd2)
    error (EXIT_FAILURE, 0, "value of fd1 and fd2 is the same");
  if (fd1 == fd3)
    error (EXIT_FAILURE, 0, "value of fd1 and fd3 is the same");
  if (fd1 == fd4)
    error (EXIT_FAILURE, 0, "value of fd1 and fd4 is the same");
  if (fd2 == fd3)
    error (EXIT_FAILURE, 0, "value of fd2 and fd3 is the same");
  if (fd2 == fd4)
    error (EXIT_FAILURE, 0, "value of fd2 and fd4 is the same");
  if (fd3 == fd4)
    error (EXIT_FAILURE, 0, "value of fd3 and fd4 is the same");

  /* First the easy part: read from the file descriptor which is
     supposed to be open.  */
  if (lseek (fd2, 0, SEEK_CUR) != strlen (fd2string))
    error (EXIT_FAILURE, errno, "file 2 not in right position");
  /* The duped descriptor must have the same position.  */
  if (lseek (fd4, 0, SEEK_CUR) != strlen (fd2string))
    error (EXIT_FAILURE, errno, "file 4 not in right position");
  if (lseek (fd2, 0, SEEK_SET) != 0)
    error (EXIT_FAILURE, 0, "cannot reset position in file 2");
  if (lseek (fd4, 0, SEEK_CUR) != 0)
    error (EXIT_FAILURE, errno, "file 4 not set back, too");
  if (read (fd2, buf, sizeof buf) != strlen (fd2string))
    error (EXIT_FAILURE, 0, "cannot read file 2");
  if (memcmp (fd2string, buf, strlen (fd2string)) != 0)
    error (EXIT_FAILURE, 0, "file 2 does not match");

  /* Now read from the third file.  */
  if (read (fd3, buf, sizeof buf) != strlen (fd3string))
    error (EXIT_FAILURE, 0, "cannot read file 3");
  if (memcmp (fd3string, buf, strlen (fd3string)) != 0)
    error (EXIT_FAILURE, 0, "file 3 does not match");
  /* Try to write to the file.  This should not be allowed.  */
  if (write (fd3, "boo!", 4) != -1 || errno != EBADF)
    error (EXIT_FAILURE, 0, "file 3 is writable");

  /* Now try to read the first file.  First make sure it is not opened.  */
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
  int fd4;
  int status;
  posix_spawn_file_actions_t actions;
  char fd1name[18];
  char fd2name[18];
  char fd3name[18];
  char fd4name[18];
  char *name3_copy;
  char *spargv[12];
  int i;

  /* We must have
     - one or four parameters left if called initially
       + path for ld.so		optional
       + "--library-path"	optional
       + the library path	optional
       + the application name
     - five parameters left if called through re-execution
       + file descriptor number which is supposed to be closed
       + the open file descriptor
       + the newly opened file descriptor
       + thhe duped second descriptor
       + the name of the closed descriptor
  */
  if (argc != (restart ? 6 : 2) && argc != (restart ? 6 : 5))
    error (EXIT_FAILURE, 0, "wrong number of arguments (%d)", argc);

  if (restart)
    return handle_restart (argv[1], argv[2], argv[3], argv[4], argv[5]);

  /* Prepare the test.  We are creating two files: one which file descriptor
     will be marked with FD_CLOEXEC, another which is not.  */

   /* Write something in the files.  */
   if (write (temp_fd1, fd1string, strlen (fd1string)) != strlen (fd1string))
     error (EXIT_FAILURE, errno, "cannot write to first file");
   if (write (temp_fd2, fd2string, strlen (fd2string)) != strlen (fd2string))
     error (EXIT_FAILURE, errno, "cannot write to second file");
   if (write (temp_fd3, fd3string, strlen (fd3string)) != strlen (fd3string))
     error (EXIT_FAILURE, errno, "cannot write to third file");

   /* Close the third file.  It'll be opened by `spawn'.  */
   close (temp_fd3);

   /* Tell `spawn' what to do.  */
   if (posix_spawn_file_actions_init (&actions) != 0)
     error (EXIT_FAILURE, errno, "posix_spawn_file_actions_init");
   /* Close `temp_fd1'.  */
   if (posix_spawn_file_actions_addclose (&actions, temp_fd1) != 0)
     error (EXIT_FAILURE, errno, "posix_spawn_file_actions_addclose");
   /* We want to open the third file.  */
   name3_copy = strdup (name3);
   if (name3_copy == NULL)
     error (EXIT_FAILURE, errno, "strdup");
   if (posix_spawn_file_actions_addopen (&actions, temp_fd3, name3_copy,
					 O_RDONLY, 0666) != 0)
     error (EXIT_FAILURE, errno, "posix_spawn_file_actions_addopen");
   /* Overwrite the name to check that a copy has been made.  */
   memset (name3_copy, 'X', strlen (name3_copy));

   /* We dup the second descriptor.  */
   fd4 = MAX (2, MAX (temp_fd1, MAX (temp_fd2, temp_fd3))) + 1;
   if (posix_spawn_file_actions_adddup2 (&actions, temp_fd2, fd4) != 0)
     error (EXIT_FAILURE, errno, "posix_spawn_file_actions_adddup2");

   /* Now spawn the process.  */
   snprintf (fd1name, sizeof fd1name, "%d", temp_fd1);
   snprintf (fd2name, sizeof fd2name, "%d", temp_fd2);
   snprintf (fd3name, sizeof fd3name, "%d", temp_fd3);
   snprintf (fd4name, sizeof fd4name, "%d", fd4);

   for (i = 0; i < (argc == (restart ? 6 : 5) ? 4 : 1); i++)
     spargv[i] = argv[i + 1];
   spargv[i++] = (char *) "--direct";
   spargv[i++] = (char *) "--restart";
   spargv[i++] = fd1name;
   spargv[i++] = fd2name;
   spargv[i++] = fd3name;
   spargv[i++] = fd4name;
   spargv[i++] = name1;
   spargv[i] = NULL;

   if (posix_spawn (&pid, argv[1], &actions, NULL, spargv, environ) != 0)
     error (EXIT_FAILURE, errno, "posix_spawn");

   /* Same test but with a NULL pid argument.  */
   if (posix_spawn (NULL, argv[1], &actions, NULL, spargv, environ) != 0)
     error (EXIT_FAILURE, errno, "posix_spawn");

   /* Cleanup.  */
   if (posix_spawn_file_actions_destroy (&actions) != 0)
     error (EXIT_FAILURE, errno, "posix_spawn_file_actions_destroy");
   free (name3_copy);

  /* Wait for the children.  */
  TEST_VERIFY (xwaitpid (pid, &status, 0) == pid);
  TEST_VERIFY (WIFEXITED (status));
  TEST_VERIFY (!WIFSIGNALED (status));
  TEST_VERIFY (WEXITSTATUS (status) == 0);

  xwaitpid (-1, &status, 0);
  TEST_VERIFY (WIFEXITED (status));
  TEST_VERIFY (!WIFSIGNALED (status));
  TEST_VERIFY (WEXITSTATUS (status) == 0);

  return 0;
}
