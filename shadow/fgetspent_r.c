/* Copyright (C) 1996 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <shadow.h>
#include <stdio.h>

/* Define a line parsing function using the common code
   used in the nss_files module.  */

#define STRUCTURE	spwd
#define ENTNAME		spent
#define	EXTERN_PARSER	1
struct spent_data {};

#include "../nss/nss_files/files-parse.c"


/* Read one shadow entry from the given stream.  */
struct spwd *
__fgetspent_r (FILE *stream, struct spwd *result, char *buffer, int buflen)
{
  char *p;

  do
    {
      p = fgets (buffer, buflen, stream);
      if (p == NULL)
	return NULL;

      /* Skip leading blanks.  */
      while (isspace (*p))
	++p;
    } while (*p == '\0' || *p == '#' ||	/* Ignore empty and comment lines.  */
	     /* Parse the line.  If it is invalid, loop to
		get the next line of the file to parse.  */
	     ! parse_line (buffer, (void *) result, NULL, 0));

  return result;
}
weak_alias (__fgetspent_r, fgetspent_r)
