/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

/* Put the absolute pathname of the current working direction in BUF.
   If successful, return BUF.  If not, put an error message in
   BUF and return NULL.  BUF should be at least PATH_MAX bytes long.  */
char *
DEFUN(getwd, (buf), char *buf)
{
  if (buf == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

#ifndef	PATH_MAX
#define	PATH_MAX	1024	/* Arbitrary; this function is unreliable.  */
#endif
  if (getcwd (buf, PATH_MAX) == NULL)
    {
      (void) strncpy (buf, strerror (errno), PATH_MAX);
      return NULL;
    }

  return buf;
}
