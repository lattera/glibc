/* Make sure that the stackaddr returned by pthread_getattr_np is
   reachable.

   Copyright (C) 2012 Free Software Foundation, Inc.
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
#include <string.h>
#include <sys/resource.h>
#include <pthread.h>
#include <alloca.h>

/* Move the stack pointer so that stackaddr is accessible and then check if it
   really is accessible.  This will segfault if it fails.  */
static void
allocate_and_test (void *stackaddr)
{
  void *mem = &mem;
  /* FIXME:  The difference will be negative for _STACK_GROWSUP.  */
  mem = alloca ((size_t) (mem - stackaddr));
  *(int *)(mem) = 0;
}

static int
get_self_pthread_attr (const char *id, void **stackaddr, size_t *stacksize)
{
  pthread_attr_t attr;
  int ret;
  pthread_t me = pthread_self ();

  if ((ret = pthread_getattr_np (me, &attr)))
    {
      printf ("%s: pthread_getattr_np failed: %s\n", id, strerror (ret));
      return 1;
    }

  if ((ret = pthread_attr_getstack (&attr, stackaddr, stacksize)))
    {
      printf ("%s: pthread_attr_getstack returned error: %s\n", id,
	      strerror (ret));
      return 1;
    }

  return 0;
}

/* Verify that the stack size returned by pthread_getattr_np is usable when
   the returned value is subject to rlimit.  */
static int
check_stack_top (void)
{
  struct rlimit stack_limit;
  void *stackaddr;
  size_t stacksize = 0;
  int ret;

  puts ("Verifying that stack top is accessible");

  ret = getrlimit (RLIMIT_STACK, &stack_limit);
  if (ret)
    {
      perror ("getrlimit failed");
      return 1;
    }

  if (get_self_pthread_attr ("check_stack_top", &stackaddr, &stacksize))
    return 1;

  /* Reduce the rlimit to a page less that what is currently being returned so
     that we ensure that pthread_getattr_np uses rlimit.  The figure is
     intentionally unaligned so to verify that pthread_getattr_np returns an
     aligned stacksize that correctly fits into the rlimit.  We don't bother
     about the case where the stack is limited by the vma below it and not by
     the rlimit because the stacksize returned in that case is computed from
     the end of that vma and is hence safe.  */
  stack_limit.rlim_cur = stacksize - 4095;
  printf ("Adjusting RLIMIT_STACK to %zu\n", stack_limit.rlim_cur);
  if ((ret = setrlimit (RLIMIT_STACK, &stack_limit)))
    {
      perror ("setrlimit failed");
      return 1;
    }

  if (get_self_pthread_attr ("check_stack_top2", &stackaddr, &stacksize))
    return 1;

  printf ("Adjusted rlimit: stacksize=%zu, stackaddr=%p\n", stacksize,
          stackaddr);
  allocate_and_test (stackaddr);

  puts ("Stack top tests done");

  return 0;
}

/* TODO: Similar check for thread stacks once the thread stack sizes are
   fixed.  */
static int
do_test (void)
{
  return check_stack_top ();
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
