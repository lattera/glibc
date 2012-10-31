/* Test program for POSIX shm_* functions.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>


/* We want to see output immediately.  */
#define STDOUT_UNBUFFERED


static int
do_open (void)
{
  int fd;

  /* Create the shared memory object.  */
  fd = shm_open ("/shm-test", O_RDWR, 0600);
  if (fd == -1)
    {
      /* We don't regard this as a bug.  Simply don't run the test.  It could
	 means there is no such implementation or the object is already in
	 use in which case we don't want to disturb.  */
      perror ("failed to open shared memory object: shm_open");
      return -1;
    }

  return fd;
}


static void
worker (int write_now)
{
  struct timespec ts;
  struct stat64 st;
  int i;
  int fd = do_open ();
  char *mem;

  if (fd == -1)
    exit (fd);

  if (fstat64 (fd, &st) == -1)
    error (EXIT_FAILURE, 0, "stat failed");
  if (st.st_size != 4000)
    error (EXIT_FAILURE, 0, "size incorrect");

  mem = mmap (NULL, 4000, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mem == MAP_FAILED)
    error (EXIT_FAILURE, 0, "mmap failed");

  ts.tv_sec = 0;
  ts.tv_nsec = 500000000;

  if (write_now)
    for (i = 0; i <= 4; ++i)
      mem[i] = i;
  else
    /* Wait until the first bytes of the memory region are 0, 1, 2, 3, 4.  */
    while (1)
      {
	for (i = 0; i <= 4; ++i)
	  if (mem[i] != i)
	    break;

	if (i > 4)
	  /* OK, that's done.  */
	  break;

	nanosleep (&ts, NULL);
      }

  if (!write_now)
    for (i = 0; i <= 4; ++i)
      mem[i] = 4 + i;
  else
    /* Wait until the first bytes of the memory region are 4, 5, 6, 7, 8.  */
    while (1)
      {
	for (i = 0; i <= 4; ++i)
	  if (mem[i] != 4 + i)
	    break;

	if (i > 4)
	  /* OK, that's done.  */
	  break;

	nanosleep (&ts, NULL);
      }

  if (munmap (mem, 4000) == -1)
    error (EXIT_FAILURE, errno, "munmap");

  close (fd);

  exit (0);
}


static int
do_test (void)
{
  int fd;
  pid_t pid1;
  pid_t pid2;
  int status1;
  int status2;
  struct stat64 st;

  /* Create the shared memory object.  */
  fd = shm_open ("/shm-test", O_RDWR | O_CREAT | O_TRUNC | O_EXCL, 0600);
  if (fd == -1)
    {
      /* We don't regard this as a bug.  Simply don't run the test.  It could
	 means there is no such implementation or the object is already in
	 use in which case we don't want to disturb.  */
      perror ("failed to create a shared memory object: shm_open");
      return 0;
    }

  /* Size the object.  We make it 4000 bytes long.  */
  if (ftruncate (fd, 4000) == -1)
    {
      /* This failed.  Must be a bug in the implementation of the
         shared memory itself.  */
      perror ("failed to size of shared memory object: ftruncate");
      close (fd);
      shm_unlink ("/shm-test");
      return 0;
    }

  if (fstat64 (fd, &st) == -1)
    {
      shm_unlink ("/shm-test");
      error (EXIT_FAILURE, 0, "initial stat failed");
    }
  if (st.st_size != 4000)
    {
      shm_unlink ("/shm-test");
      error (EXIT_FAILURE, 0, "initial size not correct");
    }

  /* Spawn to processes which will do the work.  */
  pid1 = fork ();
  if (pid1 == 0)
    worker (0);
  else if (pid1 == -1)
    {
      /* Couldn't create a second process.  */
      perror ("fork");
      close (fd);
      shm_unlink ("/shm-test");
      return 0;
    }

  pid2 = fork ();
  if (pid2 == 0)
    worker (1);
  else if (pid2 == -1)
    {
      /* Couldn't create a second process.  */
      int ignore;
      perror ("fork");
      kill (pid1, SIGTERM);
      waitpid (pid1, &ignore, 0);
      close (fd);
      shm_unlink ("/shm-test");
      return 0;
    }

  /* Wait until the two processes are finished.  */
  waitpid (pid1, &status1, 0);
  waitpid (pid2, &status2, 0);

  /* Now we can unlink the shared object.  */
  shm_unlink ("/shm-test");

  return (!WIFEXITED (status1) || WEXITSTATUS (status1) != 0
	  || !WIFEXITED (status2) || WEXITSTATUS (status2) != 0);
}
#define TEST_FUNCTION do_test ()

#define CLEANUP_HANDLER shm_unlink ("/shm-test");


#include "../test-skeleton.c"
