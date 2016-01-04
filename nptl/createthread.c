/* Low-level thread creation for NPTL.  Stub version.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

/* See the comments in pthread_create.c for the requirements for these
   two macros and the create_thread function.  */

#define START_THREAD_DEFN \
  static void __attribute__ ((noreturn)) start_thread (void)
#define START_THREAD_SELF THREAD_SELF

static int
create_thread (struct pthread *pd, const struct pthread_attr *attr,
	       bool stopped_start, STACK_VARIABLES_PARMS, bool *thread_ran)
{
  /* If the implementation needs to do some tweaks to the thread after
     it has been created at the OS level, it can set STOPPED_START here.  */

  pd->stopped_start = stopped_start;
  if (__glibc_unlikely (stopped_start))
    /* We make sure the thread does not run far by forcing it to get a
       lock.  We lock it here too so that the new thread cannot continue
       until we tell it to.  */
    lll_lock (pd->lock, LLL_PRIVATE);

  return ENOSYS;

  /* It's started now, so if we fail below, we'll have to cancel it
     and let it clean itself up.  */
  *thread_ran = true;

  return 0;
}
