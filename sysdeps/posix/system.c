/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/types.h>


#ifndef	HAVE_GNU_LD
#define	__environ	environ
#endif

#define	SHELL_PATH	"/bin/sh"	/* Path of the shell.  */
#define	SHELL_NAME	"sh"		/* Name to give it.  */

/* Execute LINE as a shell command, returning its status.  */
int
DEFUN(system, (line), register CONST char *line)
{
  int status, save;
  pid_t pid;
  struct sigaction sa, intr, quit;
#ifndef WAITPID_CANNOT_BLOCK_SIGCHLD
  sigset_t block, omask;
#endif

  if (line == NULL)
    return 1;

  sa.sa_handler = SIG_IGN;
  sa.sa_flags = 0;
  __sigemptyset (&sa.sa_mask);

  if (__sigaction (SIGINT, &sa, &intr) < 0)
    return -1;
  if (__sigaction (SIGQUIT, &sa, &quit) < 0)
    {
      save = errno;
      (void) __sigaction (SIGINT, &intr, (struct sigaction *) NULL);
      errno = save;
      return -1;
    }

#ifndef WAITPID_CANNOT_BLOCK_SIGCHLD

/* SCO 3.2v4 has a bug where `waitpid' will never return if SIGCHLD is
   blocked.  This makes it impossible for `system' to be implemented in
   compliance with POSIX.2-1992.  They have acknowledged that this is a bug
   but I have not seen nor heard of any forthcoming fix.  */

  __sigemptyset (&block);
  __sigaddset (&block, SIGCHLD);
  save = errno;
  if (__sigprocmask (SIG_BLOCK, &block, &omask) < 0)
    {
      if (errno == ENOSYS)
	errno = save;
      else
	{
	  save = errno;
	  (void) __sigaction (SIGINT, &intr, (struct sigaction *) NULL);
	  (void) __sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
	  errno = save;
	  return -1;
	}
    }
#define UNBLOCK	__sigprocmask (SIG_SETMASK, &omask, (sigset_t *) NULL)
#else
#define UNBLOCK 0
#endif

  pid = __vfork ();
  if (pid == (pid_t) 0)
    {
      /* Child side.  */
      CONST char *new_argv[4];
      new_argv[0] = SHELL_NAME;
      new_argv[1] = "-c";
      new_argv[2] = line;
      new_argv[3] = NULL;

      /* Restore the signals.  */
      (void) __sigaction (SIGINT, &intr, (struct sigaction *) NULL);
      (void) __sigaction (SIGQUIT, &quit, (struct sigaction *) NULL);
      (void) UNBLOCK;

      /* Exec the shell.  */
      (void) __execve (SHELL_PATH, (char *CONST *) new_argv, __environ);
      _exit (127);
    }
  else if (pid < (pid_t) 0)
    /* The fork failed.  */
    status = -1;
  else
    /* Parent side.  */
#ifdef	NO_WAITPID
    {
      pid_t child;
      do
	{
	  child = __wait (&status);
	  if (child <= -1)
	    {
	      status = -1;
	      break;
	    }
	} while (child != pid);
    }
#else
    if (__waitpid (pid, &status, 0) != pid)
      status = -1;
#endif

  save = errno;
  if ((__sigaction (SIGINT, &intr, (struct sigaction *) NULL) |
       __sigaction (SIGQUIT, &quit, (struct sigaction *) NULL) |
       UNBLOCK) != 0)
    {
      if (errno == ENOSYS)
	errno = save;
      else
	return -1;
    }

  return status;
}
