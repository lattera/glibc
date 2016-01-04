/* Copyright (C) 1999-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@arthur.rhein-neckar.de>, 1999.

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
  void *p, *q;
  int save;

  errno = 0;

  p = malloc (-1);
  save = errno;

  if (p != NULL)
    merror ("malloc (-1) succeeded.");

  if (p == NULL && save != ENOMEM)
    merror ("errno is not set correctly");

  p = malloc (10);
  if (p == NULL)
    merror ("malloc (10) failed.");

  /* realloc (p, 0) == free (p).  */
  p = realloc (p, 0);
  if (p != NULL)
    merror ("realloc (p, 0) failed.");

  p = malloc (0);
  if (p == NULL)
    merror ("malloc (0) failed.");

  p = realloc (p, 0);
  if (p != NULL)
    merror ("realloc (p, 0) failed.");

  p = malloc (513 * 1024);
  if (p == NULL)
    merror ("malloc (513K) failed.");

  q = malloc (-512 * 1024);
  if (q != NULL)
    merror ("malloc (-512K) succeeded.");

  free (p);

  return errors != 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
