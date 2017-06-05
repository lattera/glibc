/* Common function for preadv2 and pwritev2 tests.
   Copyright (C) 2017 Free Software Foundation, Inc.
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

#include <support/check.h>

static void
do_test_with_invalid_flags (void)
{
  int invalid_flag = 0x1;
#ifdef RWF_HIPRI
  invalid_flag <<= 1;
#endif
#ifdef RWF_DSYNC
  invalid_flag <<= 1;
#endif
#ifdef RWF_SYNC
  invalid_flag <<= 1;
#endif

  char buf[32];
  const struct iovec vec = { .iov_base = buf, .iov_len = sizeof (buf) };
  if (preadv2 (temp_fd, &vec, 1, 0, invalid_flag) != -1)
    FAIL_EXIT1 ("preadv2 did not fail with an invalid flag");
  if (errno != ENOTSUP)
    FAIL_EXIT1 ("preadv2 failure did not set errno to ENOTSUP (%d)", errno);

  /* This might fail for compat syscall (32 bits running on 64 bits kernel)
     due a kernel issue.  */
  if (pwritev2 (temp_fd, &vec, 1, 0, invalid_flag) != -1)
    FAIL_EXIT1 ("pwritev2 did not fail with an invalid flag");
  if (errno != ENOTSUP)
    FAIL_EXIT1 ("pwritev2 failure did not set errno to ENOTSUP (%d)", errno);
}
