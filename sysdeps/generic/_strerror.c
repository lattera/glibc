/* Copyright (C) 1991, 1993, 1995 Free Software Foundation, Inc.
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

#ifndef HAVE_GNU_LD
#define _sys_errlist sys_errlist
#define _sys_nerr sys_nerr
#endif

/* Return a string describing the errno code in ERRNUM.  */
char *
_strerror_internal (errnum, buf, buflen)
     int errnum;
     char *buf;
     size_t buflen;
{
  if (errnum < 0 || errnum > _sys_nerr)
    {
      int len = __snprintf (buf, buflen, _("Unknown error %d"), errnum);
      if (len < 0)
	return NULL;
      buf[len - 1] = '\0';
      return buf;
    }

  return (char *) _(_sys_errlist[errnum]);
}
