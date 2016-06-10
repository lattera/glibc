/* Check sendmmsg cancellation.

   Copyright (C) 2016 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "tst-cancel4-common.h"

static void *
tf_sendmmsg (void *arg)
{
  if (arg == NULL)
    // XXX If somebody can provide a portable test case in which sendmmsg()
    // blocks we can enable this test to run in both rounds.
    abort ();

  struct sockaddr_un sun;

  tempfd = socket (AF_UNIX, SOCK_DGRAM, 0);
  if (tempfd == -1)
    {
      printf ("%s: first socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int tries = 0;
  do
    {
      if (++tries > 10)
	{
	  printf ("%s: too many unsuccessful bind calls\n", __FUNCTION__);
	}

      strcpy (sun.sun_path, "/tmp/tst-cancel4-socket-7-XXXXXX");
      if (mktemp (sun.sun_path) == NULL)
	{
	  printf ("%s: cannot generate temp file name\n", __FUNCTION__);
	  exit (1);
	}

      sun.sun_family = AF_UNIX;
    }
  while (bind (tempfd, (struct sockaddr *) &sun,
	       offsetof (struct sockaddr_un, sun_path)
	       + strlen (sun.sun_path) + 1) != 0);
  tempfname = strdup (sun.sun_path);

  tempfd2 = socket (AF_UNIX, SOCK_DGRAM, 0);
  if (tempfd2 == -1)
    {
      printf ("%s: second socket call failed\n", __FUNCTION__);
      exit (1);
    }

  int r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  r = pthread_barrier_wait (&b2);
  if (r != 0 && r != PTHREAD_BARRIER_SERIAL_THREAD)
    {
      printf ("%s: 2nd barrier_wait failed\n", __FUNCTION__);
      exit (1);
    }

  pthread_cleanup_push (cl, NULL);

  char mem[1];
  struct iovec iov[1];
  iov[0].iov_base = mem;
  iov[0].iov_len = 1;

  struct mmsghdr mm;
  mm.msg_hdr.msg_name = &sun;
  mm.msg_hdr.msg_namelen = (offsetof (struct sockaddr_un, sun_path)
			   + strlen (sun.sun_path) + 1);
  mm.msg_hdr.msg_iov = iov;
  mm.msg_hdr.msg_iovlen = 1;
  mm.msg_hdr.msg_control = NULL;
  mm.msg_hdr.msg_controllen = 0;

  ssize_t ret = sendmmsg (tempfd2, &mm, 1, 0);
  if (ret == -1 && errno == ENOSYS)
    exit (77);

  pthread_cleanup_pop (0);

  printf ("%s: sendmmsg returned\n", __FUNCTION__);

  exit (1);
}

struct cancel_tests tests[] =
{
  ADD_TEST (sendmmsg, 2, 1),
};
#define ntest_tf (sizeof (tests) / sizeof (tests[0]))

#include "tst-cancel4-common.c"
