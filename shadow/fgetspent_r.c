/* Copyright (C) 1996, 1997, 1998, 1999 Free Software Foundation, Inc.
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

#include <ctype.h>
#include <errno.h>
#include <shadow.h>
#include <stdio.h>

#ifdef USE_IN_LIBIO
# define flockfile(s) _IO_flockfile (s)
# define funlockfile(s) _IO_funlockfile (s)
#endif

/* Define a line parsing function using the common code
   used in the nss_files module.  */

#define STRUCTURE	spwd
#define ENTNAME		spent
#define	EXTERN_PARSER	1
struct spent_data {};

#include <nss/nss_files/files-parse.c>


/* Read one shadow entry from the given stream.  */
int
__fgetspent_r (FILE *stream, struct spwd *resbuf, char *buffer, size_t buflen,
	       struct spwd **result)
{
  char *p;

  flockfile (stream);
  do
    {
      buffer[buflen - 1] = '\xff';
      p = fgets_unlocked (buffer, buflen, stream);
      if (p == NULL && feof_unlocked (stream))
	{
	  funlockfile (stream);
	  *result = NULL;
	  __set_errno (ENOENT);
	  return errno;
	}
      if (p == NULL || buffer[buflen - 1] != '\xff')
	{
	  funlockfile (stream);
	  *result = NULL;
	  __set_errno (ERANGE);
	  return errno;
	}

      /* Skip leading blanks.  */
      while (isspace (*p))
	++p;
    } while (*p == '\0' || *p == '#' ||	/* Ignore empty and comment lines.  */
	     /* Parse the line.  If it is invalid, loop to
		get the next line of the file to parse.  */
	     ! parse_line (buffer, (void *) resbuf, NULL, 0, &errno));

  funlockfile (stream);

  *result = resbuf;
  return 0;
}
weak_alias (__fgetspent_r, fgetspent_r)
