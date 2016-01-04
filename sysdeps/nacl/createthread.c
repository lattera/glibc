/* Low-level thread creation for NPTL.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <nacl-interfaces.h>
#include <pthread-pids.h>

/* See the comments in pthread_create.c for the requirements for these
   two macros and the create_thread function.  */

#define START_THREAD_DEFN \
  static void __attribute__ ((noreturn)) start_thread (void)
#define START_THREAD_SELF THREAD_SELF

/* pthread_create.c defines this using START_THREAD_DEFN
   We need a forward declaration here so we can take its address.  */
static void start_thread (void) __attribute__ ((noreturn));

static int
create_thread (struct pthread *pd, const struct pthread_attr *attr,
	       bool stopped_start, STACK_VARIABLES_PARMS, bool *thread_ran)
{
  pd->tid = __nacl_get_tid (pd);

  pd->stopped_start = stopped_start;
  if (__glibc_unlikely (stopped_start))
    /* We make sure the thread does not run far by forcing it to get a
       lock.  We lock it here too so that the new thread cannot continue
       until we tell it to.  */
    lll_lock (pd->lock, LLL_PRIVATE);

  TLS_DEFINE_INIT_TP (tp, pd);

  return __nacl_irt_thread.thread_create (&start_thread, stackaddr, tp);
}
