/* Copyright (C) 1991, 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <stddef.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>


/* Open a new stream that is a one-way pipe to a
   child process running the given shell command.  */
FILE *
popen (command, mode)
     const char *command;
     const char *mode;
{
  if (command == NULL || mode == NULL || (*mode != 'r' && *mode != 'w'))
    {
      __set_errno (EINVAL);
      return NULL;
    }

  __set_errno (ENOSYS);
  return NULL;
}

/* Close a stream opened by popen and return its status.
   Returns -1 if the stream was not opened by popen.  */
int
pclose (stream)
     register FILE *stream;
{
  if (!__validfp (stream) || !stream->__ispipe)
    {
      __set_errno (EINVAL);
      return -1;
    }

  __set_errno (ENOSYS);
  return -1;
}

stub_warning (popen)
stub_warning (pclose)
#include <stub-tag.h>
