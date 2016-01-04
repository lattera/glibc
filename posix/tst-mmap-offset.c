/* BZ #18877 mmap offset test.

   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <sys/mman.h>

static int
printmsg (int rc, const char *msg)
{
  printf ("%s failed: %m\n", msg);
  return rc;
}

/* Check if negative offsets are handled correctly by mmap.  */
static int
do_test (void)
{
  const int prot = PROT_READ | PROT_WRITE;
  const int flags = MAP_SHARED;
  const unsigned long length = 0x10000;
  const unsigned long offset = 0xace00000;
  const unsigned long size = offset + length;
  void *addr;
  int fd;
  char fname[] = "tst-mmap-offset-XXXXXX";

  fd = mkstemp64 (fname);
  if (fd < 0)
    return printmsg (1, "mkstemp");

  if (unlink (fname))
    return printmsg (1, "unlink");

  if (ftruncate64 (fd, size))
    return printmsg (0, "ftruncate64");

  addr = mmap (NULL, length, prot, flags, fd, offset);
  if (MAP_FAILED == addr)
    return printmsg (1, "mmap");

  /* This memcpy is likely to SIGBUS if mmap has messed up with offset.  */
  memcpy (addr, fname, sizeof (fname));

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
