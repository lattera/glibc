/* Copyright (C) 1991, 1993, 1996 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <string.h>


/* Generate a unique filename in P_tmpdir.

   This function is *not* thread safe!  */
char *
tmpnam (char *s)
{
  /* By using two buffers we manage to be thread safe in the case
     where S != NULL.  */
  static char buf[L_tmpnam];
  char tmpbuf[L_tmpnam];
  char *result;

  /* In the following call we use the buffer pointed to by S if
     non-NULL although we don't know the size.  But we limit the size
     to FILENAME_MAX characters in any case.  */
  result = __stdio_gen_tempname (s ?: tmpbuf, L_tmpnam, (const char *) NULL,
				 (const char *) NULL, 0,
				 (size_t *) NULL, (FILE **) NULL);

  if (result != NULL && s == NULL)
    {
      memcpy (buf, result, L_tmpnam);
      result = buf;
    }

  return result;
}
