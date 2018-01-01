/* Copyright (C) 2017-2018 Free Software Foundation, Inc.
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

/* Verify that tunables correctly filter out unsafe tunables like
   glibc.malloc.check and glibc.malloc.mmap_threshold but also retain
   glibc.malloc.mmap_threshold in an unprivileged child.  */

/* This is compiled as part of the testsuite but needs to see
   HAVE_TUNABLES. */
#define _LIBC 1
#include "config.h"
#undef _LIBC

#define test_parent test_parent_tunables
#define test_child test_child_tunables

static int test_child_tunables (void);
static int test_parent_tunables (void);

#include "tst-env-setuid.c"

#define CHILD_VALSTRING_VALUE "glibc.malloc.mmap_threshold=4096"
#define PARENT_VALSTRING_VALUE \
  "glibc.malloc.check=2:glibc.malloc.mmap_threshold=4096"

static int
test_child_tunables (void)
{
  const char *val = getenv ("GLIBC_TUNABLES");

#if HAVE_TUNABLES
  if (val != NULL && strcmp (val, CHILD_VALSTRING_VALUE) == 0)
    return 0;

  if (val != NULL)
    printf ("Unexpected GLIBC_TUNABLES VALUE %s\n", val);

  return 1;
#else
  if (val != NULL)
    {
      printf ("GLIBC_TUNABLES not cleared\n");
      return 1;
    }
  return 0;
#endif
}

static int
test_parent_tunables (void)
{
  const char *val = getenv ("GLIBC_TUNABLES");

  if (val != NULL && strcmp (val, PARENT_VALSTRING_VALUE) == 0)
    return 0;

  if (val != NULL)
    printf ("Unexpected GLIBC_TUNABLES VALUE %s\n", val);

  return 1;
}
