/* Copyright (C) 1991, 1996 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <pwd.h>

/* Define a line parsing function using the common code
   used in the nss_files module.  */

#define STRUCTURE	passwd
#define ENTNAME		pwent
struct pwent_data {};

#include "../nss/nss_files/files-parse.c"
LINE_PARSER
(,
 STRING_FIELD (result->pw_name, ISCOLON, 0);
 STRING_FIELD (result->pw_passwd, ISCOLON, 0);
 if (result->pw_name[0] == '+' || result->pw_name[0] == '-')
   {
     INT_FIELD_MAYBE_NULL (result->pw_uid, ISCOLON, 0, 10, , 0)
     INT_FIELD_MAYBE_NULL (result->pw_gid, ISCOLON, 0, 10, , 0)
   }
 else
   {
     INT_FIELD (result->pw_uid, ISCOLON, 0, 10,)
     INT_FIELD (result->pw_gid, ISCOLON, 0, 10,)
   }
 STRING_FIELD (result->pw_gecos, ISCOLON, 0);
 STRING_FIELD (result->pw_dir, ISCOLON, 0);
 result->pw_shell = line;
 )


/* Read one entry from the given stream.  */
int
__fgetpwent_r (FILE *stream, struct passwd *resbuf, char *buffer,
	       size_t buflen, struct passwd **result)
{
  char *p;

  do
    {
      p = fgets (buffer, buflen, stream);
      if (p == NULL)
	{
	  *result = NULL;
	  return errno;
	}

      /* Skip leading blanks.  */
      while (isspace (*p))
	++p;
    } while (*p == '\0' || *p == '#' ||	/* Ignore empty and comment lines.  */
	     /* Parse the line.  If it is invalid, loop to
		get the next line of the file to parse.  */
	     ! parse_line (p, resbuf, (void *) buffer, buflen));

  *result = resbuf;
  return 0;
}
weak_alias (__fgetpwent_r, fgetpwent_r)
