/* Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static int errors = 0;

static void
merror (const char *msg)
{
  ++errors;
  printf ("Error: %s\n", msg);
}

static int
do_test (void)
{
  void *p;
  unsigned long pagesize = getpagesize();
  unsigned long ptrval;
  int save;

  errno = 0;

  p = pvalloc (-1);

  save = errno;

  if (p != NULL)
    merror ("pvalloc (-1) succeeded.");

  if (p == NULL && save != ENOMEM)
    merror ("pvalloc (-1) errno is not set correctly");

  errno = 0;

  p = pvalloc (-pagesize);

  save = errno;

  if (p != NULL)
    merror ("pvalloc (-pagesize) succeeded.");

  if (p == NULL && save != ENOMEM)
    merror ("pvalloc (-pagesize) errno is not set correctly");

  p = pvalloc (0);

  if (p == NULL)
    merror ("pvalloc (0) failed.");

  free (p);

  p = pvalloc (32);

  if (p == NULL)
    merror ("pvalloc (32) failed.");

  ptrval = (unsigned long)p;

  if (p == NULL)
    merror ("pvalloc (32) failed.");

  if ((ptrval & (pagesize - 1)) != 0)
    merror ("returned pointer is not page aligned.");

  free (p);

  return errors != 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
