/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <grp.h>

static FILE *stream = NULL;

/* Rewind the stream.  */
void
DEFUN_VOID(setgrent)
{
  if (stream != NULL)
    rewind(stream);
}


/* Close the stream.  */
void
DEFUN_VOID(endgrent)
{
  if (stream != NULL)
    {
      (void) fclose(stream);
      stream = NULL;
    }
}


/* Read an entry from the stream.  */
struct group *
DEFUN_VOID(getgrent)
{
  static PTR info = NULL;
  if (info == NULL)
    {
      info = __grpalloc();
      if (info == NULL)
	return(NULL);
    }

  if (stream == NULL)
    {
      stream = __grpopen();
      if (stream == NULL)
	return(NULL);
    }

  return(__grpread(stream, info));
}
