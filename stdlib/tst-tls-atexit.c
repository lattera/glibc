/* Verify that DSO is unloaded only if its TLS objects are destroyed.
   Copyright (C) 2013-2015 Free Software Foundation, Inc.
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

/* There are two tests in this test case.  The first is implicit where it is
   assumed that the destructor call on exit of the LOAD function does not
   segfault.  The other is a verification that after the thread has exited, a
   dlclose will unload the DSO.  */

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void *handle;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void *
load (void *u)
{
  pthread_mutex_lock (&m);
  handle = dlopen ("$ORIGIN/tst-tls-atexit-lib.so", RTLD_LAZY);
  if (!handle)
    {
      printf ("Unable to load DSO: %s\n", dlerror ());
      return (void *) (uintptr_t) 1;
    }

  void (*foo) (void) = (void (*) (void)) dlsym(handle, "do_foo");

  if (!foo)
    {
      printf ("Unable to find symbol: %s\n", dlerror ());
      exit (1);
    }

  foo ();

  /* This should not unload the DSO.  If it does, then the thread exit will
     result in a segfault.  */
  dlclose (handle);
  pthread_mutex_unlock (&m);

  return NULL;
}

static int
do_test (void)
{
  pthread_t t;
  int ret;
  void *thr_ret;

  if ((ret = pthread_create (&t, NULL, load, NULL)) != 0)
    {
      printf ("pthread_create failed: %s\n", strerror (ret));
      return 1;
    }

  if ((ret = pthread_join (t, &thr_ret)) != 0)
    {
      printf ("pthread_create failed: %s\n", strerror (ret));
      return 1;
    }

  if (thr_ret != NULL)
    return 1;

  /* Now this should unload the DSO.  */
  dlclose (handle);

  /* Run through our maps and ensure that the DSO is unloaded.  */
  FILE *f = fopen ("/proc/self/maps", "r");

  if (f == NULL)
    {
      perror ("Failed to open /proc/self/maps");
      fprintf (stderr, "Skipping verification of DSO unload\n");
      return 0;
    }

  char *line = NULL;
  size_t s = 0;
  while (getline (&line, &s, f) > 0)
    {
      if (strstr (line, "tst-tls-atexit-lib.so"))
        {
	  printf ("DSO not unloaded yet:\n%s", line);
	  return 1;
	}
    }
  free (line);

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
