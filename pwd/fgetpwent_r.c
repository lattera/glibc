/* Copyright (C) 1991, 1996, 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <pwd.h>

/* Define a line parsing function using the common code
   used in the nss_files module.  */

#define STRUCTURE	passwd
#define ENTNAME		pwent
struct pwent_data {};

#include <nss/nss_files/files-parse.c>
LINE_PARSER
(,
 STRING_FIELD (result->pw_name, ISCOLON, 0);
 if (line[0] == '\0'
     && (result->pw_name[0] == '+' || result->pw_name[0] == '-'))
   {
     /* This a special case.  We allow lines containing only a `+' sign
	since this is used for nss_compat.  All other services will
	reject this entry later.  Initialize all other fields now.  */
     result->pw_passwd = NULL;
     result->pw_uid = 0;
     result->pw_gid = 0;
     result->pw_gecos = NULL;
     result->pw_dir = NULL;
     result->pw_shell = NULL;
   }
 else
   {
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
   }
 )


/* Read one entry from the given stream.  */
int
__fgetpwent_r (FILE *stream, struct passwd *resbuf, char *buffer,
	       size_t buflen, struct passwd **result)
{
  char *p;

  do
    {
      buffer[buflen] = '\xff';
      p = fgets (buffer, buflen, stream);
      if (p == NULL && feof (stream))
	{
	  *result = NULL;
	  return errno;
	}
      if (p == NULL || buffer[buflen] != '\xff')
	{
	  *result = NULL;
	  return errno = ERANGE;
	}

      /* Skip leading blanks.  */
      while (isspace (*p))
	++p;
    } while (*p == '\0' || *p == '#' ||	/* Ignore empty and comment lines.  */
	     /* Parse the line.  If it is invalid, loop to
		get the next line of the file to parse.  */
	     ! parse_line (p, resbuf, (void *) buffer, buflen, &errno));

  *result = resbuf;
  return 0;
}
weak_alias (__fgetpwent_r, fgetpwent_r)
