/* Copyright (C) 1991,1992,1993,1997,1998,2000 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

/* Print a line on stderr consisting of the text in S, a colon, a space,
   a message describing the meaning of the contents of `errno' and a newline.
   If S is NULL or "", the colon and space are omitted.  */
void
perror (const char *s)
{
  char buf[1024];
  int errnum = errno;
  const char *colon;
  const char *errstring;

  if (s == NULL || *s == '\0')
    s = colon = "";
  else
    colon = ": ";

  errstring = __strerror_r (errnum, buf, sizeof buf);

#ifdef USE_IN_LIBIO
  if (fwide (stderr, 0) > 0)
    (void) fwprintf (stderr, L"%s%s%s\n", s, colon, errstring);
  else
#endif
    (void) fprintf (stderr, "%s%s%s\n", s, colon, errstring);
}
