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
#include <pwd.h>

static FILE *stream = NULL;

/* Rewind the stream.  */
void
DEFUN_VOID(setpwent)
{
  if (stream != NULL)
    rewind(stream);
}


/* Close the stream.  */
void
DEFUN_VOID(endpwent)
{
  if (stream != NULL)
    {
      (void) fclose(stream);
      stream = NULL;
    }
}


/* Return one entry from the password file.  */
struct passwd *
DEFUN_VOID(getpwent)
{
  static PTR info = NULL;
  if (info == NULL)
    {
      info = __pwdalloc();
      if (info == NULL)
	return(NULL);
    }

  if (stream == NULL)
    {
      stream = __pwdopen();
      if (stream == NULL)
	return(NULL);
    }

  return(__pwdread(stream, info));
}
