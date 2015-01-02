/* Copyright (C) 1991-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <ctype.h>
#include <errno.h>
#include <grp.h>
#include <stdio.h>

#include <libio/iolibio.h>
#define flockfile(s) _IO_flockfile (s)
#define funlockfile(s) _IO_funlockfile (s)

/* Define a line parsing function using the common code
   used in the nss_files module.  */

#define STRUCTURE	group
#define ENTNAME		grent
struct grent_data {};

#define TRAILING_LIST_MEMBER		gr_mem
#define TRAILING_LIST_SEPARATOR_P(c)	((c) == ',')
#include <nss/nss_files/files-parse.c>
LINE_PARSER
(,
 STRING_FIELD (result->gr_name, ISCOLON, 0);
 if (line[0] == '\0'
     && (result->gr_name[0] == '+' || result->gr_name[0] == '-'))
   {
     result->gr_passwd = NULL;
     result->gr_gid = 0;
   }
 else
   {
     STRING_FIELD (result->gr_passwd, ISCOLON, 0);
     if (result->gr_name[0] == '+' || result->gr_name[0] == '-')
       INT_FIELD_MAYBE_NULL (result->gr_gid, ISCOLON, 0, 10, , 0)
     else
       INT_FIELD (result->gr_gid, ISCOLON, 0, 10,)
   }
 )


/* Read one entry from the given stream.  */
int
__fgetgrent_r (FILE *stream, struct group *resbuf, char *buffer, size_t buflen,
	       struct group **result)
{
  char *p;
  int parse_result;

  flockfile (stream);
  do
    {
      buffer[buflen - 1] = '\xff';
      p = fgets_unlocked (buffer, buflen, stream);
      if (__builtin_expect (p == NULL, 0) && feof_unlocked (stream))
	{
	  funlockfile (stream);
	  *result = NULL;
	  __set_errno (ENOENT);
	  return errno;
	}
      if (__builtin_expect (p == NULL, 0) || buffer[buflen - 1] != '\xff')
	{
	  funlockfile (stream);
	  *result = NULL;
	  __set_errno (ERANGE);
	  return errno;
	}

      /* Skip leading blanks.  */
      while (isspace (*p))
	++p;
    } while (*p == '\0' || *p == '#'	/* Ignore empty and comment lines.  */
	     /* Parse the line.  If it is invalid, loop to
		get the next line of the file to parse.  */
	     || ! (parse_result = parse_line (p, resbuf,
					      (void *) buffer, buflen,
					      &errno)));

  funlockfile (stream);

  if (__builtin_expect (parse_result, 0) == -1)
    {
      /* The parser ran out of space.  */
      *result = NULL;
      return errno;
    }

  *result = resbuf;
  return 0;
}
weak_alias (__fgetgrent_r, fgetgrent_r)
