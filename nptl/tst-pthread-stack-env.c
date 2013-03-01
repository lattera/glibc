/* Verify that pthreads uses the default thread stack size set with the
   GLIBC_PTHREAD_DEFAULT_STACKSIZE environment variable.
   Copyright (C) 2013 Free Software Foundation, Inc.
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

#include <pthread.h>
#include <stdio.h>
#include <string.h>

/* It is possible that the default stack size somehow ends up being 1MB, thus
   giving a false positive.  The ideal way to test this would be to get the
   current stacksize fork a process with the default stack size set to
   something different to the current stack size and verify in the child
   process that the environment variable worked.  */
#define STACKSIZE 1024 * 1024L

void *
thr (void *u)
{
  size_t stacksize, guardsize;
  pthread_attr_t attr;
  pthread_getattr_np (pthread_self (), &attr);

  pthread_attr_getstacksize (&attr, &stacksize);
  pthread_attr_getguardsize (&attr, &guardsize);

  /* FIXME once guardsize is excluded from stacksize.  */
  if (stacksize - guardsize != STACKSIZE)
    {
      printf ("Stack size is %zu, should be %zu\n", stacksize - guardsize,
	      STACKSIZE);
      return (void *) 1;
    }

  return NULL;
}

int
do_test (int argc, char **argv)
{
  pthread_t t;
  void *thr_ret;
  int ret;

  if ((ret = pthread_create (&t, NULL, thr, NULL)) != 0)
    {
      printf ("thread create failed: %s\n", strerror (ret));
      return 1;
    }

  if ((ret = pthread_join (t, &thr_ret)) != 0)
    {
      printf ("join failed: %s\n", strerror (ret));
      return 1;
    }

  if (thr_ret != NULL && thr_ret != PTHREAD_CANCELED)
    return 1;

  return 0;
}

#include "../test-skeleton.c"
