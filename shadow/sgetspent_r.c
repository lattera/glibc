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
#include <string.h>

/* Define a line parsing function using the common code
   used in the nss_files module.  */

#define STRUCTURE	spwd
#define ENTNAME		spent
struct spent_data {};

/* Predicate which always returns false, needed below.  */
#define FALSE(arg) 0


#include "../nss/nss_files/files-parse.c"
LINE_PARSER
(,
 STRING_FIELD (result->sp_namp, ISCOLON, 0);
 STRING_FIELD (result->sp_pwdp, ISCOLON, 0);
 INT_FIELD (result->sp_lstchg, ISCOLON, 0, 10, (long int));
 INT_FIELD (result->sp_min, ISCOLON, 0, 10, (long int));
 INT_FIELD (result->sp_max, ISCOLON, 0, 10, (long int));
 while (isspace (*line))
   ++line;
 if (*line == '\0')
   {
     /* The old form.  */
     result->sp_warn = (long int) -1;
     result->sp_inact = (long int) -1;
     result->sp_expire = (long int) -1;
     result->sp_flag = ~0ul;
   }
 else
   {
     INT_FIELD_MAYBE_NULL (result->sp_warn, ISCOLON, 0, 10, (long int),
			   (long int) -1);
     INT_FIELD_MAYBE_NULL (result->sp_inact, ISCOLON, 0, 10, (long int),
			   (long int) -1);
     INT_FIELD_MAYBE_NULL (result->sp_expire, ISCOLON, 0, 10, (long int),
			   (long int) -1);
     if (*line != '\0')
       INT_FIELD_MAYBE_NULL (result->sp_flag, FALSE, 0, 10,
			     (unsigned long int), ~0ul)
     else
       result->sp_flag = ~0ul;
   }
 )


/* Read one shadow entry from the given stream.  */
struct spwd *
__sgetspent_r (const char *string, struct spwd *result, char *buffer,
	       int buflen)
{
  return parse_line (strncpy (buffer, string, buflen), result, NULL, 0)
    ? result : NULL;
}
weak_alias (__sgetspent_r, sgetspent_r)
