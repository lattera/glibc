/* Test for notification mechanism in lio_listio.
   Copyright (C) 2000,02 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <aio.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int flag;


static void
thrfct (sigval_t arg)
{
  flag = 1;
}


static int
do_test (int argc, char *argv[])
{
  char name[] = "/tmp/aio3.XXXXXX";
  int fd;
  struct aiocb *arr[1];
  struct aiocb cb;
  static const char buf[] = "Hello World\n";

  fd = mkstemp (name);
  if (fd == -1)
    {
      printf ("cannot open temp name: %m\n");
      return 1;
    }

  unlink (name);

  arr[0] = &cb;

  cb.aio_fildes = fd;
  cb.aio_lio_opcode = LIO_WRITE;
  cb.aio_reqprio = 0;
  cb.aio_buf = (void *) buf;
  cb.aio_nbytes = sizeof (buf) - 1;
  cb.aio_offset = 0;
  cb.aio_sigevent.sigev_notify = SIGEV_THREAD;
  cb.aio_sigevent.sigev_notify_function = thrfct;
  cb.aio_sigevent.sigev_notify_attributes = NULL;
  cb.aio_sigevent.sigev_value.sival_ptr = NULL;

  if (lio_listio (LIO_NOWAIT, arr, 1, NULL) < 0)
    {
      if (errno == ENOSYS)
	{
	  puts ("no aio support in this configuration");
	  return 0;
	}
      printf ("lio_listio failed: %m\n");
      return 1;
    }

  if (aio_suspend ((const struct aiocb *const *) arr, 1, NULL) < 0)
    {
      printf ("aio_suspend failed: %m\n");
      return 1;
    }

  if (flag != 0)
    {
      puts ("thread created, should not have happened");
      return 1;
    }

  puts ("all OK");

  return 0;
}

#include "../test-skeleton.c"
