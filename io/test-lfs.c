/* Some basic tests for LFS.
   Copyright (C) 2000 Free Software Foundation, Inc.
   Contributed by Andreas Jaeger <aj@suse.de>, 2000.

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

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <errno.h>
#include <sys/resource.h>

/* Prototype for our test function.  */
extern void do_prepare (int argc, char *argv[]);
extern int do_test (int argc, char *argv[]);

/* We have a preparation function.  */
#define PREPARE do_prepare

/* We might need a bit longer timeout.  */
#define TIMEOUT 20 /* sec */

/* This defines the `main' function and some more.  */
#include <test-skeleton.c>

/* These are for the temporary file we generate.  */
char *name;
int fd;

/* 2^31 = 2GB.  */
#define TWO_GB 2147483648LL

void
do_prepare (int argc, char *argv[])
{
  char name_len;

  name_len = strlen (test_dir);
  name = malloc (name_len + sizeof ("/lfsXXXXXX"));
  mempcpy (mempcpy (name, test_dir, name_len),
           "/lfsXXXXXX", sizeof ("/lfsXXXXXX"));
  add_temp_file (name);

  /* Open our test file.   */
  if (mktemp (name) == NULL)
    error (EXIT_FAILURE, errno, "cannot create temporary file name");

  fd = open64 (name, O_CREAT|O_TRUNC|O_RDWR, 0666);
  if (fd == -1 && errno == ENOSYS)
    {
      /* Fail silently.  */
      error (0, errno, "open64 is not supported");
      exit (EXIT_SUCCESS);
    }

  if (fd == -1)
    error (EXIT_FAILURE, errno, "cannot open test file `%s'", name);

  if (setrlimit64 (RLIMIT_FSIZE, &((const struct rlimit64)
                                   { RLIM_INFINITY, RLIM_INFINITY }))
      == -1)
    error (EXIT_FAILURE, errno, "cannot reset file size limits");
}

int
do_test (int argc, char *argv[])
{
  int ret;
  struct stat64 statbuf;

  ret = lseek64 (fd, TWO_GB+100, SEEK_SET);
  if (ret == -1 && errno == ENOSYS)
    {
      error (0, errno, "lseek64 is not supported");
      exit (EXIT_SUCCESS);
    }

  ret = write (fd, "Hello", 5);
  if (ret == -1 && errno == EINVAL)
    {
      error (0, errno, "LFS seems not to be supported.");
      exit (EXIT_SUCCESS);
    }

  if (ret != 5)
    error (EXIT_FAILURE, errno, "cannot write test string to large file");

  ret = close (fd);

  if (ret == -1)
    error (EXIT_FAILURE, errno, "error closing file");

  ret = stat64 (name, &statbuf);

  if (ret == -1 && (errno == ENOSYS || errno == EOVERFLOW))
    error (0, errno, "stat64 is not supported");
  else if (ret == -1)
    error (EXIT_FAILURE, errno, "cannot stat file `%s'", name);
  else if (statbuf.st_size != (TWO_GB+100+5))
    error (EXIT_FAILURE, 0, "stat reported size %lld instead of %lld.",
	   statbuf.st_size, (TWO_GB+100+5));

  return 0;
}
