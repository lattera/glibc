/* Low-level thread creation for NPTL.  Linux version.
   Copyright (C) 2002-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <sched.h>
#include <setjmp.h>
#include <signal.h>
#include <stdlib.h>
#include <atomic.h>
#include <ldsodefs.h>
#include <tls.h>
#include <stdint.h>

#include <arch-fork.h>

#ifdef __NR_clone2
# define ARCH_CLONE __clone2
#else
# define ARCH_CLONE __clone
#endif

/* See the comments in pthread_create.c for the requirements for these
   two macros and the create_thread function.  */

#define START_THREAD_DEFN \
  static int __attribute__ ((noreturn)) start_thread (void *arg)
#define START_THREAD_SELF arg

/* pthread_create.c defines this using START_THREAD_DEFN
   We need a forward declaration here so we can take its address.  */
static int start_thread (void *arg) __attribute__ ((noreturn));

static int
create_thread (struct pthread *pd, const struct pthread_attr *attr,
	       bool *stopped_start, STACK_VARIABLES_PARMS, bool *thread_ran)
{
  /* Determine whether the newly created threads has to be started
     stopped since we have to set the scheduling parameters or set the
     affinity.  */
  if (attr != NULL
      && (__glibc_unlikely (attr->cpuset != NULL)
	  || __glibc_unlikely ((attr->flags & ATTR_FLAG_NOTINHERITSCHED) != 0)))
    *stopped_start = true;

  pd->stopped_start = *stopped_start;
  if (__glibc_unlikely (*stopped_start))
    /* See CONCURRENCY NOTES in nptl/pthread_creat.c.  */
    lll_lock (pd->lock, LLL_PRIVATE);

  /* We rely heavily on various flags the CLONE function understands:

     CLONE_VM, CLONE_FS, CLONE_FILES
	These flags select semantics with shared address space and
	file descriptors according to what POSIX requires.

     CLONE_SIGHAND, CLONE_THREAD
	This flag selects the POSIX signal semantics and various
	other kinds of sharing (itimers, POSIX timers, etc.).

     CLONE_SETTLS
	The sixth parameter to CLONE determines the TLS area for the
	new thread.

     CLONE_PARENT_SETTID
	The kernels writes the thread ID of the newly created thread
	into the location pointed to by the fifth parameters to CLONE.

	Note that it would be semantically equivalent to use
	CLONE_CHILD_SETTID but it is be more expensive in the kernel.

     CLONE_CHILD_CLEARTID
	The kernels clears the thread ID of a thread that has called
	sys_exit() in the location pointed to by the seventh parameter
	to CLONE.

     The termination signal is chosen to be zero which means no signal
     is sent.  */
  const int clone_flags = (CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SYSVSEM
			   | CLONE_SIGHAND | CLONE_THREAD
			   | CLONE_SETTLS | CLONE_PARENT_SETTID
			   | CLONE_CHILD_CLEARTID
			   | 0);

  TLS_DEFINE_INIT_TP (tp, pd);

  if (__glibc_unlikely (ARCH_CLONE (&start_thread, STACK_VARIABLES_ARGS,
				    clone_flags, pd, &pd->tid, tp, &pd->tid)
			== -1))
    return errno;

  /* It's started now, so if we fail below, we'll have to cancel it
     and let it clean itself up.  */
  *thread_ran = true;

  /* Now we have the possibility to set scheduling parameters etc.  */
  if (attr != NULL)
    {
      INTERNAL_SYSCALL_DECL (err);
      int res;

      /* Set the affinity mask if necessary.  */
      if (attr->cpuset != NULL)
	{
	  assert (*stopped_start);

	  res = INTERNAL_SYSCALL (sched_setaffinity, err, 3, pd->tid,
				  attr->cpusetsize, attr->cpuset);

	  if (__glibc_unlikely (INTERNAL_SYSCALL_ERROR_P (res, err)))
	  err_out:
	    {
	      /* The operation failed.  We have to kill the thread.
		 We let the normal cancellation mechanism do the work.  */

	      pid_t pid = __getpid ();
	      INTERNAL_SYSCALL_DECL (err2);
	      (void) INTERNAL_SYSCALL_CALL (tgkill, err2, pid, pd->tid,
					    SIGCANCEL);

	      return INTERNAL_SYSCALL_ERRNO (res, err);
	    }
	}

      /* Set the scheduling parameters.  */
      if ((attr->flags & ATTR_FLAG_NOTINHERITSCHED) != 0)
	{
	  assert (*stopped_start);

	  res = INTERNAL_SYSCALL (sched_setscheduler, err, 3, pd->tid,
				  pd->schedpolicy, &pd->schedparam);

	  if (__glibc_unlikely (INTERNAL_SYSCALL_ERROR_P (res, err)))
	    goto err_out;
	}
    }

  return 0;
}
