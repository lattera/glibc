/* Test recursive dlopen using malloc hooks.
   Copyright (C) 2015 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <malloc.h>
#include <dlfcn.h>

#define DSO "moddummy1.so"
#define FUNC "dummy1"

#define DSO1 "moddummy2.so"
#define FUNC1 "dummy2"

/* Result of the called function.  */
int func_result;

/* Prototype for my hook.  */
void *custom_malloc_hook (size_t, const void *);

/* Pointer to old malloc hooks.  */
void *(*old_malloc_hook) (size_t, const void *);

/* Call function func_name in DSO dso_name via dlopen.  */
void
call_func (const char *dso_name, const char *func_name)
{
  int ret;
  void *dso;
  int (*func) (void);
  char *err;

  /* Open the DSO.  */
  dso = dlopen (dso_name, RTLD_NOW|RTLD_GLOBAL);
  if (dso == NULL)
    {
      err = dlerror ();
      fprintf (stderr, "%s\n", err);
      exit (1);
    }
  /* Clear any errors.  */
  dlerror ();

  /* Lookup func.  */
  *(void **) (&func) = dlsym (dso, func_name);
  if (func == NULL)
    {
      err = dlerror ();
      if (err != NULL)
        {
	  fprintf (stderr, "%s\n", err);
	  exit (1);
        }
    }
  /* Call func.  */
  func_result = (*func) ();

  /* Close the library and look for errors too.  */
  ret = dlclose (dso);
  if (ret != 0)
    {
      err = dlerror ();
      fprintf (stderr, "%s\n", err);
      exit (1);
    }

}

/* Empty hook that does nothing.  */
void *
custom_malloc_hook (size_t size, const void *caller)
{
  void *result;
  /* Restore old hooks.  */
  __malloc_hook = old_malloc_hook;
  /* First call a function in another library via dlopen.  */
  call_func (DSO1, FUNC1);
  /* Called recursively.  */
  result = malloc (size);
  /* Restore new hooks.  */
  __malloc_hook = custom_malloc_hook;
  return result;
}

static int
do_test (void)
{
  /* Save old hook.  */
  old_malloc_hook = __malloc_hook;
  /* Install new hook.  */
  __malloc_hook = custom_malloc_hook;

  /* Bug 17702 fixes two things:
       * A recursive dlopen unmapping the ld.so.cache.
       * An assertion that _r_debug is RT_CONSISTENT at entry to dlopen.
     We can only test the latter. Testing the former requires modifying
     ld.so.conf to cache the dummy libraries, then running ldconfig,
     then run the test. If you do all of that (and glibc's test
     infrastructure doesn't support that yet) then the test will
     SEGFAULT without the fix. If you don't do that, then the test
     will abort because of the assert described in detail below.  */
  call_func (DSO, FUNC);

  /* Restore old hook.  */
  __malloc_hook = old_malloc_hook;

  /* The function dummy2() is called by the malloc hook. Check to
     see that it was called. This ensures the second recursive
     dlopen happened and we called the function in that library.
     Before the fix you either get a SIGSEGV when accessing mmap'd
     ld.so.cache data or an assertion failure about _r_debug not
     beint RT_CONSISTENT.  We don't test for the SIGSEGV since it
     would require finding moddummy1 or moddummy2 in the cache and
     we don't have any infrastructure to test that, but the _r_debug
     assertion triggers.  */
  printf ("Returned result is %d\n", func_result);
  if (func_result <= 0)
    {
      printf ("FAIL: Function call_func() not called.\n");
      exit (1);
    }

  printf ("PASS: Function call_func() called more than once.\n");
  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
