/* Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

#include <sched.h>
#include <signal.h>
#include <string.h>	/* For the real memset prototype.  */
#include <sysdep.h>
#include <unistd.h>
#include <sys/wait.h>
#include <libc-lock.h>

/* We have to and actually can handle cancelable system().  The big
   problem: we have to kill the child process if necessary.  To do
   this a cleanup handler has to be registered and is has to be able
   to find the PID of the child.  The main problem is to reliable have
   the PID when needed.  It is not necessary for the parent thread to
   return.  It might still be in the kernel when the cancellation
   request comes.  Therefore we have to use the clone() calls ability
   to have the kernel write the PID into the user-level variable.  */
#ifndef FORK
# define FORK() \
  INLINE_SYSCALL (clone, 3, CLONE_PARENT_SETTID | SIGCHLD, 0, &pid)
#endif

#ifdef _LIBC_REENTRANT
static void cancel_handler (void *arg);

# define CLEANUP_HANDLER \
  __libc_cleanup_region_start (1, cancel_handler, &pid)

# define CLEANUP_RESET \
  __libc_cleanup_region_end (0)
#endif


/* Linux has waitpid(), so override the generic unix version.  */
#include <sysdeps/posix/system.c>


#ifdef _LIBC_REENTRANT
/* The cancellation handler.  */
static void
cancel_handler (void *arg)
{
  pid_t child = *(pid_t *) arg;

  INTERNAL_SYSCALL_DECL (err);
  INTERNAL_SYSCALL (kill, err, 2, child, SIGKILL);

  TEMP_FAILURE_RETRY (__waitpid (child, NULL, 0));

  DO_LOCK ();

  if (SUB_REF () == 0)
    {
      (void) __sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
      (void) __sigaction (SIGINT, &intr, (struct sigaction *) NULL);
    }

  DO_UNLOCK ();
}
#endif
