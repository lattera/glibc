/* Verify that backtrace does not deadlock on itself on memory corruption.
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


#include <stdlib.h>

#define SIZE 4096

/* Wrap free with a function to prevent gcc from optimizing it out.  */
static void
__attribute__((noinline))
call_free (void *ptr)
{
  free (ptr);
  *(size_t *)(ptr - sizeof (size_t)) = 1;
}

int do_test (void);

#define TEST_FUNCTION do_test ()
#define EXPECTED_SIGNAL SIGABRT

#include "../test-skeleton.c"

int
do_test (void)
{
  void *ptr1 = malloc (SIZE);
  void *ptr2 = malloc (SIZE);

  /* Avoid unwanted output to TTY after an expected memory corruption.  */
  ignore_stderr();

  call_free (ptr1);
  ptr1 = malloc (SIZE);

  /* Not reached.  The return statement is to put ptr2 into use so that gcc
     doesn't optimize out that malloc call.  */
  return (ptr1 == ptr2);
}
