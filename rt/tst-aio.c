/* Tests for AIO in librt.
   Copyright (C) 1998 Free Software Foundation, Inc.
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

#include <aio.h>
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>


/* prototype for our test function.  */
extern int do_test (int argc, char *argv[]);


/* We might need a bit longer timeout.  */
#define TIMEOUT 20 /* sec */

/* This defines the `main' function and some more.  */
#include <test-skeleton.c>


int
test_file (const void *buf, size_t size, int fd, const char *msg)
{
  struct stat st;
  char tmp[size];

  errno = 0;
  if (fstat (fd, &st) < 0)
    {
      error (0, errno, "%s: failed stat", msg);
      return 1;
    }

  if (st.st_size != size)
    {
      error (0, errno, "%s: wrong size: %lu, should be %lu",
	     msg, (unsigned long int) st.st_size, (unsigned long int) size);
      return 1;
    }

  if (pread (fd, tmp, size, 0) != size)
    {
      error (0, errno, "%s: failed stat", msg);
      return 1;
    }

  if (memcmp (buf, tmp, size) != 0)
    {
      error (0, errno, "%s: failed comparison", msg);
      return 1;
    }

  if (ftruncate (fd, 0) < 0)
    {
      error (0, errno, "%s: failed truncate", msg);
      return 1;
    }

  return 0;
}


int
do_test (int argc, char *argv[])
{
  char *name;
  char name_len;
  struct aiocb cbs[10];
  struct aiocb *cbp[10];
  char buf[1000];
  size_t cnt;
  int fd;
  int result = 0;
  int go_on;

  name_len = strlen (test_dir);
  name = malloc (name_len + sizeof ("/aioXXXXXX"));
  mempcpy (mempcpy (name, test_dir, name_len),
	   "/aioXXXXXX", sizeof ("/aioXXXXXX"));
  add_temp_file (name);

  /* Open our test file.   */
  fd = mkstemp (name);
  if (fd == -1)
    error (EXIT_FAILURE, errno, "cannot open test file `%s'", name);

  /* Preparation.  */
  for (cnt = 0; cnt < 10; ++cnt)
    {
      cbs[cnt].aio_fildes = fd;
      cbs[cnt].aio_reqprio = 0;
      cbs[cnt].aio_buf = memset (&buf[cnt * 100], '0' + cnt, 100);
      cbs[cnt].aio_nbytes = 100;
      cbs[cnt].aio_offset = cnt * 100;
      cbs[cnt].aio_sigevent.sigev_notify = SIGEV_NONE;

      cbp[cnt] = &cbs[cnt];
    }

  /* First a simple test.  */
  for (cnt = 10; cnt > 0; )
    aio_write (cbp[--cnt]);
  /* Wait 'til the results are there.  */
  do
    {
      aio_suspend ((const struct aiocb *const *) cbp, 10, NULL);
      go_on = 0;
      for (cnt = 0; cnt < 10; ++cnt)
	if (cbp[cnt] != NULL && aio_error (cbp[cnt]) == EINPROGRESS)
	  go_on = 1;
	else
	  {
	    if (cbp[cnt] != NULL)
	      printf ("request %d finished\n", cnt);
	    cbp[cnt] = NULL;
	  }
    }
  while (go_on);
  /* Test this.  */
  result |= test_file (buf, sizeof (buf), fd, "aio_write");

  return result;
}
