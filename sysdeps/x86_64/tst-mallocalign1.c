/* Copyright (C) 2012-2015 Free Software Foundation, Inc.
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

/* Specified by x86-64 psABI.  */
#define ALIGN_MASK (16 - 1)

void *
test (size_t s)
{
  void *p = malloc (s);

  printf ("malloc: %ld, %p: %ld\n", (unsigned long) s, p,
	  ((unsigned long) p) & ALIGN_MASK);
  return p;
}

static int
do_test (void)
{
  void *p;
  int ret = 0;

  p = test (2);
  ret |= (unsigned long) p & ALIGN_MASK;
  free (p);

  p = test (8);
  ret |= (unsigned long) p & ALIGN_MASK;
  free (p);

  p = test (13);
  ret |= (unsigned long) p & ALIGN_MASK;
  free (p);

  p = test (16);
  ret |= (unsigned long) p & ALIGN_MASK;
  free (p);

  p = test (23);
  ret |= (unsigned long) p & ALIGN_MASK;
  free (p);

  p = test (43);
  ret |= (unsigned long) p & ALIGN_MASK;
  free (p);

  p = test (123);
  ret |= (unsigned long) p & ALIGN_MASK;
  free (p);

  return ret;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
