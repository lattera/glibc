/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int
do_test (void)
{
  int i;
  pthread_attr_t a;

  if (pthread_attr_init (&a) != 0)
    {
      puts ("attr_init failed");
      exit (1);
    }

  for (i = 0; i < 10000; ++i)
    {
      long int r = random ();

      if (r != PTHREAD_CREATE_DETACHED && r != PTHREAD_CREATE_JOINABLE)
	{
	  int e = pthread_attr_setdetachstate (&a, r);

	  if (e == 0)
	    {
	      printf ("attr_setdetachstate with value %ld succeeded\n", r);
	      exit (1);
	    }

	  int s;
	  if (pthread_attr_getdetachstate (&a, &s) != 0)
	    {
	      puts ("attr_getdetachstate failed");
	      exit (1);
	    }

	  if (s != PTHREAD_CREATE_JOINABLE)
	    {
	      printf ("\
detach state changed to %d by invalid setdetachstate call\n", s);
	      exit (1);
	    }
	}
    }

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
