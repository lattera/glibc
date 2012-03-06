/* This file is part of the GNU C Library.
   Copyright (C) 2012 Free Software Foundation, Inc.
   Contributed by Marek Polacek <polacek@redhat.com>, 2012.

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

#include <wchar.h>

/* Prototype for our test function.  */
static int do_test (void);

static int
do_test (void)
{
  const long double x = 24.5;
  wchar_t a[16 * sizeof (wchar_t)];
  swprintf (a, 16 * sizeof (wchar_t), L"%La\n", x);
  wchar_t A[16 * sizeof (wchar_t)];
  swprintf (A, 16 * sizeof (wchar_t), L"%LA\n", x);

  return (wmemcmp (a, L"0xc.4p+1", 8) != 0
	  || wmemcmp (A, L"0XC.4P+1", 8) != 0);
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
