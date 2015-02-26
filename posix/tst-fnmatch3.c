/* Test for fnmatch not reading past the end of the pattern.
   Copyright (C) 2014-2015 Free Software Foundation, Inc.
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

#include <fnmatch.h>

int
do_test (void)
{
  if (fnmatch ("[[:alpha:]'[:alpha:]\0]", "a", 0) != FNM_NOMATCH)
    return 1;
  if (fnmatch ("[a[.\0.]]", "a", 0) != FNM_NOMATCH)
    return 1;
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
