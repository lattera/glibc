/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
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

#include <libintl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <_itoa.h>

/* It is critical here that we always use the `dcgettext' function for
   the message translation.  Since <libintl.h> only defines the macro
   `dgettext' to use `dcgettext' for optimizing programs this is not
   always guaranteed.  */
#ifndef dgettext
# include <locale.h>		/* We need LC_MESSAGES.  */
# define dgettext(domainname, msgid) dcgettext (domainname, msgid, LC_MESSAGES)
#endif

/* Return a string describing the errno code in ERRNUM.  */
char *
__strerror_r (int errnum, char *buf, size_t buflen)
{
  if (__builtin_expect (errnum < 0 || errnum >= _sys_nerr_internal
			|| _sys_errlist_internal[errnum] == NULL, 0))
    {
      /* Buffer we use to print the number in.  For a maximum size for
	 `int' of 8 bytes we never need more than 20 digits.  */
      char numbuf[21];
      const char *unk = _("Unknown error ");
      size_t unklen = strlen (unk);
      char *p, *q;
      bool negative = errnum < 0;

      numbuf[20] = '\0';
      p = _itoa_word (abs (errnum), &numbuf[20], 10, 0);

      /* Now construct the result while taking care for the destination
	 buffer size.  */
      q = __mempcpy (buf, unk, MIN (unklen, buflen));
      if (negative && unklen < buflen)
	{
	  *q++ = '-';
	  ++unklen;
	}
      if (unklen < buflen)
	memcpy (q, p, MIN ((size_t) (&numbuf[21] - p), buflen - unklen));

      /* Terminate the string in any case.  */
      if (buflen > 0)
	buf[buflen - 1] = '\0';

      return buf;
    }

  return (char *) _(_sys_errlist_internal[errnum]);
}
weak_alias (__strerror_r, strerror_r)
libc_hidden_def (__strerror_r)
