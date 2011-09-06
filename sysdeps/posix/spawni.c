/* Guts of POSIX spawn interface.  Generic POSIX.1 version.
   Copyright (C) 2000-2005, 2006, 2011 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/resource.h>
#include "spawn_int.h"
#include <not-cancel.h>
#include <local-setxid.h>
#include <shlib-compat.h>


/* The Unix standard contains a long explanation of the way to signal
   an error after the fork() was successful.  Since no new wait status
   was wanted there is no way to signal an error using one of the
   available methods.  The committee chose to signal an error by a
   normal program exit with the exit code 127.  */
#define SPAWN_ERROR	127


#if SHLIB_COMPAT (libc, GLIBC_2_2, GLIBC_2_15)
/* The file is accessible but it is not an executable file.  Invoke
   the shell to interpret it as a script.  */
static void
internal_function
script_execute (const char *file, char *const argv[], char *const envp[])
{
  /* Count the arguments.  */
  int argc = 0;
  while (argv[argc++])
    ;

  /* Construct an argument list for the shell.  */
  {
    char *new_argv[argc + 1];
    new_argv[0] = (char *) _PATH_BSHELL;
    new_argv[1] = (char *) file;
    while (argc > 1)
      {
	new_argv[argc] = argv[argc - 1];
	--argc;
      }

    /* Execute the shell.  */
    __execve (new_argv[0], new_argv, envp);
  }
}
# define tryshell (xflags & SPAWN_XFLAGS_TRY_SHELL)
#else
# define tryshell 0
#endif


/* Spawn a new process executing PATH with the attributes describes in *ATTRP.
   Before running the process perform the actions described in FILE-ACTIONS. */
