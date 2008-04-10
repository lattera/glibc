/* Copyright (C) 2006, 2007, 2008 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <ucontext.h>

ucontext_t ucp;
char st1[8192];
__thread int thr;

int somevar = -76;
long othervar = -78L;

void
cf (int i)
{
  if (i != othervar || thr != 94)
    {
      printf ("i %d thr %d\n", i, thr);
      exit (1);
    }
  exit (0);
}

int
do_test (void)
{
  if (getcontext (&ucp) != 0)
    {
      if (errno == ENOSYS)
	{
	  puts ("context handling not supported");
	  return 0;
	}

      puts ("getcontext failed");
      return 1;
    }
  thr = 94;
  ucp.uc_link = NULL;
  ucp.uc_stack.ss_sp = st1;
  ucp.uc_stack.ss_size = sizeof st1;
  makecontext (&ucp, (void (*) (void)) cf, 1, somevar - 2);
  if (setcontext (&ucp) != 0)
    {
      puts ("setcontext failed");
      return 1;
    }
  return 2;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
