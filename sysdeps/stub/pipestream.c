/* Copyright (C) 1991, 1993 Free Software Foundation, Inc.
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
#include <signal.h>
#include <stdio.h>
#include <errno.h>


/* Open a new stream that is a one-way pipe to a
   child process running the given shell command.  */
FILE *
DEFUN(popen, (command, mode), CONST char *command AND CONST char *mode)
{
  if (command == NULL || mode == NULL || (*mode != 'r' && *mode != 'w'))
    {
      errno = EINVAL;
      return NULL;
    }

  errno = ENOSYS;
  return NULL;
}

/* Close a stream opened by popen and return its status.
   Returns -1 if the stream was not opened by popen.  */
int
DEFUN(pclose, (stream), register FILE *stream)
{
  if (!__validfp (stream) || !stream->__ispipe)
    {
      errno = EINVAL;
      return -1;
    }

  errno = ENOSYS;
  return -1;
}


#ifdef	 HAVE_GNU_LD

#include <gnu-stabs.h>

stub_warning(popen);
stub_warning(pclose);

#endif	/* GNU stabs.  */
