/* Copyright (C) 1991-2000,2002,2003,2005,2007 Free Software Foundation, Inc.
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

#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <bits/libc-lock.h>
#include <sysdep-cancel.h>


#define	SHELL_PATH	"/bin/sh"	/* Path of the shell.  */
#define	SHELL_NAME	"sh"		/* Name to give it.  */


#ifdef _LIBC_REENTRANT
static struct sigaction intr, quit;
static int sa_refcntr;
__libc_lock_define_initialized (static, lock);

# define DO_LOCK() __libc_lock_lock (lock)
# define DO_UNLOCK() __libc_lock_unlock (lock)
# define INIT_LOCK() ({ __libc_lock_init (lock); sa_refcntr = 0; })
# define ADD_REF() sa_refcntr++
# define SUB_REF() --sa_refcntr
#else
# define DO_LOCK()
# define DO_UNLOCK()
# define INIT_LOCK()
# define ADD_REF() 0
# define SUB_REF() 0
#endif


/* Execute LINE as a shell command, returning its status.  */
static int
do_system (const char *line)
{
  int status, save;
  pid_t pid;
  struct sigaction sa;
#ifndef _LIBC_REENTRANT
  struct sigaction intr, quit;
#endif
  sigset_t omask;

  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  __sigemptyset (&sa.sa_mask);

  DO_LOCK ();
  if (ADD_REF () == 0)
    {
      if (__sigaction (SIGINT, &sa, &intr) < 0)
	{
	  SUB_REF ();
	  goto out;
	}
      if (__sigaction (SIGQUIT, &sa, &quit) < 0)
	{
	  save = errno;
	  SUB_REF ();
	  goto out_restore_sigint;
	}
    }
  DO_UNLOCK ();

  /* We reuse the bitmap in the 'sa' structure.  */
  __sigaddset (&sa.sa_mask, SIGCHLD);
  save = errno;
  if (__sigprocmask (SIG_BLOCK, &sa.sa_mask, &omask) < 0)
    {
#ifndef _LIBC
      if (errno == ENOSYS)
	__set_errno (save);
      else
#endif
	{
	  DO_LOCK ();
	  if (SUB_REF () == 0)
	    {
	      save = errno;
	      (void) __sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
	    out_restore_sigint:
	      (void) __sigaction (SIGINT, &intr, (struct sigaction *) NULL);
	      __set_errno (save);
	    }
	out:
	  DO_UNLOCK ();
	  return -1;
	}
    }

#ifdef CLEANUP_HANDLER
  CLEANUP_HANDLER;
#endif

#ifdef FORK
  pid = FORK ();
#else
  pid = __fork ();
#endif
  if (pid == (pid_t) 0)
    {
      /* Child side.  */
      const char *new_argv[4];
      new_argv[0] = SHELL_NAME;
      new_argv[1] = "-c";
      new_argv[2] = line;
      new_argv[3] = NULL;

      /* Restore the signals.  */
      (void) __sigaction (SIGINT, &intr, (struct sigaction *) NULL);
      (void) __sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
      (void) __sigprocmask (SIG_SETMASK, &omask, (sigset_t *) NULL);
      INIT_LOCK ();

      /* Exec the shell.  */
      (void) __execve (SHELL_PATH, (char *const *) new_argv, __environ);
      _exit (127);
    }
  else if (pid < (pid_t) 0)
    /* The fork failed.  */
    status = -1;
  else
    /* Parent side.  */
    {
      /* Note the system() is a cancellation point.  But since we call
	 waitpid() which itself is a cancellation point we do not
	 have to do anything here.  */
      if (TEMP_FAILURE_RETRY (__waitpid (pid, &status, 0)) != pid)
	status = -1;
    }

#ifdef CLEANUP_HANDLER
  CLEANUP_RESET;
#endif

  save = errno;
  DO_LOCK ();
  if ((SUB_REF () == 0
       && (__sigaction (SIGINT, &intr, (struct sigaction *) NULL)
	   | __sigaction (SIGQUIT, &quit, (struct sigaction *) NULL)) != 0)
      || __sigprocmask (SIG_SETMASK, &omask, (sigset_t *) NULL) != 0)
    {
#ifndef _LIBC
      /* glibc cannot be used on systems without waitpid.  */
      if (errno == ENOSYS)
	__set_errno (save);
      else
#endif
	status = -1;
    }
  DO_UNLOCK ();

  return status;
}

int
__libc_system (const char *line)
{
  if (line == NULL)
    /* Check that we have a command processor available.  It might
       not be available after a chroot(), for example.  */
    return do_system ("exit 0") == 0;

  if (SINGLE_THREAD_P)
    return do_system (line);

  int oldtype = LIBC_CANCEL_ASYNC ();

  int result = do_system (line);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}
weak_alias (__libc_system, system)
