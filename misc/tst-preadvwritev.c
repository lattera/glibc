/* Tests for preadv and pwritev.
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

#include <sys/uio.h>

/* Allow testing of the 64-bit versions as well.  */
#ifndef PREADV
# define PREADV  preadv
# define PWRITEV pwritev
#endif

static void do_prepare (void);
static int do_test (void);
#define PREPARE(argc, argv)     do_prepare ()
#define TEST_FUNCTION           do_test ()
#include "../test-skeleton.c"

static char *temp_filename;
static int temp_fd;

void
do_prepare (void)
{
  temp_fd = create_temp_file ("tst-PREADVwritev.", &temp_filename);
  if (temp_fd == -1)
    {
      printf ("cannot create temporary file: %m\n");
      exit (1);
    }
}

#define FAIL(str) \
  do { printf ("error: %s (line %d)\n", str, __LINE__); return 1; } while (0)

int
do_test (void)
{
  struct iovec iov[2];
  ssize_t ret;

  char buf1[32];
  char buf2[64];

  memset (buf1, 0xf0, sizeof buf1);
  memset (buf2, 0x0f, sizeof buf2);

  memset (iov, 0, sizeof iov);
  iov[0].iov_base = buf1;
  iov[0].iov_len = sizeof buf1;
  iov[1].iov_base = buf2;
  iov[1].iov_len = sizeof buf2;

  ret = PWRITEV (temp_fd, iov, 2, 0);
  if (ret == -1)
    FAIL ("first PWRITEV returned -1");
  if (ret != (sizeof buf1 + sizeof buf2))
    FAIL ("first PWRITEV returned an unexpected value");

  ret = PWRITEV (temp_fd, iov, 2, sizeof buf1 + sizeof buf2);
  if (ret == -1)
    FAIL ("second PWRITEV returned -1");
  if (ret != (sizeof buf1 + sizeof buf2))
    FAIL ("second PWRITEV returned an unexpected value");

  char buf3[32];
  char buf4[64];

  memset (buf3, 0x0f, sizeof buf3);
  memset (buf4, 0xf0, sizeof buf4);

  iov[0].iov_base = buf3;
  iov[0].iov_len = sizeof buf3;
  iov[1].iov_base = buf4;
  iov[1].iov_len = sizeof buf4;

  ret = PREADV (temp_fd, iov, 2, 0);
  if (ret == -1)
    FAIL ("first PREADV returned -1");
  if (ret != (sizeof buf3 + sizeof buf4))
    FAIL ("first PREADV returned an unexpected value");

  if (memcmp (buf1, buf3, sizeof buf1) != 0)
    FAIL ("first buffer from first PREADV different than expected");
  if (memcmp (buf2, buf4, sizeof buf2) != 0)
    FAIL ("second buffer from first PREADV different than expected");

  ret = PREADV (temp_fd, iov, 2, sizeof buf3 + sizeof buf4);
  if (ret == -1)
    FAIL ("second PREADV returned -1");
  if (ret != (sizeof buf3 + sizeof buf4))
    FAIL ("second PREADV returned an unexpected value");

  if (memcmp (buf1, buf3, sizeof buf1) != 0)
    FAIL ("first buffer from second PREADV different than expected");
  if (memcmp (buf2, buf4, sizeof buf2) != 0)
    FAIL ("second buffer from second PREADV different than expected");

  return 0;
}
