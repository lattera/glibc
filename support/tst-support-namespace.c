/* Test entering namespaces.
   Copyright (C) 2016-2017 Free Software Foundation, Inc.
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
#include <support/namespace.h>

static int
do_test (void)
{
  if (support_become_root ())
    printf ("info: acquired root-like privileges\n");
  if (support_enter_network_namespace ())
    printf ("info: entered network namespace\n");
  if (support_in_uts_namespace ())
    printf ("info: also entered UTS namespace\n");
  return 0;
}

#include <support/test-driver.c>
