/* Tests for res_init in libresolv
   Copyright (C) 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#undef gethostbyname

#include <mcheck.h>
#include <netdb.h>
#include <resolv.h>

static int
do_test (void)
{
  mtrace ();
  for (int i = 0; i < 20; ++i)
    {
      res_init ();
      gethostbyname ("www.gnu.org");
    }
  return 0;
}

#define TEST_FUNCTION do_test ()
#define TIMEOUT 30
/* This defines the `main' function and some more.  */
#include <test-skeleton.c>
