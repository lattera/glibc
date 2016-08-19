/* Common posix_fallocate tests definitions.
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

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

static void do_prepare (void);
#define PREPARE(argc, argv)     do_prepare ()
static int do_test (void);
#define TEST_FUNCTION           do_test ()

#define TIMEOUT 20 /* sec.  */

#include <test-skeleton.c>

static char *temp_filename;
static int temp_fd;

static void
do_prepare (void)
{
  temp_fd = create_temp_file ("tst-posix_fallocate.", &temp_filename);
  if (temp_fd == -1)
    FAIL_EXIT1 ("cannot create temporary file: %m\n");
}

#define FAIL(str) \
  do { printf ("error: %s (line %d)\n", str, __LINE__); return 1; } while (0)

static int
do_test_with_offset (off_t offset)
{
  struct stat st;

  if (posix_fallocate (temp_fd, offset, 768) != 0)
    FAIL_EXIT1 ("1st posix_fallocate call failed");

  if (fstat (temp_fd, &st) != 0)
    FAIL_EXIT1 ("2nd fstat failed");

  if (st.st_size != (offset + 768))
    FAIL_EXIT1 ("file size after first posix_fallocate call is %lu, "
		"expected %lu",
		(unsigned long int) st.st_size, 512lu + 768lu);

  if (posix_fallocate (temp_fd, 0, 1024) != 0)
    FAIL_EXIT1 ("2nd posix_fallocate call failed");

  if (fstat (temp_fd, &st) != 0)
    FAIL_EXIT1 ("3rd fstat failed");

  if (st.st_size != (offset) + 768)
    FAIL_EXIT1 ("file size changed in second posix_fallocate");

  offset += 2048;
  if (posix_fallocate (temp_fd, offset, 64) != 0)
    FAIL_EXIT1 ("3rd posix_fallocate call failed");

  if (fstat (temp_fd, &st) != 0)
    FAIL_EXIT1 ("4th fstat failed");

  if (st.st_size != (offset + 64))
    FAIL_EXIT1 ("file size after first posix_fallocate call is %llu, "
		"expected %u",
		(unsigned long long int) st.st_size, 2048u + 64u);

  return 0;
}
