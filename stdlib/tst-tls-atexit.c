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

/* This test dynamically loads a DSO and spawns a thread that subsequently
   calls into the DSO to register a destructor for an object in the DSO and
   then calls dlclose on the handle for the DSO.  When the thread exits, the
   DSO should not be unloaded or else the destructor called during thread exit
   will crash.  Further in the main thread, the DSO is opened and closed again,
   at which point the DSO should be unloaded.  */

#include <dlfcn.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <link.h>

#define DSO_NAME "$ORIGIN/tst-tls-atexit-lib.so"

/* Walk through the map in the _r_debug structure to see if our lib is still
   loaded.  */
static bool
is_loaded (void)
{
  struct link_map *lm = (struct link_map *) _r_debug.r_map;

  for (; lm; lm = lm->l_next)
    if (lm->l_type == lt_loaded && lm->l_name
	&& strcmp (basename (DSO_NAME), basename (lm->l_name)) == 0)
	  return true;
  return false;
}

/* Accept a valid handle returned by DLOPEN, load the reg_dtor symbol to
   register a destructor and then call dlclose on the handle.  The dlclose
   should not unload the DSO since the destructor has not been called yet.  */
static void *
reg_dtor_and_close (void *h)
{
  void (*reg_dtor) (void) = (void (*) (void)) dlsym (h, "reg_dtor");

  if (reg_dtor == NULL)
    {
      printf ("Unable to find symbol: %s\n", dlerror ());
      return (void *) (uintptr_t) 1;
    }

  reg_dtor ();

  dlclose (h);

  return NULL;
}

static int
spawn_thread (void *h)
{
  pthread_t t;
  int ret;
  void *thr_ret;

  if ((ret = pthread_create (&t, NULL, reg_dtor_and_close, h)) != 0)
    {
      printf ("pthread_create failed: %s\n", strerror (ret));
      return 1;
    }

  if ((ret = pthread_join (t, &thr_ret)) != 0)
    {
      printf ("pthread_join failed: %s\n", strerror (ret));
      return 1;
    }

  if (thr_ret != NULL)
    return 1;

  return 0;
}

static int
do_test (void)
{
  /* Load the DSO.  */
  void *h1 = dlopen (DSO_NAME, RTLD_LAZY);
  if (h1 == NULL)
    {
      printf ("h1: Unable to load DSO: %s\n", dlerror ());
      return 1;
    }

  if (spawn_thread (h1) != 0)
    return 1;

  /* Now this should unload the DSO.  FIXME: This is a bug, calling dlclose
     like this is actually wrong, but it works because cxa_thread_atexit_impl
     has a bug which results in dlclose allowing this to work.  */
  dlclose (h1);

  /* Check link maps to ensure that the DSO has unloaded.  */
  if (is_loaded ())
    return 1;

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
