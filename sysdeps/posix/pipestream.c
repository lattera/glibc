/* Copyright (C) 1991, 1992, 1993, 1996, 1997 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define	SH_PATH	"/bin/sh"	/* Shell to run.  */
#define	SH_NAME	"sh"		/* Name to give it.  */

/* Structure describing a popen child.  */
struct child
  {
    pid_t pid;			/* PID of the child.  */
    __ptr_t cookie;		/* Original cookie from fdopen.  */
    __io_functions funcs;	/* Original functions from fdopen.  */
  };

/* io_functions for pipe streams.
   These all simply call the corresponding
   original function with the original cookie.  */

#define FUNC(type, name, proto, args)					      \
  static type __CONCAT(child_,name) proto				      \
  {									      \
    struct child *c = (struct child *) cookie;				      \
    {									      \
      __ptr_t cookie = c->cookie;					      \
      return (*c->funcs.__CONCAT(__,name)) args;			      \
    }									      \
  }

FUNC (int, read, (void *cookie, char *buf, size_t n), (cookie, buf, n))
FUNC (int, write, (void *cookie, const char *buf, size_t n), (cookie, buf, n))
FUNC (int, seek, (void *cookie, fpos_t *pos, int whence),
      (cookie, pos, whence))
FUNC (int, close, (void *cookie), (cookie))
FUNC (int, fileno, (void *cookie), (cookie))

static const __io_functions child_funcs
  = { child_read, child_write, child_seek, child_close, child_fileno };

/* Open a new stream that is a one-way pipe to a
   child process running the given shell command.  */
FILE *
popen (command, mode)
     const char *command;
     const char *mode;
{
  pid_t pid;
  int pipedes[2];
  FILE *stream;
  struct child *child;

  if (command == NULL || mode == NULL || (*mode != 'r' && *mode != 'w'))
    {
      __set_errno (EINVAL);
      return NULL;
    }

  /* Create the pipe.  */
  if (pipe (pipedes) < 0)
    return NULL;

  /* Fork off the child.  */
  pid = __vfork ();
  if (pid == (pid_t) -1)
    {
      /* The fork failed.  */
      (void) close (pipedes[0]);
      (void) close (pipedes[1]);
      return NULL;
    }
  else if (pid == (pid_t) 0)
    {
      /* We are the child side.  Make the write side of
	 the pipe be stdin or the read side be stdout.  */

      const char *new_argv[4];

      if ((*mode == 'w' ? dup2(pipedes[STDIN_FILENO], STDIN_FILENO) :
	  dup2 (pipedes[STDOUT_FILENO], STDOUT_FILENO)) < 0)
	_exit (127);

      /* Close the pipe descriptors.  */
      (void) close (pipedes[STDIN_FILENO]);
      (void) close (pipedes[STDOUT_FILENO]);

      /* Exec the shell.  */
      new_argv[0] = SH_NAME;
      new_argv[1] = "-c";
      new_argv[2] = command;
      new_argv[3] = NULL;
      (void) execve (SH_PATH, (char *const *) new_argv, environ);
      /* Die if it failed.  */
      _exit (127);
    }

  /* We are the parent side.  */

  /* Close the irrelevant side of the pipe and open the relevant side as a
     new stream.  Mark our side of the pipe to close on exec, so new children
     won't see it.  */
  if (*mode == 'r')
    {
      (void) close (pipedes[STDOUT_FILENO]);
      (void) fcntl (pipedes[STDIN_FILENO], F_SETFD, FD_CLOEXEC);
      stream = fdopen (pipedes[STDIN_FILENO], mode);
    }
  else
    {
      (void) close (pipedes[STDIN_FILENO]);
      (void) fcntl (pipedes[STDOUT_FILENO], F_SETFD, FD_CLOEXEC);
      stream = fdopen (pipedes[STDOUT_FILENO], mode);
    }

  if (stream == NULL)
    goto error;

  child = (struct child *) malloc (sizeof (struct child));
  if (child == NULL)
    goto error;

  {
    /* Make sure STREAM has its functions set before
       we try to squirrel them away in CHILD.  */
    extern void __stdio_check_funcs __P ((FILE *));
    __stdio_check_funcs (stream);
  }

  child->pid = pid;
  child->cookie = stream->__cookie;
  child->funcs = stream->__io_funcs;
  stream->__cookie = (void *) child;
  stream->__io_funcs = child_funcs;
  stream->__ispipe = 1;
  return stream;

 error:
  {
    /* The stream couldn't be opened or the child structure couldn't be
       allocated.  Kill the child and close the other side of the pipe.  */
    int save = errno;
    (void) kill (pid, SIGKILL);
    if (stream == NULL)
      (void) close (pipedes[*mode == 'r' ? STDOUT_FILENO : STDIN_FILENO]);
    else
      (void) fclose (stream);
#ifndef	NO_WAITPID
    (void) waitpid (pid, (int *) NULL, 0);
#else
    {
      pid_t dead;
      do
	dead = wait ((int *) NULL);
      while (dead > 0 && dead != pid);
    }
#endif
    __set_errno (save);
    return NULL;
  }
}

/* Close a stream opened by popen and return its status.
   Returns -1 if the stream was not opened by popen.  */
int
pclose (stream)
     register FILE *stream;
{
  struct child *c;
  pid_t pid, dead;
  int status;

  if (!__validfp (stream) || !stream->__ispipe)
    {
      __set_errno (EINVAL);
      return -1;
    }

  c = (struct child *) stream->__cookie;
  pid = c->pid;
  stream->__cookie = c->cookie;
  stream->__io_funcs = c->funcs;
  free ((void *) c);
  stream->__ispipe = 0;
  if (fclose (stream))
    return -1;

#ifndef	NO_WAITPID
  dead = waitpid (pid, &status, 0);
#else
  do
    dead = wait (&status);
  while (dead > 0 && dead != pid);
#endif
  if (dead != pid)
    status = -1;

  return status;
}