int
__spawni (pid_t *pid, const char *file,
	  const posix_spawn_file_actions_t *file_actions,
	  const posix_spawnattr_t *attrp, char *const argv[],
	  char *const envp[], int xflags)
{
  pid_t new_pid;
  char *path, *p, *name;
  size_t len;
  size_t pathlen;

  /* Do this once.  */
  short int flags = attrp == NULL ? 0 : attrp->__flags;

  /* Generate the new process.  */
  if ((flags & POSIX_SPAWN_USEVFORK) != 0
      /* If no major work is done, allow using vfork.  Note that we
	 might perform the path searching.  But this would be done by
	 a call to execvp(), too, and such a call must be OK according
	 to POSIX.  */
      || ((flags & (POSIX_SPAWN_SETSIGMASK | POSIX_SPAWN_SETSIGDEF
		    | POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER
		    | POSIX_SPAWN_SETPGROUP | POSIX_SPAWN_RESETIDS)) == 0
	  && file_actions == NULL))
    new_pid = __vfork ();
  else
    new_pid = __fork ();

  if (new_pid != 0)
    {
      if (new_pid < 0)
	return errno;

      /* The call was successful.  Store the PID if necessary.  */
      if (pid != NULL)
	*pid = new_pid;

      return 0;
    }

  /* Set signal mask.  */
  if ((flags & POSIX_SPAWN_SETSIGMASK) != 0
      && __sigprocmask (SIG_SETMASK, &attrp->__ss, NULL) != 0)
    _exit (SPAWN_ERROR);

  /* Set signal default action.  */
  if ((flags & POSIX_SPAWN_SETSIGDEF) != 0)
    {
      /* We have to iterate over all signals.  This could possibly be
	 done better but it requires system specific solutions since
	 the sigset_t data type can be very different on different
	 architectures.  */
      int sig;
      struct sigaction sa;

      memset (&sa, '\0', sizeof (sa));
      sa.sa_handler = SIG_DFL;

      for (sig = 1; sig <= _NSIG; ++sig)
	if (__sigismember (&attrp->__sd, sig) != 0
	    && __sigaction (sig, &sa, NULL) != 0)
	  _exit (SPAWN_ERROR);

    }

#ifdef _POSIX_PRIORITY_SCHEDULING
  /* Set the scheduling algorithm and parameters.  */
  if ((flags & (POSIX_SPAWN_SETSCHEDPARAM | POSIX_SPAWN_SETSCHEDULER))
      == POSIX_SPAWN_SETSCHEDPARAM)
    {
      if (__sched_setparam (0, &attrp->__sp) == -1)
	_exit (SPAWN_ERROR);
    }
  else if ((flags & POSIX_SPAWN_SETSCHEDULER) != 0)
    {
      if (__sched_setscheduler (0, attrp->__policy, &attrp->__sp) == -1)
	_exit (SPAWN_ERROR);
    }
#endif

  /* Set the process group ID.  */
  if ((flags & POSIX_SPAWN_SETPGROUP) != 0
      && __setpgid (0, attrp->__pgrp) != 0)
    _exit (SPAWN_ERROR);

  /* Set the effective user and group IDs.  */
  if ((flags & POSIX_SPAWN_RESETIDS) != 0
      && (local_seteuid (__getuid ()) != 0
	  || local_setegid (__getgid ()) != 0))
    _exit (SPAWN_ERROR);

  /* Execute the file actions.  */
  if (file_actions != NULL)
    {
      int cnt;
      struct rlimit64 fdlimit;
      bool have_fdlimit = false;

      for (cnt = 0; cnt < file_actions->__used; ++cnt)
	{
	  struct __spawn_action *action = &file_actions->__actions[cnt];

	  switch (action->tag)
	    {
	    case spawn_do_close:
	      if (close_not_cancel (action->action.close_action.fd) != 0)
		{
		  if (! have_fdlimit)
		    {
		      getrlimit64 (RLIMIT_NOFILE, &fdlimit);
		      have_fdlimit = true;
		    }

		  /* Only signal errors for file descriptors out of range.  */
		  if (action->action.close_action.fd < 0
		      || action->action.close_action.fd >= fdlimit.rlim_cur)
		    /* Signal the error.  */
		    _exit (SPAWN_ERROR);
		}
	      break;

	    case spawn_do_open:
	      {
		int new_fd = open_not_cancel (action->action.open_action.path,
					      action->action.open_action.oflag
					      | O_LARGEFILE,
					      action->action.open_action.mode);

		if (new_fd == -1)
		  /* The `open' call failed.  */
		  _exit (SPAWN_ERROR);

		/* Make sure the desired file descriptor is used.  */
		if (new_fd != action->action.open_action.fd)
		  {
		    if (__dup2 (new_fd, action->action.open_action.fd)
			!= action->action.open_action.fd)
		      /* The `dup2' call failed.  */
		      _exit (SPAWN_ERROR);

		    if (close_not_cancel (new_fd) != 0)
		      /* The `close' call failed.  */
		      _exit (SPAWN_ERROR);
		  }
	      }
	      break;

	    case spawn_do_dup2:
	      if (__dup2 (action->action.dup2_action.fd,
			  action->action.dup2_action.newfd)
		  != action->action.dup2_action.newfd)
		/* The `dup2' call failed.  */
		_exit (SPAWN_ERROR);
	      break;
	    }
	}
    }

  if ((xflags & SPAWN_XFLAGS_USE_PATH) == 0 || strchr (file, '/') != NULL)
    {
      /* The FILE parameter is actually a path.  */
      __execve (file, argv, envp);

      if (tryshell && errno == ENOEXEC)
	script_execute (file, argv, envp);

      /* Oh, oh.  `execve' returns.  This is bad.  */
      _exit (SPAWN_ERROR);
    }

  /* We have to search for FILE on the path.  */
  path = getenv ("PATH");
  if (path == NULL)
    {
      /* There is no `PATH' in the environment.
	 The default search path is the current directory
	 followed by the path `confstr' returns for `_CS_PATH'.  */
      len = confstr (_CS_PATH, (char *) NULL, 0);
      path = (char *) __alloca (1 + len);
      path[0] = ':';
      (void) confstr (_CS_PATH, path + 1, len);
    }

  len = strlen (file) + 1;
  pathlen = strlen (path);
  name = __alloca (pathlen + len + 1);
  /* Copy the file name at the top.  */
  name = (char *) memcpy (name + pathlen + 1, file, len);
  /* And add the slash.  */
  *--name = '/';

  p = path;
  do
    {
      char *startp;

      path = p;
      p = __strchrnul (path, ':');

      if (p == path)
	/* Two adjacent colons, or a colon at the beginning or the end
	   of `PATH' means to search the current directory.  */
	startp = name + 1;
      else
	startp = (char *) memcpy (name - (p - path), path, p - path);

      /* Try to execute this name.  If it works, execv will not return.  */
      __execve (startp, argv, envp);

      if (tryshell && errno == ENOEXEC)
	script_execute (startp, argv, envp);

      switch (errno)
	{
	case EACCES:
	case ENOENT:
	case ESTALE:
	case ENOTDIR:
	  /* Those errors indicate the file is missing or not executable
	     by us, in which case we want to just try the next path
	     directory.  */
	  break;

	default:
	  /* Some other error means we found an executable file, but
	     something went wrong executing it; return the error to our
	     caller.  */
	  _exit (SPAWN_ERROR);
	    }
    }
  while (*p++ != '\0');

  /* Return with an error.  */
  _exit (SPAWN_ERROR);
}
