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
  pthread_attr_t a;

  if (pthread_attr_init (&a) != 0)
    {
      puts ("attr_init failed");
      exit (1);
    }

  /* Check default value of detach state.  */
  int s;
  if (pthread_attr_getdetachstate (&a, &s) != 0)
    {
      puts ("1st attr_getdestachstate failed");
      exit (1);
    }
  if (s != PTHREAD_CREATE_JOINABLE)
    {
      printf ("\
default detach state wrong: %d, expected %d (PTHREAD_CREATE_JOINABLE)\n",
	      s, PTHREAD_CREATE_JOINABLE);
      exit (1);
    }

  if (pthread_attr_setdetachstate (&a, PTHREAD_CREATE_DETACHED) != 0)
    {
      puts ("1st attr_setdetachstate failed");
      exit (1);
    }
  if (pthread_attr_getdetachstate (&a, &s) != 0)
    {
      puts ("2nd attr_getdestachstate failed");
      exit (1);
    }
  if (s != PTHREAD_CREATE_DETACHED)
    {
      puts ("PTHREAD_CREATE_DETACHED set, but not given back");
      exit (1);
    }

  if (pthread_attr_setdetachstate (&a, PTHREAD_CREATE_JOINABLE) != 0)
    {
      puts ("2nd attr_setdetachstate failed");
      exit (1);
    }
  if (pthread_attr_getdetachstate (&a, &s) != 0)
    {
      puts ("3rd attr_getdestachstate failed");
      exit (1);
    }
  if (s != PTHREAD_CREATE_JOINABLE)
    {
      puts ("PTHREAD_CREATE_JOINABLE set, but not given back");
      exit (1);
    }


  size_t g;
  if (pthread_attr_getguardsize (&a, &g) != 0)
    {
      puts ("1st attr_getguardsize failed");
      exit (1);
    }
  if (g != sysconf (_SC_PAGESIZE))
    {
      printf ("default guardsize %zu, expected %ld (PAGESIZE)\n",
	      g, sysconf (_SC_PAGESIZE));
      exit (1);
    }

  if (pthread_attr_setguardsize (&a, 0) != 0)
    {
      puts ("1st attr_setguardsize failed");
      exit (1);
    }
  if (pthread_attr_getguardsize (&a, &g) != 0)
    {
      puts ("2nd attr_getguardsize failed");
      exit (1);
    }
  if (g != 0)
    {
      printf ("guardsize set to zero but %zu returned\n", g);
      exit (1);
    }

  if (pthread_attr_setguardsize (&a, 1) != 0)
    {
      puts ("2nd attr_setguardsize failed");
      exit (1);
    }
  if (pthread_attr_getguardsize (&a, &g) != 0)
    {
      puts ("3rd attr_getguardsize failed");
      exit (1);
    }
  if (g != 1)
    {
      printf ("guardsize set to 1 but %zu returned\n", g);
      exit (1);
    }

  return 0;
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
